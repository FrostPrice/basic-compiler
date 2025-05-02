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
    SymbolTable::SymbolInfo *currentSymbol = nullptr;                            // Current symbol being processed
    SymbolTable::SymbolClassification pendingClassification = SymbolTable::NONE; // Classification of the last identifier
    SemanticTable::Types pendingType = SemanticTable::__NULL;                    // Type of the last identifier
    string pendingId = "";                                                       // Identifier of the last identifier
    vector<int> valueArraySizes;                                                 // Array dimensions of the declaration array value
    stack<int> arrayLengthsStack;                                                // Array length of inner arrays in declaration array value
    int arrayDepth = -1;                                                         // Array depth of the last identifier
    stack<int> operatorStack;                                                    // Stack for operators
    stack<int> idTypeStack;                                                      // Stack for identifier types
    bool idAlreadyDeclared = false;                                              // Flag to indicate if the identifier is already declared
    // bool isRawValue = false;                                                     // Flag to indicate if an expression is a value (eg. 1, 2.0, 'a', "string", true)

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
