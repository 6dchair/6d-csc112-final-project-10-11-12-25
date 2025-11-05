#include <stdio.h>
#include <string.h>
#include "symbol_table.h"

// symbol table entry: name -> allocated register
static struct {
    char name[MAX_NAME_LEN];
    int reg;
} table[MAX_SYMBOLS];

// current number of symbols in the table
static int symbol_count = 0;


// next available register to allocate
static int next_reg = REG_MIN;


// initialize/reset the symbol table
void SymbolInit(void) {
    symbol_count = 0;
    next_reg = REG_MIN;
    for(int i = 0; i < MAX_SYMBOLS; i++)
        table[i].name[0] = '\0';
}

// get the register number associated with a symbol
// returns -1 if symbol not found
int GetRegisterOfTheSymbol(const char *name) {
    for(int i = 0; i < symbol_count; i++) {
        if(strcmp(table[i].name, name) == 0) 
            return table[i].reg;
    }
    return -1;
}

// allocate a register for a new symbol
// returns the register number, or existing reg if already allocated
// returns -1 if out of table space or registers
int AllocateRegisterForTheSymbol(const char *name) {
    int r = GetRegisterOfTheSymbol(name);
    if(r != -1)
        return r; // already allocatedd
    if(symbol_count >= MAX_SYMBOLS)
        return -1; // table is full
    if(next_reg > REG_MAX)
        return -1; // out of registers
    
    // store symbol name and assigned register
    strncpy(table[symbol_count].name, name, MAX_NAME_LEN-1);
    table[symbol_count].name[MAX_NAME_LEN-1] = '\0';
    table[symbol_count].reg = next_reg;
    symbol_count++;
    return next_reg++;
}

// print all symbols and their allocated registers
void PrintAll(FILE *out) {
    for(int i = 0; i < symbol_count; i++) {
        fprintf(out, "%s -> r%d\n", table[i].name, table[i].reg);
    }
}
