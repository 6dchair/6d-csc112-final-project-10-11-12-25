#ifndef MACHINE_CODE_H
#define MACHINE_CODE_H

#include <stdio.h>

// Convert assembly (simple textual asm) to mock machine-code textual file
// asm_file: input assembly file path
// out_file: output machine code (textual) path
int MachineFromAssembly(const char *asm_file, const char *out_file);

#endif


