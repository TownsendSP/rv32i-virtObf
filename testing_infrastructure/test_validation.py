import subprocess
import os
import sys
import yaml
import shutil
import re
import time
from typing import List, Dict, Any, Optional, Tuple

# Paths
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR = os.path.join(PROJECT_ROOT, "cmake-build-release")
DIST_DIR = os.path.join(BUILD_DIR, "dist")
EXECRV32I = os.path.join(DIST_DIR, "execrv32i")
OBFUSCATE_PY = os.path.join(DIST_DIR, "obfuscate.py")
TESTING_INFRA = os.path.join(PROJECT_ROOT, "testing_infrastructure")
TESTS_YAML = os.path.join(TESTING_INFRA, "tests.yaml")
CAPSTONE_SCRIPT = os.path.join(TESTING_INFRA, "testing_utils", "capstone_disasm.py")
UNICORN_SCRIPT = os.path.join(TESTING_INFRA, "testing_utils", "unicorn_test_harness.py")
TEST_ARTIFACTS_DIR = os.path.join(BUILD_DIR, "test_artifacts")

os.makedirs(TEST_ARTIFACTS_DIR, exist_ok=True)


class TestScenario:
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.test_name = config["test_name"]
        self.test_dir = os.path.join(PROJECT_ROOT, "testing_infrastructure", config["test_dir"])
        self.source_file = os.path.join(self.test_dir, config["source_file"])
        self.source_header = os.path.splitext(self.source_file)[0] + ".h"
        self.test_main = os.path.join(self.test_dir, config["test_main"])
        self.fn_name = config["fn_name"]
        self.args = [str(a) for a in config.get("args", [])]

        self.out_dir = os.path.join(TEST_ARTIFACTS_DIR, self.test_name)

        # Artifact paths
        self.native_bin = os.path.join(self.out_dir, f"{self.test_name}_native")
        self.obf_exe_name = f"{self.test_name}_obf"
        self.obf_exe = os.path.join(self.out_dir, self.obf_exe_name)
        self.target_rv32i = os.path.join(self.out_dir, "target_fn.rv32i")
        self.target_obf_rv32i = os.path.join(self.out_dir, "target_fn.obf.rv32i")
        self.temp_main = os.path.join(self.out_dir, f"test_{self.test_name}.c")

        # Metrics
        self.time_native = 0.0
        self.time_unicorn = 0.0
        self.time_emu_non_obf = 0.0
        self.time_emu_obf_tool = 0.0
        self.time_obf_binary = 0.0

    def setup(self):
        if os.path.exists(self.out_dir):
            shutil.rmtree(self.out_dir)
        os.makedirs(self.out_dir)

        with open(self.test_main, 'r') as f:
            main_code = f.read()
        main_code = main_code.replace("doOperation", self.fn_name)

        with open(self.temp_main, 'w') as f:
            f.write(main_code)

    def build_native(self):
        cmd_native = [
            "clang", self.temp_main, self.source_file, "-o", self.native_bin,
            "-Wno-implicit-function-declaration"
        ]
        subprocess.check_call(cmd_native, cwd=PROJECT_ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)

    def build_obfuscated(self):
        cmd_obf = [
            sys.executable, OBFUSCATE_PY,
            "--main", self.temp_main,
            "--func-impl", self.source_file,
            "--func-header", self.source_header,
            "--output-name", self.obf_exe_name,
            "--output-dir", self.out_dir
        ]
        subprocess.check_call(cmd_obf, cwd=DIST_DIR, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)

    def _clean_disasm(self, output: str) -> List[str]:
        lines = []
        has_header = "---" in output
        parsing = not has_header

        for line in output.splitlines():
            line = line.strip()
            if "---" in line:
                parsing = True
                continue
            if not parsing:
                continue
            if not line:
                continue
            lines.append(" ".join(line.split()))
        return lines

    def _to_signed_32(self, val: int) -> int:
        val = val & 0xFFFFFFFF
        if val & 0x80000000:
            return val - 0x100000000
        return val

    def check_disassembly(self) -> bool:
        try:
            if not os.path.exists(self.target_rv32i):
                raise RuntimeError(f"obfuscate.py did not produce {self.target_rv32i}")

            proc_my_dis = subprocess.run([EXECRV32I, "dis", self.target_rv32i, "--onlyasm"],
                                         capture_output=True, text=True, check=True)
            proc_cap_dis = subprocess.run(["python3", CAPSTONE_SCRIPT, self.target_rv32i, "--onlyasm"],
                                          capture_output=True, text=True, check=True)

            my_dis_output = self._clean_disasm(proc_my_dis.stdout)
            cap_dis_output = self._clean_disasm(proc_cap_dis.stdout)

            if len(my_dis_output) != len(cap_dis_output):
                raise RuntimeError(
                    f"Instruction count mismatch: My[{len(my_dis_output)}] vs Cap[{len(cap_dis_output)}]")
            return True
        except Exception as e:
            print(f"    Disassembly Matches: \033[91mFAIL\033[0m ({e})")
            return False

    def check_execution(self) -> bool:
        passed = True

        # Native Execution
        try:
            start = time.perf_counter()
            proc_native = subprocess.run([self.native_bin] + self.args, capture_output=True, text=True, check=True)
            self.time_native = time.perf_counter() - start
            native_res = int(proc_native.stdout.strip())
        except Exception as e:
            print(f"    Native Execution: \033[91mFAIL\033[0m ({e})")
            return False

        # Unicorn Execution
        try:
            start = time.perf_counter()
            proc_uni = subprocess.run([sys.executable, UNICORN_SCRIPT, self.target_rv32i] + self.args,
                                      capture_output=True, text=True)
            self.time_unicorn = time.perf_counter() - start
            if proc_uni.returncode != 0:
                raise RuntimeError(f"Unicorn execution failed: {proc_uni.stderr}")

            match = re.search(r"Result \(unsigned\): (\d+)", proc_uni.stdout)
            if not match:
                raise RuntimeError(f"Could not parse Unicorn output: {proc_uni.stdout}")

            uni_val_unsigned = int(match.group(1))
            uni_res = self._to_signed_32(uni_val_unsigned)
        except Exception as e:
            print(f"    Unicorn Execution: \033[91mFAIL\033[0m ({e})")
            return False

        # Emulator (Non-Obfuscated)
        try:
            start = time.perf_counter()
            proc_emu_non_obf = subprocess.run([EXECRV32I, "emu", self.target_rv32i] + self.args, capture_output=True,
                                              text=True, check=True)
            self.time_emu_non_obf = time.perf_counter() - start
            emu_non_obf_res = int(proc_emu_non_obf.stdout.strip())
        except Exception as e:
            print(f"    Emulator (Non-Obf) Execution: \033[91mFAIL\033[0m ({e})")
            return False

        # Emulator (Obfuscated Tool)
        try:
            start = time.perf_counter()
            proc_emu_obf_tool = subprocess.run([EXECRV32I, "emu", "--obfuscated", self.target_obf_rv32i] + self.args,
                                               capture_output=True, text=True, check=True)
            self.time_emu_obf_tool = time.perf_counter() - start
            output_lines = [line for line in proc_emu_obf_tool.stdout.splitlines() if
                            "Deobfuscated input file" not in line]
            emu_obf_tool_res = int(output_lines[-1].strip()) if output_lines else 0
        except Exception as e:
            print(f"    Emulator (Obf Tool) Execution: \033[91mFAIL\033[0m ({e})")
            return False

        # Obfuscated Binary
        try:
            start = time.perf_counter()
            proc_obf_bin = subprocess.run([self.obf_exe] + self.args, capture_output=True, text=True, check=True)
            self.time_obf_binary = time.perf_counter() - start
            obf_bin_res = int(proc_obf_bin.stdout.strip())
        except Exception as e:
            print(f"    Obfuscated Binary Execution: \033[91mFAIL\033[0m")
            print(f"    DEBUG ERROR: {e}")
            if hasattr(e, 'stderr'):
                print(f"    DEBUG STDERR: {e.stderr}")
            return False

        # Comparisons
        if native_res == uni_res:
            print("    Native vs Unicorn: \033[92mPass\033[0m")
        else:
            print(f"    Native vs Unicorn: \033[91mFAIL\033[0m (Native: {native_res}, Unicorn: {uni_res})")
            passed = False

        if emu_non_obf_res == uni_res:
            print("    Emulator (Non-Obf) vs Unicorn: \033[92mPass\033[0m")
        else:
            print(
                f"    Emulator (Non-Obf) vs Unicorn: \033[91mFAIL\033[0m (Emu: {emu_non_obf_res}, Unicorn: {uni_res})")
            passed = False

        if emu_obf_tool_res == native_res:
            print("    Emulator (Obf Tool) vs Native: \033[92mPass\033[0m")
        else:
            print(
                f"    Emulator (Obf Tool) vs Native: \033[91mFAIL\033[0m (EmuTool: {emu_obf_tool_res}, Native: {native_res})")
            passed = False

        if obf_bin_res == native_res:
            print("    Obfuscated Binary vs Native: \033[92mPass\033[0m")
        else:
            print(f"    Obfuscated Binary vs Native: \033[91mFAIL\033[0m (ObfBin: {obf_bin_res}, Native: {native_res})")
            passed = False

        return passed

    def check_deobfuscation(self) -> bool:
        try:
            deobf_bin = os.path.join(self.out_dir, "target_fn.deobf.rv32i")
            subprocess.check_call([EXECRV32I, "deobf", self.target_obf_rv32i, deobf_bin], stdout=subprocess.DEVNULL,
                                  stderr=subprocess.PIPE)

            with open(self.target_rv32i, "rb") as f:
                orig_bytes = f.read()
            with open(deobf_bin, "rb") as f:
                deobf_bytes = f.read()

            if orig_bytes != deobf_bytes:
                raise RuntimeError("Bytes mismatch")
            return True
        except Exception as e:
            print(f"    Deobfuscator Is Correct: \033[91mFAIL\033[0m ({e})")
            return False

    def run(self):
        print(f"Running {self.test_name}...")
        try:
            self.setup()
            self.build_native()
            self.build_obfuscated()
        except Exception as e:
            print(f"  \033[91mBUILD FAIL\033[0m: {e}")
            return False

        passed = True

        if self.check_disassembly():
            print("    Disassembly Matches: \033[92mPass\033[0m")
        else:
            passed = False

        if self.check_deobfuscation():
            print("    Deobfuscator Is Correct: \033[92mPass\033[0m")
        else:
            passed = False

        if not self.check_execution():
            passed = False

        return passed


