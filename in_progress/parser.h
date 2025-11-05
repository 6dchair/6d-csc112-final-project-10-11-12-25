#ifndef PARSER_H
#define PARSER_H

#define MAX_STMT_LEN 256

typedef enum { STMT_INVALID=0, STMT_DECL, STMT_ASSIGN } StmtType;

typedef struct {
    StmtType type;
    char lhs[128];   // variable name on left
    char rhs[256];   // right-hand expression (as string), empty for plain decl
    char raw[300]; 
} Statement;

// parse a line (already trimmed) into Statement
// returns 1 if parsed, 0 if not recognized
int ParseStatement(const char *line, Statement *out);

#endif
