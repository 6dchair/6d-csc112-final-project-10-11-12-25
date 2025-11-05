#ifndef USAGE_COUNTER_H
#define USAGE_COUNTER_H

#include "parser.h"

// analyze how often variables appear in all statements
void AnalyzeVariableUsage(const Statement *stmts, int count);

#endif
