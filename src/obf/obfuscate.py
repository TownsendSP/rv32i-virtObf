#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

def run_command(cmd, cwd=None, verbose=False):
    if verbose:
        print(f"Running: {' '.join(cmd)} (cwd: {cwd})")
    try:
        subprocess.check_call(cmd, cwd=cwd)
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {e}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Obfuscation Pipeline")
    parser.add_argument("--verbose", action="store_true", help="Enable verbose output")
    parser.add_argument("--main", required=True, help="Path to target.c (contains main)")
    parser.add_argument("--func-impl", required=True, help="Path to target_fn.c (implementation)")
    parser.add_argument("--func-header", required=True, help="Path to target_fn.h (header)")
    parser.add_argument("--output-dir", required=True, help="Output directory")
    parser.add_argument("--output-name", required=True, help="Name of final executable")
    
    args = parser.parse_args()
    
    # Absolute paths for inputs
    main_src = Path(args.main).resolve()
    func_impl = Path(args.func_impl).resolve()
    func_header = Path(args.func_header).resolve()
    output_dir = Path(args.output_dir).resolve()
    
    # Tools (assumed to be in current directory)
    cwd = Path.cwd()
    execrv32i = cwd / "execrv32i"
    gen_trampoline = cwd / "gen_trampoline.py"
    template_file = cwd / "CMakeLists.txt.template"
    emulator_lib = cwd / "libemulator_static.a"
    emulator_header = cwd / "emulator_api.h"
    
    # Check tools
    for tool in [execrv32i, gen_trampoline, template_file, emulator_lib, emulator_header]:
        if not tool.exists():
            print(f"Error: Required tool not found: {tool}")
            sys.exit(1)
            
    # 1. Create output directory
    if not output_dir.exists():
        output_dir.mkdir(parents=True)
    
    # 2. Copy files
    shutil.copy(main_src, output_dir / main_src.name)
    shutil.copy(func_impl, output_dir / func_impl.name)
    shutil.copy(func_header, output_dir / func_header.name)
    shutil.copy(emulator_lib, output_dir / "libemulator_static.a")
    shutil.copy(emulator_header, output_dir / "emulator_api.h")
    
    # 3. Instantiate CMakeLists.txt
    with open(template_file, "r") as f:
        template = f.read()
        
    content = template.replace("@TARGET_FN_SRC@", func_impl.name)
    content = content.replace("@MAIN_SRC@", main_src.name)
    content = content.replace("@OUTPUT_NAME@", args.output_name)
    
    with open(output_dir / "CMakeLists.txt", "w") as f:
        f.write(content)
        
    # 4. Compile target function
    print("--- Compiling Target Function ---")
    run_command(["cmake", "."], cwd=output_dir, verbose=args.verbose)
    run_command(["make", "compile_target_fn"], cwd=output_dir, verbose=args.verbose)
    
    # 5. Obfuscate
    print("--- Obfuscating ---")
    input_bin = output_dir / "target_fn.rv32i"
    output_bin = output_dir / "target_fn.obf.rv32i"
    run_command([str(execrv32i), "obf", str(input_bin), str(output_bin)], verbose=args.verbose)
    
    # 6. Generate Trampoline
    print("--- Generating Trampoline ---")
    trampoline_src = output_dir / "trampoline.c"
    # Need to extract function name from header or assume it matches filename?
    # gen_trampoline.py needs function name.
    # We can parse the header to find the function name? 
    # Or just assume the user provided the function name?
    # Wait, gen_trampoline.py parses the header to find the signature, but it needs the function name as argument.
    # The user didn't provide function name in arguments!
    # I should add --func-name argument?
    # Or try to guess?
    # Let's check gen_trampoline.py again.
    # It takes --function.
    # I missed this in the plan.
    # I will assume the function name is the stem of the func-impl file?
    # e.g. doOperation.c -> doOperation
    func_name = func_impl.stem
    
    run_command([sys.executable, str(gen_trampoline), 
                 "--header", str(output_dir / func_header.name),
                 "--function", func_name,
                 "--bytecode", str(output_bin),
                 "--output", str(trampoline_src)], verbose=args.verbose)
                 
    # 7. Link Final Executable
    print("--- Linking Final Executable ---")
    # Re-run cmake to detect trampoline.c
    run_command(["cmake", "."], cwd=output_dir, verbose=args.verbose)
    run_command(["make", args.output_name], cwd=output_dir, verbose=args.verbose)
    
    print(f"--- Success! Output: {output_dir / args.output_name} ---")

if __name__ == "__main__":
    main()
