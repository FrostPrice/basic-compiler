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
    SymbolTable::SymbolClassification pendingClassification = SymbolTable::NONE; // Classification of the last identifier

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
