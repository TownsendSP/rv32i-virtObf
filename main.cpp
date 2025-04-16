#include <iostream>

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the
    // <b>lang</b> variable name to see how CLion can help you rename it.
    // <table><thead><tr><th>Immediate <code>[31:25]</code></th><th>Source Register rs2 <code>[24:20]</code></th><th>Source Register rs1 <code>[19:15]</code></th><th>funct3 <code>[14:12]</code></th><th>Immediate <code>[11:7]</code></th><th>Opcode <code>[6:0]</code></th><th>Instruction</th></tr></thead><tbody><tr><td><code>imm[11:5]</code></td><td>rs2</td><td>rs1</td><td>000</td><td><code>imm[4:0]</code></td><td>0100011</td><td>SB</td></tr><tr><td><code>imm[11:5]</code></td><td>rs2</td><td>rs1</td><td>001</td><td><code>imm[4:0]</code></td><td>0100011</td><td>SH</td></tr><tr><td><code>imm[11:5]</code></td><td>rs2</td><td>rs1</td><td>010</td><td><code>imm[4:0]</code></td><td>0100011</td><td>SW</td></tr></tbody>table>

    auto lang = "C++";
    std::cout << "Hello and welcome to " << lang << "!\n";

    for (int i = 1; i <= 5; i++) {
        // TIP Press <shortcut actionId="Debug"/> to start debugging your code.
        // We have set one <icon src="AllIcons.Debugger.Db_set_breakpoint"/>
        // breakpoint for you, but you can always add more by pressing
        // <shortcut actionId="ToggleLineBreakpoint"/>.
        std::cout << "i = " << i << std::endl;
    }
    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.