# rv32i-virtObf:
This project is a not-particularly-good virtualizing obfuscator, compiling specific functions to riscv, embedding a riscv interpreter in the binary, and replacing calls to those functions with calls to the interpreter.
It also includesa a disassembler, emulator, adn custom obfuscation logic, as well as extensive test utilities.

Running:
Everything needed to obfuscate C source is in the dist.tar.gz release artifact. 
Testing has only been performed on Linux x86_64 systems, and only with these compiler versions installed:

To run the test suite:
1. Clone the repo
2. Set up cmake
3. Get the dependencies installed (just Unicorn and Capstone)
4. Adjust the paths in the top of test_validation.py
5. Run `python3 test_validation.py`

```
riscv-gnu-toolchain-bin
riscv64-gnu-toolchain-glibc-llvm-bin
riscv64-gnu-toolchain-elf-llvm-bin
riscv32-gnu-toolchain-glibc-llvm-bin
riscv32-gnu-toolchain-elf-llvm-bin
```

Samples of obfuscated binaries are included in the examples.tar.gz release artifact.

Performance Metrics:

### Raw times:

| Test Name            | Native (s) | Unicorn Emu | Emu Non-Obf  | Emu Obf Obf | Final Obfuscated | Slowdown   |
|----------------------|------------|-------------|--------------|-------------|------------------|------------|
| add_01               | 0.000738   | 0.047153    | 0.119395     | 0.122230    | 0.528179         | 715.37     |
| add_02               | 0.000505   | 0.052594    | 0.125643     | 0.141434    | 0.462121         | 915.44     |
| sub_01               | 0.000631   | 0.053558    | 0.127836     | 0.133136    | 0.514278         | 814.46     |
| mul_01               | 0.000000   | 0.000000    | 0.000000     | 0.000000    | 0.000000         | 0.00       |
| bitwise_and          | 0.000534   | 0.053016    | 0.119061     | 0.118983    | 0.443897         | 831.02     |
| bitwise_shl          | 0.000485   | 0.053315    | 0.124307     | 0.114332    | 0.451183         | 930.95     |
| simple_if_true       | 0.000500   | 0.056441    | 0.127400     | 0.137151    | 0.503940         | 1006.88    |
| simple_if_false      | 0.000492   | 0.054582    | 0.122187     | 0.122653    | 0.441194         | 896.84     |
| nested_if            | 0.000666   | 0.051520    | 0.123439     | 0.120670    | 0.413545         | 620.60     |
| switch_case_2        | 0.000780   | 0.052558    | 0.129858     | 0.137018    | 0.458866         | 588.64     |
| sum_loop             | 0.000625   | 0.050604    | 0.120427     | 0.130981    | 0.402252         | 643.81     |
| fibonacci            | 0.000709   | 0.055391    | 0.106437     | 0.105554    | 0.407453         | 574.31     |
| while_loop           | 0.000665   | 0.053960    | 0.117097     | 0.118599    | 0.458300         | 688.98     |
| array_swap           | 0.000869   | 0.049141    | 0.114131     | 0.105893    | 0.414585         | 476.99     |
| ptr_arithmetic       | 0.001072   | 0.049375    | 0.110695     | 0.116774    | 0.449902         | 419.50     |


### Perf Statistics for fibonacci test:
| Stat                    | Native      | Obfuscated    | Emulated    |
| ----------------------- | ----------- | ------------- | ----------- |
| task-clock              | 634,965     | 643,293,046   | 118,587,962 |
| context-switches        | 0           | 81            | 40          |
| cpu-migrations          | 0           | 26            | 17          |
| page-faults             | 65          | 321,563       | 2,184       |
| instructions            | 753,014     | 1,871,980,569 | 134,014,733 |
| cycles                  | 2,825,277   | 3,107,917,615 | 528,350,534 |
| stalled-cycles-frontend | 1,621,353   | 1,142,509,109 | 55,383,326  |
| branches                | 172,361     | 417,601,307   | 26,387,931  |
| branch-misses           | 20,832      | 40,290,211    | 1,488,350   |
| seconds time elapsed    | 0.001183969 | 0.647753207   | 0.121331737 |
| seconds user            | 0           | 0.105909      | 0.022827    |
| seconds sys             | 0.001221    | 0.533947      | 0.095172    |