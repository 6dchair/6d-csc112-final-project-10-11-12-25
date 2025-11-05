#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "assembly.h"
#include "symbol_table.h"
#include "usage_counter.h"


// temporary registers used for expression evaluation (r20–r30)
static int temp_start = 20;
static int temp_next = 20;
static int temp_max = 30;

// initialize temporary next register counter
void AssemblyInit(void) {
    temp_next = temp_start;
}

// allocate a temporary register
static int NewTempRegister(void) {
    // if(temp_next > temp_max)
    //     return temp_start;
    // return temp_next++;
    int r = temp_next;
    temp_next++;
    if(temp_next > temp_max)
        temp_next = temp_start;   // wrap back to r20
    return r;
}

// reset temporary registers between statements
static void ResetTempRegister(void) {
    temp_next = temp_start;
}

// generate immediate load
static void GenerateLoadImmediate(FILE *out, int reg, long long imm) {
    if(imm > 15) {
        // positive hex
        fprintf(out, "daddiu r%d, r0, #0x%llX\n", reg, imm);
    } else if(imm < -15) {
        // negative hex
        fprintf(out, "daddiu r%d, r0, #-%#llX\n", reg, -imm);
    } else {
        // decimal for -15...15
        fprintf(out, "daddiu r%d, r0, #%lld\n", reg, imm);
    }
}

// generate binary op
static void GenerateBinOp(FILE *out, const char *op_mnemonic, int dst, int r1, int r2) {
    fprintf(out, "%s r%d, r%d, r%d\n", op_mnemonic, dst, r1, r2);
}

// === forward declarations ===
static int Parse_E(const char **p, FILE *out);
static int Parse_T(const char **p, FILE *out);
static int Parse_F(const char **p, FILE *out);

// skip spaces: take pointer to the next character that is not whitespace
static void SkipSpacesPtr(const char **p) {
    while(**p && isspace(**p)) // while not end of string and current char is space/tab/etc.
        (*p)++; // move pointer to next character
}

// parse factor (or the lowest level elements)  (F):  F -> number | (E) | variables
static int Parse_F(const char **p, FILE *out) {
    SkipSpacesPtr(p); // skip leading spaces before processing

    // case 1: (E)
    if(**p == '(') {
        (*p)++; // skip (
        int r = Parse_E(p, out); // recursively parse inner expression
        SkipSpacesPtr(p); 
        if(**p == ')') // skip ) if present
            (*p)++;
        return r; // return the register containing the result
    }

    // case 2: immediate value or numeric literal
    if(isdigit(**p)) {
        long long val = 0;
        while(**p && isdigit(**p)) { // read all digits
            val = val * 10 + (**p - '0');
            (*p)++;
        }
        int r = NewTempRegister(); // allocate new temp reister for this number
        if(r == -1)
            r = temp_start;
        GenerateLoadImmediate(out, r, val);
        return r;
    }

    // case 3: variable name
    if(isalpha(**p) || **p == '_') {
        char name[128];
        int i = 0;

        // extract variable name 
        while(**p && (isalnum(**p) || **p == '_')) {
            if(i < (int)sizeof(name) - 1)
                name[i++] = **p;
            (*p)++;
        }
        name[i] = '\0';

        // check symbol table for its register
        int reg = GetRegisterOfTheSymbol(name);
        if(reg == -1) // if variable is yet assigned a regisetr 
            reg = AllocateRegisterForTheSymbol(name);
        return reg; 
    }

    return 0;
}

// parse term (T): T -> T * F | T / F | F
static int Parse_T(const char **p, FILE *out) {
    int left = Parse_F(p, out); // parse the first factor
    while(1) {
        SkipSpacesPtr(p);

        // Multiplication
        if(**p == '*') {
            (*p)++; // consume *
            SkipSpacesPtr(p);
            int right = Parse_F(p, out); // parse next factor
            int dst = NewTempRegister(); // get a new temp register
            if(dst == -1) 
                dst = left;
            GenerateBinOp(out, "dmul", dst, left, right);
            left = dst; 
        } // Division
        else if(**p == '/') {
            (*p)++;
            SkipSpacesPtr(p);
            int right = Parse_F(p, out);
            int dst = NewTempRegister();
            if(dst == -1)
                dst = left;
            GenerateBinOp(out, "ddiv", dst, left, right);
            left = dst;
        } // stop when +, -, or end of string is reached
        else {
            SkipSpacesPtr(p);
            if(**p == '+' || **p == '-' || **p == ';' || **p == '\0')
                break;
            else
                (*p)++; // skip unknown character
        }
    }
    return left; // return final result register
} 

