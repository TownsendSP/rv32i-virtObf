#!/usr/bin/env python3
import sys
import argparse
from capstone import *

def main():
    parser = argparse.ArgumentParser(description="Disassemble RISC-V 32-bit binary with Capstone")
    parser.add_argument('file', help='file to disassemble')
    parser.add_argument('--onlyasm', action='store_true', help='Only output the assembly instructions')
    args = parser.parse_args()

    filename = args.file


    try:
        with open(filename, 'rb') as f:
            code = f.read()
    except FileNotFoundError:
        print(f"Error: Could not open {filename}")
        sys.exit(1)


    # Initialize Capstone for RISC-V 32-bit
    md = Cs(CS_ARCH_RISCV, CS_MODE_RISCV32)

    # Header
    if not args.onlyasm:
        print(f"{'Addr':<10} {'Raw Hex':<12} {'Instruction'}")
        print("-" * 45)

    for i in md.disasm(code, 0x0):
        # Format raw bytes (e.g., "37000000")
        raw_bytes = i.bytes.hex()
        addr = f"{i.address:08x}"
        if args.onlyasm:
            print(f"{i.mnemonic} {i.op_str}")
        else:
            print(f"{addr} {raw_bytes:<12} {i.mnemonic} {i.op_str}")

if __name__ == "__main__":
    main()