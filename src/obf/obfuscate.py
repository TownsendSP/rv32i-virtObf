#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
import tempfile
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
    parser.add_argument("--output-dir", help="Output directory (optional)")
    parser.add_argument("--output-name", required=True, help="Name of final executable")

    args = parser.parse_args()

    # Absolute paths for inputs
    main_src = Path(args.main).resolve()
    func_impl = Path(args.func_impl).resolve()
    func_header = Path(args.func_header).resolve()

# Tool paths
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

    # Determine build directory
    if args.output_dir:
        output_dir = Path(args.output_dir).resolve()
        build_dir = output_dir / "cmake_output"
        if not build_dir.exists():
            build_dir.mkdir(parents=True)
    else:
        output_dir = None
        build_dir_obj = tempfile.TemporaryDirectory()
        build_dir = Path(build_dir_obj.name)

    print(f"--- Building in {build_dir} ---")

    try:
        shutil.copy(main_src, build_dir / main_src.name)
        shutil.copy(func_impl, build_dir / func_impl.name)
        shutil.copy(func_header, build_dir / func_header.name)
        shutil.copy(emulator_lib, build_dir / "libemulator_static.a")
        shutil.copy(emulator_header, build_dir / "emulator_api.h")

        # Instantiate CMakeLists.txt
        with open(template_file, "r") as f:
            template = f.read()

        content = template.replace("@TARGET_FN_SRC@", func_impl.name)
        content = content.replace("@MAIN_SRC@", main_src.name)
        content = content.replace("@OUTPUT_NAME@", args.output_name)

        with open(build_dir / "CMakeLists.txt", "w") as f:
            f.write(content)

        print("--- Compiling Target Function ---")
        run_command(["cmake", "."], cwd=build_dir, verbose=args.verbose)
        run_command(["make", "compile_target_fn"], cwd=build_dir, verbose=args.verbose)

        print("--- Obfuscating ---")
        input_bin = build_dir / "target_fn.rv32i"
        output_bin = build_dir / "target_fn.obf.rv32i"
        run_command([str(execrv32i), "obf", str(input_bin), str(output_bin)], verbose=args.verbose)

        print("--- Generating Trampoline ---")
        trampoline_src = build_dir / "trampoline.c"
        func_name = func_impl.stem

        run_command([sys.executable, str(gen_trampoline),
                     "--header", str(build_dir / func_header.name),
                     "--function", func_name,
                     "--bytecode", str(output_bin),
                     "--output", str(trampoline_src)], verbose=args.verbose)

        print("--- Linking Final Executable ---")
        # Re-run cmake to detect trampoline.c
        run_command(["cmake", "."], cwd=build_dir, verbose=args.verbose)
        run_command(["make", args.output_name], cwd=build_dir, verbose=args.verbose)

        final_bin = build_dir / args.output_name

        if output_dir:
            print(f"--- Copying artifacts to {output_dir} ---")
            shutil.copy(trampoline_src, output_dir / "trampoline.c")
            shutil.copy(build_dir / "CMakeLists.txt", output_dir / "CMakeLists.txt")
            shutil.copy(input_bin, output_dir / "target_fn.rv32i")
            shutil.copy(output_bin, output_dir / "target_fn.obf.rv32i")
            shutil.copy(final_bin, output_dir / args.output_name)
            print(f"Success! Output: {output_dir / args.output_name}")
        else:
            print(f"--- Copying executable to {cwd} ---")
            shutil.copy(final_bin, cwd / args.output_name)
            print(f"Success! Output: {cwd / args.output_name}")

    finally:
        if not args.output_dir:
            build_dir_obj.cleanup()


if __name__ == "__main__":
    main()


