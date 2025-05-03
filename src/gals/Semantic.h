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
    SymbolTable::SymbolInfo *currentSymbol = nullptr; // Current symbol being processed

    // TODO: Validar se todas essas variaveis de controle são realmente necessárias e podem ser trocadas pelo currentSymbol
    SymbolTable::SymbolClassification pendingClassification = SymbolTable::NONE; // Classification of the last identifier
    SemanticTable::Types pendingType = SemanticTable::__NULL;                    // Type of the last identifier
    vector<int> valueArraySizes;                                                 // Array dimensions of the declaration array value
    stack<int> arrayLengthsStack;                                                // Array length of inner arrays in declaration array value
    int arrayDepth = -1;                                                         // Array depth of the last identifier
    stack<int> operatorStack;                                                    // Stack for operators
    stack<int> idTypeStack;                                                      // Stack for identifier types
    bool idAlreadyDeclared = false;                                              // Flag to indicate if the identifier is already declared
    // bool isRawValue = false;                                                     // Flag to indicate if an expression is a value (eg. 1, 2.0, 'a', "string", true)

public:
    SymbolTable symbolTable;
    void resetControllVariables()
    {
        if (this->currentSymbol != nullptr)
        {
            delete this->currentSymbol;
        }

        this->currentSymbol = new SymbolTable::SymbolInfo();

        while (!this->operatorStack.empty())
            this->operatorStack.pop();

        while (!this->idTypeStack.empty())
            this->operatorStack.pop();
    }

    void executeAction(int action, const Token *token);

    // Validation methods
    void validateExpressionType(SemanticTable::Types expectedType)
    {
        if (this->symbolTable.expressionStack.empty())
        {
            throw SemanticError(SemanticError::ExpressionStackEmpty());
        }

        int actualType = this->symbolTable.expressionStack.top();
        this->symbolTable.expressionStack.pop();

        if (actualType != expectedType)
        {
            throw SemanticError(SemanticError::TypeMismatch(expectedType, static_cast<SemanticTable::Types>(actualType)));
        }
    }

    void validateExistingSymbol(SymbolTable::SymbolInfo *symbol)
    {
        if (symbol == nullptr)
        {
            throw SemanticError(SemanticError::SymbolNotFound("unknown")); // If id unknown
        }
    }

    void validateSymbolClassification(SymbolTable::SymbolInfo *symbol, SymbolTable::SymbolClassification classification)
    {
        if (symbol->symbolClassification != classification)
        {
            throw SemanticError(SemanticError::SymbolNotOfClassification(symbol->id));
        }
    }

    void validateDuplicateSymbolInSameScope(SymbolTable::SymbolInfo *matchedSymbol)
    {
        if (matchedSymbol->scope == this->currentSymbol->scope)
        {
            throw SemanticError(SemanticError::DuplicateSymbol(matchedSymbol->id));
        }
    }

    void validateVariableType(SymbolTable::SymbolInfo *matchedSymbol)
    {
        if (matchedSymbol->dataType != this->currentSymbol->dataType)
        {
            throw SemanticError(SemanticError::TypeAssignmentMismatch(matchedSymbol->id, matchedSymbol->dataType));
        }
    }

    void validateReturnStatementScope(SymbolTable::SymbolInfo *currentScopeSymbol)
    {
        if (currentScopeSymbol == nullptr)
        {
            throw SemanticError(SemanticError::ReturnOutsideFunction());
        }
    }

    void validateIfVariableIsDeclared(SymbolTable::SymbolInfo *currentSymbol, bool isDeclared)
    {
        if (isDeclared)
        {
            throw SemanticError(SemanticError::InputUndeclared(currentSymbol->id));
        }
    }

    void validateIsVariable(SymbolTable::SymbolInfo *currentSymbol)
    {
        if (currentSymbol->symbolClassification != SymbolTable::VARIABLE)
        {
            throw SemanticError(SemanticError::InputNonVariable(currentSymbol->id));
        }
    }
};

#endif
