#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>

// register and symbol table settings 
#define MAX_SYMBOLS   256     // maximum number of variables that can be stored
#define MAX_NAME_LEN  64      // maximum length of variable name
#define REG_MIN       1       // r1 (r0 is reserved for 0)
#define REG_MAX       30      // up to r30; 31 is also reserved

// initialize symbol table
void SymbolInit(void);

// get existing register for a variable name, or -1 if not found
int GetRegisterOfTheSymbol(const char *name);

// allocate a new register for a variable name (and return reg id)
int AllocateRegisterForTheSymbol(const char *name);

// print all variable–register mappings
void PrintAll(FILE *out);

#endif