class TestRunner:
    def __init__(self):
        self.config = {}
        self.tests = []
        self.scenarios = []

    def setup_environment(self):
        if not os.path.exists(OBFUSCATE_PY):
            raise RuntimeError(f"obfuscate.py not found at {OBFUSCATE_PY}")
        if not os.path.exists(EXECRV32I):
            raise RuntimeError(f"execrv32i not found at {EXECRV32I}")

        with open(TESTS_YAML, "r") as f:
            self.config = yaml.safe_load(f)

    def print_profiling_report(self):
        print("\n" + "=" * 110)
        print(f"{'PROFILING REPORT':^110}")
        print("=" * 110)

        # Header
        header = f"| {'Test Name':<20} | {'Native (s)':<10} | {'Unicorn (s)':<12} | {'Emu Non-Obf':<12} | {'Emu Obf Tool':<12} | {'Obf Binary':<12} | {'Slowdown':<10} |"
        print(header)
        print(
            "|" + "-" * 22 + "|" + "-" * 12 + "|" + "-" * 14 + "|" + "-" * 14 + "|" + "-" * 14 + "|" + "-" * 14 + "|" + "-" * 12 + "|")

        for s in self.scenarios:
            slowdown = s.time_obf_binary / s.time_native if s.time_native > 0 else 0.0
            row = f"| {s.test_name:<20} | {s.time_native:<10.6f} | {s.time_unicorn:<12.6f} | {s.time_emu_non_obf:<12.6f} | {s.time_emu_obf_tool:<12.6f} | {s.time_obf_binary:<12.6f} | {slowdown:<10.2f} |"
            print(row)
        print("=" * 110 + "\n")

    def run_all(self):
        self.setup_environment()

        tests_cfg = self.config.get("tests", [])
        passed = 0
        total = len(tests_cfg)

        print(f"\nRunning {total} tests...\n")

        for test_cfg in tests_cfg:
            scenario = TestScenario(test_cfg)
            self.scenarios.append(scenario)
            if scenario.run():
                passed += 1
            print("-" * 40)

        self.print_profiling_report()

        print(f"Summary: {passed}/{total} tests passed.")
        if passed < total:
            sys.exit(1)
        sys.exit(0)


if __name__ == "__main__":
    runner = TestRunner()
    runner.run_all()
