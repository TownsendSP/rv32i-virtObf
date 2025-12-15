import sys
from capstone import *

def main():
    if len(sys.argv) < 2:
        print(f"Usage: python3 {sys.argv[0]} <file>")
        sys.exit(1)

    filename = sys.argv[1]

    try:
        with open(filename, 'rb') as f:
            code = f.read()
    except FileNotFoundError:
        print(f"Error: Could not open {filename}")
        sys.exit(1)

    # Initialize Capstone for RISC-V 32-bit
    md = Cs(CS_ARCH_RISCV, CS_MODE_RISCV32)

    # Header
    print(f"{'Addr':<10} {'Raw Hex':<12} {'Instruction'}")
    print("-" * 45)

    for i in md.disasm(code, 0x0):
        # Format raw bytes (e.g., "37000000")
        raw_bytes = i.bytes.hex()
        print(f"0x{i.address:<8x} {raw_bytes:<12} {i.mnemonic} {i.op_str}")

if __name__ == "__main__":
    main()