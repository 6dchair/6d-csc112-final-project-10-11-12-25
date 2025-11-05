#include "line_validator.h"


// Global variables to store declared variable names
char vars[MAX_VARS][MAX_VAR_LENGTH];
int valid_buffer_counter = 0;


char used_vars[MAX_VARS];
int registers[30];
int used_registers[30];


// Check if variable is already declared
int IsVariableDeclared(char *variableName) {
    for(int v = 0; v < valid_buffer_counter; v++)
        if(strcmp(vars[v], variableName) == 0)
            return 1;
    return 0;
}


// ================= Parses and validates an expression after '=' =========================
// allowClosing: 0 = no closing parenthesis allowed, 1 = allows closing one level of parenthesis
int AfterEqualsCheck(char *buffer, int *startCounter, int allowClosing) {
    int expectOperand = 1; // Expect variable/number/expression first
    int counter = *startCounter;

    while(buffer[counter] != '\0' && buffer[counter] != ';' && buffer[counter] != ',') {
        // Skip spaces
        while(buffer[counter] == ' ')
            counter++;

        if(expectOperand) {
            if(buffer[counter] == '(') {
                counter++; // consume '('
                if(!AfterEqualsCheck(buffer, &counter, 1)) // recursive check, allow closing ')'
                    return 0;
                expectOperand = 0; // next expect operator
                continue;
            }
            else if(isalpha(buffer[counter]) || buffer[counter] == '_') {
                int start = counter;
                // Parse variable name
                while(isalnum(buffer[counter]) || buffer[counter] == '_')
                    counter++;
                int len = counter - start;
                char var_name[MAX_VAR_LENGTH];
                strncpy(var_name, buffer + start, len);
                var_name[len] = '\0';


                // Variable must be declared
                if(!IsVariableDeclared(var_name))
                    return 0;

                expectOperand = 0; // next expect operator
            }
            else if(buffer[counter] == '-') {
                counter++;
                //Parse number
                while(isdigit(buffer[counter]))
                    counter++;
                expectOperand = 0; // next expect operator

            }
            else if(isdigit(buffer[counter])) {
                // Parse number
                while(isdigit(buffer[counter]))
                    counter++;
                expectOperand = 0; // next expect operator
            } 
            else {
                // Invalid operand
                return 0;
            }
        }
        else {
            // Operator or closing
            while(buffer[counter] == ' ')
                counter++;

            if(buffer[counter] == ')') {
                if(allowClosing) {
                    counter++; // consume ')'
                    *startCounter = counter;
                    return 1; // successfully closed parentheses
                }
                else
                    return 0; // unexpected ')'
            }
            else if(strchr("+-*/", buffer[counter])) {
                counter++; // consume operator
                expectOperand = 1; // expect operand next
            }
            else if(buffer[counter] == ';' || buffer[counter] == ',') {
                // End of this expression segment: handled by caller
                break;
            }
            else {
                // Unknown character in expression
                return 0;
            }
        }
    }

    // Must end expression after a valid operand
    if(expectOperand) {
        // still expecting operand but hit end, then invalid
        return 0;
    }

    *startCounter = counter;
    return 1;
}


// ============== Parses one variable declaration or initialization from current startIndex =============
// examples it handles: "x", "x = 2", "x = (a+3)"
// moves startIndex to after parsed variable declaration including optional initialization
// returns 1 if valid, 0 if invalid
int ParseVariableAssignment(char *buffer, int *startIndex) {
    int i = *startIndex;

    // TO DO: add more
    char *forbidden[] = {
    "int", "return", "for", "while", "if", "else",
    "char", "float", "double", "goto", "main", "double"
    };

    // Skip leading spaces
    while(buffer[i] == ' ')
        i++;

    // Parse variable name (must not start with digit or underscore)
    if(buffer[i] == '_' || isdigit(buffer[i]))
        return 0;

    int startVar = i;
    while(isalnum(buffer[i]) || buffer[i] == '_')
        i++;

    if(i == startVar) // no variable name found
        return 0;

    int len = i - startVar;

    char var_name[MAX_VAR_LENGTH];
    strncpy(var_name, buffer + startVar, len);
    var_name[len] = '\0';

    for(int i = 0; i < 11; i++) {
        if(strcmp(var_name, forbidden[i]) == 0) 
            return 0; // invalid name found
    }

    // Skip spaces after variable name
    while(buffer[i] == ' ')
        i++;

    // Check for optional initialization '='
    if(buffer[i] == '=') {
        i++; // consume '='
        while(buffer[i] == ' ')
            i++;

        // Validate the expression to the next ',' or ';'
        if(!AfterEqualsCheck(buffer, &i, 0))
            return 0; // invalid RHS expression
    }

    // Store variable if not already declared
    if(!IsVariableDeclared(var_name)) {
        strcpy(vars[valid_buffer_counter], var_name);
        valid_buffer_counter++;
    } else 
        return 0;

    // Skip spaces after variable or initialization
    while(buffer[i] == ' ')
        i++;

    *startIndex = i;

    return 1;
}

