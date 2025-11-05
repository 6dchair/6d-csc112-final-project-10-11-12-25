#ifndef LINE_VALIDATOR_H
#define LINE_VALIDATOR_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 1024
#define MAX_VAR_LENGTH 100
#define BUFFER 256


// Struct for variable-register pairs (reserved for future use)
typedef struct {    
    int reg;
    char var[MAX_VAR_LENGTH];
} Pair;


// Global variables (shared across translation units)
extern char vars[MAX_VARS][MAX_VAR_LENGTH];
extern int valid_buffer_counter;
extern char used_vars[MAX_VARS];
extern int registers[30];
extern int used_registers[30];


// Function prototypes
int IsVariableDeclared(char *variableName);
int AfterEqualsCheck(char *buffer, int *startCounter, int allowClosing);
int ParseVariableAssignment(char *buffer, int *startIndex);
int StartsWithInt(char *buffer);
int StartsWithVariableName(char *buffer);
void RemoveLeadingAndTrailingSpaces(char *buffer);
char* RemoveAllSpaces(char *buffer, char *spacelessBuffer);

#endif
