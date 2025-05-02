#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <vector>
#include <stack>

#include "Token.h"
#include "SemanticError.h"
#include "SemanticTable.hpp"
#include "SymbolTable.hpp"

using namespace std;

class Semantic
{
private:
    SymbolTable::SymbolClassification pendingClassification; // Classification of the last identifier
    SemanticTable::Types pendingType;                        // Type of the last identifier
    string pendingId = "";                                   // Identifier of the last identifier
    vector<int> symbolArrayDimensions;                       // Array dimensions of the last identifier
    vector<int> valueArrayDimensions;                        // Array dimensions of the declaration array value
    int valueArrayLength;                                    // Array length of inner arrays in declaration array value
    int currentArrayDimension = -1;                          // Current array dimension
    stack<int> operatorStack;                                // Stack for operators
    stack<int> idTypeStack;                                  // Stack for identifier types

public:
    SymbolTable symbolTable;

    void executeAction(int action, const Token *token);
    bool validateExpressionType(SemanticTable::Types expectedType);
    void reset();
    void validateExistingSymbol(SymbolTable::SymbolInfo *symbol);
    void validateSymbolClassification(SymbolTable::SymbolInfo *symbol, SymbolTable::SymbolClassification classification);
    void validateVariableType(SymbolTable::SymbolInfo *matchedSymbol);
    void validateDuplicateSymbolInSameScope(SymbolTable::SymbolInfo *symbol);
    void validateExitScope(bool isValid);
};

#endif