// ============================= Buffer starts with "int" =====================================
// handles parsing of full line starting with "int" keyword with multiple variable declarations separated by commas
// returns 1 if entire line is valid, otherwise 0
int StartsWithInt(char *buffer) {
    int i = 3; // position after 'int'
    int buffer_valid = 0;

    // Check input starts with "int "
    if(strncmp(buffer, "int ", 4) != 0) {
        return 0;
    }

    i++; // move past space following int

    // Parse comma-separated variable declarations until ';'
    while(1) {
        // Parse one variable (with optional initializer)
        if(!ParseVariableAssignment(buffer, &i))
            return 0;

        // Skip spaces
        while(buffer[i] == ' ')
            i++;

        if(buffer[i] == ',') {
            i++; // consume comma and continue next variable
            continue;
        } else if(buffer[i] == ';') {
            i++; // consume semicolon and end parsing variables
            buffer_valid = 1;
            break;
        } else {
            // Unexpected character after variable declaration
            return 0;
        }
    }

    // Skip trailing spaces after ';'
    while(buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r')
        i++;

    // Make sure end of line or string
    if(buffer[i] != '\0')
        return 0;

    return buffer_valid;
}


// ========================== Buffer starts with a variable ===============================
// parses lines that start directly with a variable name and initialization (no 'int')
// e.g., "x = 5;"
int StartsWithVariableName(char *buffer) {
    int i = 0;

    if(buffer[0] == '_' || isdigit(buffer[0]))
        return 0;

    int startVar = i;
    while(isalnum(buffer[i]) || buffer[i] == '_')
        i++;

    if(i == startVar)
        return 0;

    int len = i - startVar;
    char var_name[MAX_VAR_LENGTH];
    strncpy(var_name, buffer + startVar, len);
    var_name[len] = '\0';

    while(buffer[i] == ' ')
        i++;

    // Next character must be '=' for assignment
    if(buffer[i] != '=')
        return 0;
    i++; // consume '='

    while(buffer[i] == ' ')
        i++;

    // Validate expression after '=' up to ';' (no commas allowed here)
    if(!AfterEqualsCheck(buffer, &i, 0))
        return 0;

    while(buffer[i] == ' ')
        i++;

    if(buffer[i] != ';')
        return 0;

    i++; // consume ';'

    // End of line check
    while(buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r')
        i++;

    if(buffer[i] != '\0')
        return 0;

    // Variable must have been declared previously to be valid
    if(!IsVariableDeclared(var_name))
        return 0;

    return 1;
}

// ======================== Remove leading and trailing spaces from the buffer =======================
void RemoveLeadingAndTrailingSpaces(char *buffer) {
    int start = 0;
    int end = strlen(buffer) - 1;

    while(isspace(buffer[start]))
        start++;
    while(end >= start && isspace(buffer[end]))
        end--;

    int j = 0;
    for(int i = start; i <= end; i++)
        buffer[j++] = buffer[i];
    buffer[j] = '\0';
}

// ==================== Removes all spaces from the string (used for output purposes) ===================
char* RemoveAllSpaces(char *buffer, char *spacelessBuffer) {
    int i, j;
    for(i = 0, j = 0; buffer[i] != '\0'; i++) {
        if(buffer[i] == ' ')
            continue;
       spacelessBuffer[j++] = buffer[i];
    }
    spacelessBuffer[j] = '\0';
    return spacelessBuffer;
}
