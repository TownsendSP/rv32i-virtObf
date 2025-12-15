import unittest
import subprocess
import os
import sys
import glob
import re

# Paths
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXECRV32I = os.path.join(PROJECT_ROOT, "build", "execrv32i")
RV32I_FILES_DIR = os.path.join(PROJECT_ROOT, "testing_infrastructure", "rv32i_files")
CAPSTONE_SCRIPT = os.path.join(PROJECT_ROOT, "testing_infrastructure", "testing_utils", "capstone_disasm.py")
UNICORN_SCRIPT = os.path.join(PROJECT_ROOT, "testing_infrastructure", "testing_utils", "unicorn_test_harness.py")

# Register mapping (ABI -> xN)
class TestValidation(unittest.TestCase):
    def setUp(self):
        self.assertTrue(os.path.exists(EXECRV32I), f"execrv32i not found at {EXECRV32I}")
        self.rv32i_files = glob.glob(os.path.join(RV32I_FILES_DIR, "*.rv32i"))

    def test_disassembler(self):
        """Compare execrv32i disassembler output with Capstone"""
        print("\n[Disassembler Test]")
        for rv32i_file in self.rv32i_files:
            filename = os.path.basename(rv32i_file)
            print(f"Testing {filename}...", end=" ", flush=True)
            with self.subTest(file=filename):
                # Run execrv32i
                proc_exec = subprocess.run([EXECRV32I, "dis", rv32i_file], capture_output=True, text=True)
                self.assertEqual(proc_exec.returncode, 0, f"execrv32i failed: {proc_exec.stderr}")
                
                # Run Capstone
                proc_cap = subprocess.run([sys.executable, CAPSTONE_SCRIPT, rv32i_file], capture_output=True, text=True)
                self.assertEqual(proc_cap.returncode, 0, f"Capstone script failed: {proc_cap.stderr}")
                
                # Parse outputs
                exec_lines = self._parse_execrv32i_disasm(proc_exec.stdout)
                cap_lines = self._parse_capstone_disasm(proc_cap.stdout)
                
                # Compare number of instructions
                self.assertEqual(len(exec_lines), len(cap_lines), "Instruction count mismatch")
                
                # Compare each instruction
                for i, (exec_inst, cap_inst) in enumerate(zip(exec_lines, cap_lines)):
                    norm_exec = normalize_instruction(exec_inst)
                    norm_cap = normalize_instruction(cap_inst)
                    self.assertEqual(norm_exec, norm_cap, f"Mismatch at index {i}:\nExec: {exec_inst} -> {norm_exec}\nCap:  {cap_inst} -> {norm_cap}")
            print("PASS")

    def test_emulator(self):
        """Compare execrv32i emulator output with Unicorn"""
        print("\n[Emulator Test]")
        
        # Known arguments for specific files
        known_args = {
            "complicated_fn_riscv_test.rv32i": [["3", "5"]],
            "only_fn_riscv_test.rv32i": [["5", "6"]],
            "riscv_test.rv32i": [[]],
            "doOperation.rv32i": [["10", "20", "1"]]
        }
        
        for rv32i_file in self.rv32i_files:
            filename = os.path.basename(rv32i_file)
            
            # Determine args to test
            args_list = known_args.get(filename, [[]]) # Default to empty args if unknown
            
            for args in args_list:
                args_str = " ".join(args)
                print(f"Testing {filename} args=[{args_str}]...", end=" ", flush=True)
                
                with self.subTest(file=filename, args=args):
                    # Run Unicorn to get ground truth
                    cmd_uni = [sys.executable, UNICORN_SCRIPT, rv32i_file] + args
                    proc_uni = subprocess.run(cmd_uni, capture_output=True, text=True)
                    
                    # Parse Unicorn result
                    match = re.search(r"Result \(unsigned\): (\d+)", proc_uni.stdout)
                    if not match:
                        if "Execution failed" in proc_uni.stdout:
                            self.fail(f"Unicorn execution failed: {proc_uni.stdout}")
                        self.fail(f"Could not parse Unicorn output: {proc_uni.stdout}")
                    
                    expected_result = int(match.group(1))
                    
                    # Run execrv32i
                    cmd_exec = [EXECRV32I, "emu", rv32i_file] + args
                    proc_exec = subprocess.run(cmd_exec, capture_output=True, text=True)
                    self.assertEqual(proc_exec.returncode, 0, f"execrv32i failed: {proc_exec.stderr}")
                    
                    # Parse execrv32i result (last line)
                    lines = proc_exec.stdout.strip().splitlines()
                    self.assertTrue(len(lines) > 0, "execrv32i produced no output")
                    try:
                        actual_result = int(lines[-1])
                    except ValueError:
                        self.fail(f"Could not parse execrv32i output: {lines[-1]}")
                    
                    self.assertEqual(actual_result, expected_result, f"Result mismatch for {filename} {args}")
                print("PASS")

    def _parse_execrv32i_disasm(self, output):
        instructions = []
        start_parsing = False
        for line in output.splitlines():
            if line.startswith("---"):
                start_parsing = True
                continue
            if not start_parsing:
                continue
            if not line.strip():
                continue
                
            match = re.match(r"[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+(.*)", line)
            if match:
                instructions.append(match.group(1))
        return instructions

    def _parse_capstone_disasm(self, output):
        instructions = []
        start_parsing = False
        for line in output.splitlines():
            if line.startswith("---"):
                start_parsing = True
                continue
            if not start_parsing:
                continue
            if not line.strip():
                continue
            
            # Line format: "0x0        130101ff     addi sp, sp, -0x10"
            # Split by whitespace, but instruction might have spaces
            parts = line.split(maxsplit=2)
            if len(parts) >= 3:
                instructions.append(parts[2])
        return instructions

if __name__ == "__main__":
    unittest.main()
