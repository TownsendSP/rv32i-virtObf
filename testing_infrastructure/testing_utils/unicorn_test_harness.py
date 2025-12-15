#!/usr/bin/env python3
"""
Test RV32I bare-metal binaries using Unicorn Engine

Usage:
    python3 test_rv32i.py <binary.rv32i> [arg0] [arg1] ... [-v]

Example:
    python3 test_rv32i.py cmake-build-debug/output/riscv_test.rv32i 5 3 -v

Install Unicorn:
    pip install unicorn
"""

import os
import argparse

try:
    from unicorn import *
    from unicorn.riscv_const import *
except ImportError:
    print("ERROR: Unicorn Engine not installed!")
    print("Install with: pip install unicorn")
    exit(1)


def test_rv32i_function(binary_path, args=None, verbose=False):
    """
    Execute a bare-metal RV32I binary with Unicorn Engine

    Args:
        binary_path: Path to .rv32i binary file
        args: List of integer arguments (passed in a0, a1, a2, ...)
        verbose: Print debug information

    Returns:
        Integer result from a0 register, or None on error
    """
    if args is None:
        args = []

    # Create RISC-V 32-bit emulator
    try:
        mu = Uc(UC_ARCH_RISCV, UC_MODE_RISCV32)
    except UcError as e:
        print(f"ERROR: Failed to initialize Unicorn: {e}")
        return None

    # Memory configuration
    CODE_ADDRESS = 0x10000
    CODE_SIZE = 1024 * 1024  # 1MB
    STACK_ADDRESS = 0x200000  # Place stack after code region (0x10000 + 0x100000 = 0x110000, so 0x200000 is safe)
    STACK_SIZE = 64 * 1024   # 64KB

    # Map memory for code
    try:
        mu.mem_map(CODE_ADDRESS, CODE_SIZE)
        if verbose:
            print(f"Mapped code memory: 0x{CODE_ADDRESS:08x} - 0x{CODE_ADDRESS + CODE_SIZE:08x}")
    except UcError as e:
        print(f"ERROR: Failed to map code memory: {e}")
        return None

    # Map memory for stack
    try:
        mu.mem_map(STACK_ADDRESS, STACK_SIZE)
        if verbose:
            print(f"Mapped stack memory: 0x{STACK_ADDRESS:08x} - 0x{STACK_ADDRESS + STACK_SIZE:08x}")
    except UcError as e:
        print(f"ERROR: Failed to map stack memory: {e}")
        return None

    # Load binary
    try:
        with open(binary_path, 'rb') as f:
            code = f.read()
        mu.mem_write(CODE_ADDRESS, code)
        if verbose:
            print(f"Loaded {len(code)} bytes from {binary_path}")
    except Exception as e:
        print(f"ERROR: Failed to load binary: {e}")
        return None

    # Initialize registers
    # x0 is hardwired to 0 (read-only)
    # x2 (sp) = stack pointer
    stack_top = STACK_ADDRESS + STACK_SIZE - 16  # Leave some space
    mu.reg_write(UC_RISCV_REG_SP, stack_top)

    # Set up arguments in registers a0-a7 (x10-x17)
    arg_regs = [
        UC_RISCV_REG_A0, UC_RISCV_REG_A1, UC_RISCV_REG_A2, UC_RISCV_REG_A3,
        UC_RISCV_REG_A4, UC_RISCV_REG_A5, UC_RISCV_REG_A6, UC_RISCV_REG_A7
    ]

    for i, arg in enumerate(args[:8]):  # Max 8 register arguments
        mu.reg_write(arg_regs[i], int(arg))
        if verbose:
            print(f"Set a{i} (x{10+i}) = {arg}")

    # Set return address (ra/x1) to a recognizable value
    # When function returns (ret), it jumps to ra
    RETURN_ADDRESS = 0xDEAD0000
    mu.reg_write(UC_RISCV_REG_RA, RETURN_ADDRESS)

    if verbose:
        print(f"\nStarting execution at 0x{CODE_ADDRESS:08x}")
        print(f"Stack pointer: 0x{stack_top:08x}")
        print(f"Return address: 0x{RETURN_ADDRESS:08x}")

    # Execute the code
    try:
        # Execute until we hit the return address or end of code
        mu.emu_start(CODE_ADDRESS, CODE_ADDRESS + len(code))
    except UcError as e:
        # Check if we hit the return address (normal termination)
        pc = mu.reg_read(UC_RISCV_REG_PC)
        if pc == RETURN_ADDRESS:
            if verbose:
                print(f"Execution completed (returned to 0x{RETURN_ADDRESS:08x})")
        else:
            print(f"ERROR: Execution failed at PC=0x{pc:08x}: {e}")
            return None

    # Get return value from a0 (x10)
    result = mu.reg_read(UC_RISCV_REG_A0)

    if verbose:
        print(f"\nResult in a0: {result} (0x{result:08x})")
        print(f"Result (signed): {result if result < 0x80000000 else result - 0x100000000}")

    return result


def main():
    parser = argparse.ArgumentParser(description="Test RV32I bare-metal binaries using Unicorn Engine")
    parser.add_argument("binary", help="Path to the .rv32i binary file")
    parser.add_argument("args", nargs="*", type=lambda x: int(x, 0), help="Integer arguments (supports decimal and hex)")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output")
    args = parser.parse_args()

    binary_path = args.binary
    verbose = args.verbose

    # Check if file exists
    if not os.path.exists(binary_path):
        print(f"ERROR: File not found: {binary_path}")
        exit(1)

    # Run the test
    print(f"Testing: {binary_path}")
    if args.args:
        print(f"Arguments: {args.args}")

    result = test_rv32i_function(binary_path, args.args, verbose=verbose)

    if result is not None:
        # Print result as both unsigned and signed
        print(f"\n Execution successful!")
        print(f"Result (unsigned): {result}")
        if result >= 0x80000000:
            signed_result = result - 0x100000000
            print(f"Result (signed): {signed_result}")
    else:
        print("\n Execution failed!")
        exit(1)


if __name__ == "__main__":
    main()

