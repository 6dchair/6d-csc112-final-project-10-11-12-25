#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "line_validator.h"     // syntax validation functions
#include "parser.h"        // parsing input lines into Statement structs
#include "assembly.h"        // assembly code generation from parsed statements
#include "symbol_table.h"  // variable-to-register mapping management
#include "machine_code.h"    // conversion of assembly to machine code

int main(void) {
    // ─────────────────────────────
    // 1) OPEN SOURCE FILE
    // ─────────────────────────────
    FILE *f = fopen("SAMPLE.txt", "r");    // open input source file
    if(!f) {
        printf("Unable to access the text file\n");
        return 1;                          // terminate if file cannot be opened
    }

    // ─────────────────────────────
    // 2) INITIAL SETUP
    // ─────────────────────────────
    char buffer[BUFFER];       // stores each line read from source file
    Statement stmts[1024];     // global storage for all parsed statements
    int stmt_count = 0;        // total count of valid parsed statements

    SymbolInit();                // initialize the symbol table before parsing

    // Display header for readability in terminal
    printf("----------------------------------------------------------------------------------\n");
    printf("         SOURCE -> ASSEMBLY ->  MACHINE CODE\n");
    printf("----------------------------------------------------------------------------------\n");
    // ─────────────────────────────
    // 3) READ FILE LINE BY LINE
    // ─────────────────────────────
    while(fgets(buffer, sizeof(buffer), f)) {
        RemoveLeadingAndTrailingSpaces(buffer); // trim leading/trailing spaces
        if(buffer[0] == '\0') continue;         // skip blank lines

        int isbuffervalid = 0; // flag for syntax validation result

        // ─────────────────────────────
        // 3A) DETERMINE LINE TYPE
        // ─────────────────────────────
        // If line starts with "int ", it’s a declaration.
        // Otherwise, it must be an assignment or expression.
        if(strncmp(buffer, "int ", 4) == 0)
            isbuffervalid = StartsWithInt(buffer);
        else
            isbuffervalid = StartsWithVariableName(buffer);

        // ─────────────────────────────
        // 3B) HANDLE INVALID LINES
        // ─────────────────────────────
        if(!isbuffervalid) {
            printf("[SOURCE]\n%s : Error\n", buffer);  // print invalid line message
            // separate each source block visually in terminal
           printf("----------------------------------------------------------------------------------\n");
            continue;                          // skip to next line
        }

        // ─────────────────────────────
        // 3C) VALID LINE HANDLING
        // ─────────────────────────────
        printf("[SOURCE]\n %s\n\n", buffer);  // display the valid source line

        // Parse valid line into Statement structures
        Statement parsed[32];
        int parsed_count = ParseStatement(buffer, parsed);

        // ─────────────────────────────
        // 4) PRINT ASSEMBLY CODE
        // ─────────────────────────────
        printf("[ASSEMBLY]\n");
        for(int k = 0; k < parsed_count; k++) {
            // store parsed statements globally
            if(stmt_count < (int)(sizeof(stmts)/sizeof(stmts[0])))
                stmts[stmt_count++] = parsed[k];
            // print assembly instructions for this line
            GenerateAssemblyStatement(&parsed[k], stdout);
        }
        printf("\n");

        // ─────────────────────────────
        // 5) TEMPORARY ASM OUTPUT
        // ─────────────────────────────
        // Save current line's assembly to a temp file for machine code generation
        FILE *temp = fopen("temp_buffer.asm", "w");
        if(temp) {
            for(int k = 0; k < parsed_count; k++)
                GenerateAssemblyStatement(&parsed[k], temp);
            fclose(temp);

            // ─────────────────────────────
            // 6) GENERATE MACHINE CODE
            // ─────────────────────────────
            printf("[MACHINE_CODE]\n");
            MachineFromAssembly("temp_buffer.asm", "temp_buffer.mc");

            // ─────────────────────────────
            // 7) DISPLAY MACHINE CODE
            // ─────────────────────────────
            // Print generated machine code (binary:hex) on terminal
            FILE *mcout = fopen("temp_buffer.mc", "r");
            if(mcout) {
                char line[256];
                while(fgets(line, sizeof(line), mcout))
                    printf("%s", line); // show each line from .mc
                printf("\n");
                fclose(mcout);
            }
        }

        // separate each source block visually in terminal
        printf("----------------------------------------------------------------------------------\n");
    }

    fclose(f); // close source file

    // // ─────────────────────────────
    // // 8) FINAL OUTPUT FILES
    // // ─────────────────────────────

    // // Write all assembly instructions from the program to output.asm
    // FILE *asmout = fopen("OUTPUT.asm", "w");
    // if(!asmout) {
    //     printf("Cannot open output.asm for writing\n");
    //     return 1;
    // }
    // AssemblyGenerateProgram(stmts, stmt_count, asmout);
    // fclose(asmout);

    // // Generate full machine code output file (output.mc)
    // if(!MachineFromAssembly("OUTPUT.asm", "OUTPUT.mc")) {
    //     printf("Machine code generator failed\n");
    //     return 1;
    // }

    // // Optional debug print of symbol table:
    // // printf("Declared variables (symbol table):\n");
    // // sym_print_all(stdout);

    // // Optional summary output:
    // // printf("\nGenerated full output.asm and output.mc\n");

    // return 0; // end program

    // ─────────────────────────────
    // 8): FINAL OUTPUT FILES
    // ─────────────────────────────

    // Before generating the final assembly, analyze variable usage frequency
    #include "usage_counter.h"
    AnalyzeVariableUsage(stmts, stmt_count);

    // then generate the assembly using the (possibly re-prioritized) symbol table
    FILE *asmout = fopen("OUTPUT.asm", "w");
    if(!asmout) {
        printf("Cannot open output.asm for writing\n");
        return 1;
    }
    AssemblyGenerateProgram(stmts, stmt_count, asmout);
    fclose(asmout);

}