// parse expression: E -> E + T | E - T | T
// this builds the full expression tree by combining terms form Parse_T(), generating instructions along the way
static int Parse_E(const char **p, FILE *out) {
    int left = Parse_T(p, out); // parse first term
    while(1) {
        SkipSpacesPtr(p);

        // Addition
        if(**p == '+') {
            (*p)++;
            SkipSpacesPtr(p);
            int right = Parse_T(p, out);
            int dst = NewTempRegister();
            if(dst == -1)
                dst = left;
            GenerateBinOp(out, "daddu", dst, left, right);
            left = dst;
        } // Subtraction
        else if(**p == '-') {
            (*p)++;AssemblyInit();
            SkipSpacesPtr(p);
            int right = Parse_T(p, out);
            int dst = NewTempRegister();
            if(dst == -1)
                dst = left;
            GenerateBinOp(out, "dsubu", dst, left, right);
            left = dst;
        } // stop when end of string is reached
        else {
            SkipSpacesPtr(p);
            if(**p == ';' || **p == '\0')
                break;
            else (*p)++;
        }
    }
    return left;
}

// Generate declaration (int x; or int x = expression;)
int AssemblyGenerateDeclaration(const Statement *stmt, FILE *out) {
    // if stmt is null or not a declaration, stop
    if(!stmt || stmt->type != STMT_DECL)
        return 0;

    // allocate register for the declared variable (lhs)
    int reg = AllocateRegisterForTheSymbol(stmt->lhs);
    if(reg == -1)
        return 0;

    // if there is an initializer (like int x = ...)
    if(stmt->rhs[0] != '\0') {
        const char *p = stmt->rhs; // pointer to start of the RHS expression
        int rres = Parse_E(&p, out); // parse and generate assebly for the RHS exp
        // if te resulting register is n
        if(rres != reg)
            fprintf(out, "daddu r%d, r%d, r0\n", reg, rres);
    } else {
        // fprintf(out, "daddiu r%d, r0, #0\n", reg);
        GenerateLoadImmediate(out, reg, 0);
    }
    return 1;
}

// Generate assignment (x = expression;)
// int AssemblyGenerateAssignment(const Statement *stmt, FILE *out) {
//     if(!stmt || stmt->type != STMT_ASSIGN) return 0;

//     int lhs_reg = GetRegisterOfTheSymbol(stmt->lhs);
//     if(lhs_reg == -1)
//         lhs_reg = AllocateRegisterForTheSymbol(stmt->lhs);
//     if(lhs_reg == -1) 
//         return 0;

//     const char *p = stmt->rhs;
//     int rres = Parse_E(&p, out);
//     if(rres != lhs_reg)
//         fprintf(out, "daddu r%d, r%d, r0\n", lhs_reg, rres);
//     return 1;
// }

// Generate assignment (x = expression;)
int AssemblyGenerateAssignment(const Statement *stmt, FILE *out) {
    if(!stmt || stmt->type != STMT_ASSIGN) return 0;

    // 1) get or allocate the LHS permanent register
    int lhs_reg = GetRegisterOfTheSymbol(stmt->lhs);
    if(lhs_reg == -1)
        lhs_reg = AllocateRegisterForTheSymbol(stmt->lhs);
    if(lhs_reg == -1)
        return 0;

    // 2) quick check: is RHS a pure integer immediate?
    const char *rhs = stmt->rhs;
    char *endptr;
    long imm_val = strtol(rhs, &endptr, 0); // supports decimal or 0x-prefixed hex

    // 3) if the entire RHS is a number (no trailing tokens), load it directly
    if(*endptr == '\0') {
        fprintf(out, "daddiu r%d, r0, #%ld\n", lhs_reg, imm_val);
        return 1;
    }

    // 4). otherwise, parse the expression normally
    int rres = Parse_E(&rhs, out);

    // 5) Move result into LHS register if needed
    if(rres != lhs_reg)
        fprintf(out, "daddu r%d, r%d, r0\n", lhs_reg, rres);

    return 1;
}


// Generate single statement
int GenerateAssemblyStatement(const Statement *stmt, FILE *out) {
    if(!stmt || !out) return 0;
    ResetTempRegister();
    if(stmt->type == STMT_DECL) 
        return AssemblyGenerateDeclaration(stmt, out);
    if(stmt->type == STMT_ASSIGN) 
        return AssemblyGenerateAssignment(stmt, out);
    return 0;
}

// Generate program
void AssemblyGenerateProgram(const Statement *stmts, int count, FILE *out) {
    //AssemblyInit();
    SymbolInit();   // ensure symbol table is reset before generateting code
    for(int i = 0; i < count; i++) {
        fprintf(out, "\n%s\n", stmts[i].raw); // print buffer for reference
        GenerateAssemblyStatement(&stmts[i], out);
    }
}
