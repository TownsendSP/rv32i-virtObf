#!/usr/bin/env python3
"""
gen_trampoline.py - Generate trampoline from header and bytecode

Usage:
    python gen_trampoline.py --header secret.h --function secret \
           --bytecode secret.rv32i --output trampoline_secret.c
"""

import argparse
import re
import sys
from pathlib import Path


def parse_function_from_header(header_text: str, func_name: str):
    """Extract function signature from header."""
    
    # Match: return_type func_name(params);
    pattern = rf'''
        ([\w\s\*]+?)              # return type
        \s+
        ({re.escape(func_name)})  # function name
        \s*\(\s*
        ([^)]*?)                  # parameters
        \s*\)\s*;
    '''
    
    match = re.search(pattern, header_text, re.VERBOSE | re.DOTALL)
    if not match:
        return None, None, None
    
    return_type = ' '.join(match.group(1).split())
    params_str = match.group(3).strip()
    
    # Parse parameters
    params = []
    if params_str and params_str.lower() != 'void':
        for param in params_str.split(','):
            param = ' '.join(param.split())
            if not param:
                continue
            
            parts = param.rsplit(None, 1)
            if len(parts) == 2:
                params.append((parts[0], parts[1]))
            else:
                m = re.match(r'(.+[*&])\s*(\w+)$', param)
                if m:
                    params.append((m.group(1), m.group(2)))
                else:
                    params.append((param, f'arg{len(params)}'))
    
    return return_type, func_name, params


def generate_trampoline(func_name: str, return_type: str, params: list, bytecode: bytes) -> str:
    """Generate trampoline C code."""
    
    param_str = ', '.join(f'{t} {n}' for t, n in params) if params else 'void'
    
    args = [f'(uint32_t){n}' for _, n in params]
    args += ['0'] * (8 - len(args))
    args_str = ', '.join(args)
    
    bytecode_lines = []
    for i in range(0, len(bytecode), 12):
        chunk = bytecode[i:i+12]
        bytecode_lines.append('    ' + ', '.join(f'0x{b:02x}' for b in chunk) + ',')
    bytecode_arr = '\n'.join(bytecode_lines)
    
    if return_type == 'void':
        call = f'rv32i_call(__bc_{func_name}, sizeof(__bc_{func_name}), {args_str});'
    elif return_type in ('int64_t', 'uint64_t'):
        call = f'return ({return_type})rv32i_call64(__bc_{func_name}, sizeof(__bc_{func_name}), {args_str});'
    else:
        call = f'return ({return_type})rv32i_call(__bc_{func_name}, sizeof(__bc_{func_name}), {args_str});'
    
    return f'''#include "emulator_api.h"

static const uint8_t __bc_{func_name}[] = {{
{bytecode_arr}
}};

{return_type} {func_name}({param_str}) {{
    {call}
}}
'''


def main():
    p = argparse.ArgumentParser(description='Generate trampoline from header and bytecode')
    p.add_argument('--header', '-H', type=Path, required=True)
    p.add_argument('--function', '-f', required=True)
    p.add_argument('--bytecode', '-b', type=Path, required=True)
    p.add_argument('--output', '-o', type=Path, required=True)
    args = p.parse_args()
    
    if not args.header.exists():
        print(f'Error: {args.header} not found', file=sys.stderr)
        sys.exit(1)
    
    return_type, name, params = parse_function_from_header(args.header.read_text(), args.function)
    if not return_type:
        print(f'Error: "{args.function}" not found in {args.header}', file=sys.stderr)
        sys.exit(1)
    
    if not args.bytecode.exists():
        print(f'Error: {args.bytecode} not found', file=sys.stderr)
        sys.exit(1)
    
    bytecode = args.bytecode.read_bytes()
    code = generate_trampoline(name, return_type, params, bytecode)
    args.output.write_text(code)
    
    sig = ', '.join(f'{t} {n}' for t, n in params) or 'void'
    print(f'{args.output}: {return_type} {name}({sig}) [{len(bytecode)} bytes]')


if __name__ == '__main__':
    main()
