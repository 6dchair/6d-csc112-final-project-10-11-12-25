#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <stdio.h>
#include "parser.h"

// initialize assembly generator (resets temp reg pool)
void AssemblyInit(void);

// process a declaration or assignment statement and generate assembly to out file
// returns 1 on success, 0 on failure
int GenerateAssemblyStatement(const Statement *stmt, FILE *out);

// generate a whole program given an array of statements
void AssemblyGenerateProgram(const Statement *stmts, int count, FILE *out);

#endif
