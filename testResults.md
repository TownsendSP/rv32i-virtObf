# Tests:
Running 15 tests...

Running add_01...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running add_02...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running sub_01...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running mul_01...
BUILD FAIL: Command '['/home/tgsp/.cache/pypoetry/virtualenvs/rv321-virtobf-2AZbZpTv-py3.13/bin/python', '/home/tgsp/CLionProjects/rv321-virtObf/cmake-build-release/dist/obfuscate.py', '--main', '/home/tgsp/CLionProjects/rv321-virtObf/cmake-build-release/test_artifacts/mul_01/test_mul_01.c', '--func-impl', '/home/tgsp/CLionProjects/rv321-virtObf/testing_infrastructure/test_source/arithmetic/mul.c', '--func-header', '/home/tgsp/CLionProjects/rv321-virtObf/testing_infrastructure/test_source/arithmetic/mul.h', '--output-name', 'mul_01_obf', '--output-dir', '/home/tgsp/CLionProjects/rv321-virtObf/cmake-build-release/test_artifacts/mul_01']' returned non-zero exit status 1.
----------------------------------------
Running bitwise_and...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running bitwise_shl...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running simple_if_true...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running simple_if_false...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running nested_if...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running switch_case_2...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running sum_loop...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running factorial...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running while_loop...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running array_swap...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------
Running ptr_arithmetic...
Disassembly Matches: Pass
Deobfuscator Is Correct: Pass
Native vs Unicorn: Pass
Emulator (Non-Obf) vs Unicorn: Pass
Emulator (Obf Tool) vs Native: Pass
Obfuscated Binary vs Native: Pass
----------------------------------------

# Slowdown:

| Test Name            | Native (s) | Unicorn (s)  | Emu Non-Obf  | Emu Obf Tool | Obf Binary   | Slowdown   |
|----------------------|------------|--------------|--------------|--------------|--------------|------------|
| add_01               | 0.000657   | 0.055650     | 0.192887     | 0.163284     | 1.382871     | 2103.60    |
| add_02               | 0.000626   | 0.050877     | 0.163537     | 0.147920     | 0.927673     | 1481.19    |
| sub_01               | 0.000602   | 0.051654     | 0.160993     | 0.157131     | 0.939818     | 1560.88    |
| mul_01               | 0.000000   | 0.000000     | 0.000000     | 0.000000     | 0.000000     | 0.00       |
| bitwise_and          | 0.000460   | 0.051254     | 0.174775     | 0.153225     | 0.935977     | 2036.68    |
| bitwise_shl          | 0.000565   | 0.058778     | 0.163819     | 0.155343     | 0.933709     | 1651.39    |
| simple_if_true       | 0.000507   | 0.051554     | 0.160406     | 0.154094     | 0.912940     | 1801.35    |
| simple_if_false      | 0.000671   | 0.051503     | 0.168249     | 0.154056     | 0.926780     | 1381.49    |
| nested_if            | 0.000606   | 0.054025     | 0.168508     | 0.151233     | 0.925149     | 1527.27    |
| switch_case_2        | 0.000474   | 0.051151     | 0.154664     | 0.146166     | 0.931234     | 1965.84    |
| sum_loop             | 0.000502   | 0.051197     | 0.152807     | 0.148483     | 0.908931     | 1809.79    |
| factorial            | 0.000418   | 0.051362     | 0.154219     | 0.142837     | 0.927806     | 2221.75    |
| while_loop           | 0.000548   | 0.063083     | 0.167045     | 0.134753     | 0.915065     | 1669.07    |
| array_swap           | 0.000850   | 0.051322     | 0.150250     | 0.136634     | 0.923290     | 1086.67    |
| ptr_arithmetic       | 0.000472   | 0.049717     | 0.151079     | 0.142167     | 0.915579     | 1938.33    |


Summary: 14/15 tests passed.
