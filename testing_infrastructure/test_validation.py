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
ABI_TO_X = {
    "zero": "x0", "ra": "x1", "sp": "x2", "gp": "x3", "tp": "x4", "t0": "x5", "t1": "x6", "t2": "x7",
    "s0": "x8", "fp": "x8", "s1": "x9", "a0": "x10", "a1": "x11", "a2": "x12", "a3": "x13",
    "a4": "x14", "a5": "x15", "a6": "x16", "a7": "x17", "s2": "x18", "s3": "x19", "s4": "x20",
    "s5": "x21", "s6": "x22", "s7": "x23", "s8": "x24", "s9": "x25", "s10": "x26", "s11": "x27",
    "t3": "x28", "t4": "x29", "t5": "x30", "t6": "x31"
}

def normalize_instruction(instr_str):
    """
    Normalize instruction string for comparison.
    - Lowercase
    - Replace ABI registers with xN
    - Convert hex immediates to decimal
    - Remove extra whitespace
    """
    instr_str = instr_str.lower().strip()
    # Strip comments
    if '#' in instr_str:
        instr_str = instr_str.split('#')[0].strip()
    
    # Replace commas and parens with spaces for easier tokenization
    # But keep them for structure if needed? 
    # Let's try to tokenize by splitting on delimiters
    tokens = re.split(r'[ ,()]+', instr_str)
    tokens = [t for t in tokens if t] # Remove empty tokens
    
    normalized_tokens = []
    for token in tokens:
        # Check if register
        if token in ABI_TO_X:
            normalized_tokens.append(ABI_TO_X[token])
        elif token.startswith('x') and token[1:].isdigit():
            normalized_tokens.append(token) # Already xN
        else:
            # Check if immediate (hex or decimal)
            try:
                val = int(token, 0) # Handles 0x and decimal
                normalized_tokens.append(str(val))
            except ValueError:
                normalized_tokens.append(token)
                
    norm_str = " ".join(normalized_tokens)
    
    # Handle aliases
    # jal x0, offset -> j offset
    if norm_str.startswith("jal x0 "):
        norm_str = "j " + norm_str[7:]
    
    # jal x1, offset -> jal offset
    if norm_str.startswith("jal x1 "):
        norm_str = "jal " + norm_str[7:]
    
    # addi x0, x0, 0 -> nop
    if norm_str == "addi x0 x0 0":
        return "nop"
        
    # addi rd, rs, 0 -> mv rd, rs
    # Regex: addi (x\d+) (x\d+) 0
    match = re.match(r"addi (x\d+) (x\d+) 0", norm_str)
    if match:
        rd, rs = match.groups()
        return f"mv {rd} {rs}"
        
    return norm_str

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
                
            # Line format: "00000000:  ff010113  ADDI x2, x2, -16"
            # We want "ADDI x2, x2, -16"
            # It starts at index 22 (approx)
            parts = line.split("  ")
            # parts[0] is "00000000:", parts[1] is "ff010113", parts[2] is "ADDI..."
            # But splitting by double space might be fragile if instruction has double space?
            # Let's use fixed width or regex
            # Address is 8 chars + ":  " = 11 chars
            # Raw is 8 chars + "  " = 10 chars
            # Total prefix is 21 chars?
            # Let's try regex
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
