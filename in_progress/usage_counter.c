#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "usage_counter.h"
#include "symbol_table.h"

typedef struct {
    char name[MAX_NAME_LEN];
    int count;
} VarUsage;

static VarUsage vars[MAX_SYMBOLS];
static int var_count = 0;

// add occurrence of a variable
static void AddVariableUsage(const char *name) {
    if(!name || !*name)
        return;

    // ignore keywords or numbers
    if(isdigit(name[0]))
        return;

    for(int i = 0; i < var_count; i++) {
        if(strcmp(vars[i].name, name) == 0) {
            vars[i].count++;
            return;
        }
    }

    if(var_count < MAX_SYMBOLS) {
        strncpy(vars[var_count].name, name, MAX_NAME_LEN-1);
        vars[var_count].name[MAX_NAME_LEN-1] = '\0';
        vars[var_count].count = 1;
        var_count++;
    }
}

// scan variables in an expression (simple tokenizer)
static void CountVarsInExpression(const char *expr) {
    char token[128];
    int i = 0;

    while(*expr) {
        if(isalnum(*expr) || *expr == '_') {
            token[i++] = *expr;
        } else {
            if(i > 0) {
                token[i] = '\0';
                AddVariableUsage(token);
                i = 0;
            }
        }
        expr++;
    }

    if(i > 0) {
        token[i] = '\0';
        AddVariableUsage(token);
    }
}

// main usage analysis
void AnalyzeVariableUsage(const Statement *stmts, int count) {
    var_count = 0;

    // 1) Count occurrences
    for(int i = 0; i < count; i++) {
        AddVariableUsage(stmts[i].lhs);
        CountVarsInExpression(stmts[i].rhs);
    }

    // 2) Sort by descending frequency
    for(int i = 0; i < var_count - 1; i++) {
        for(int j = i + 1; j < var_count; j++) {
            if(vars[j].count > vars[i].count) {
                VarUsage tmp = vars[i];
                vars[i] = vars[j];
                vars[j] = tmp;
            }
        }
    }

    // 3) Assign registers to top vars
    SymbolInit();  // reset symbol table before assignment
    for(int i = 0; i < var_count && i < (REG_MAX - REG_MIN + 1); i++) {
        AllocateRegisterForTheSymbol(vars[i].name);
    }

    // debug print (optional)
    // printf("Variable frequency table:\n");
    // for(int i = 0; i < var_count; i++)
    //     printf("%s -> %d uses\n", vars[i].name, vars[i].count);
}
