#!/usr/bin/env bash

BINJA="/opt/binaryninja/binaryninja"
CUTTER="/usr/bin/cutter"

# accept file and disassembler as commandline arguments
if [ $# -ne 2 ]; then
    echo "Error: Please provide a file and a disassembler ('binja' or 'cutter') as arguments."
    exit 1
fi
FILE=$1
DISASM=$2

if [ "$DISASM" = "binja" ]; then
    # Load options for a raw RISC‑V binary
    LOAD_OPTS='{
      "loader": {
        "mapped": true,
        "entryPointOffset": 0,
        "imageBase": 0
      },
      "platform": "riscv32-generic"
    }'

    # Launch Binary Ninja
    "$BINJA" --load-options "$LOAD_OPTS" "$FILE"
elif [ "$DISASM" = "cutter" ]; then
    # Launch cutter with specified options
    "$CUTTER" "$FILE" -A 1 -a riscv -b 32 -o linux -B 0x40000 -F bin --base 1024
else
    echo "Error: Invalid disassembler. Please choose 'binja' or 'cutter'."
    exit 1
fi
