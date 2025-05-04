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

    SemanticTable::Types pendingType = SemanticTable::Types::__NULL; // Type of the last identifier
    bool isDeclarating = false;                                      // Flag to indicate if the identifier is being declared
    int declarationScope = -1;                                       // Scope of the declaration

    vector<int> valueArraySizes;  // Array dimensions of the declaration array value
    stack<int> arrayLengthsStack; // Array length of inner arrays in declaration array value
    int arrayDepth = -1;          // Array depth of the last identifier

    // bool isRawValue = false;                                                     // Flag to indicate if an expression is a value (eg. 1, 2.0, 'a', "string", true)

public:
    vector<string> warnings; // Warnings generated during semantic analysis
    void reportWarning(const string &msg)
    {
        warnings.push_back(msg);
    }

    SymbolTable symbolTable;

    void resetControllVariables()
    {
        if (this->currentSymbol != nullptr)
        {
            delete this->currentSymbol;
        }

        this->currentSymbol = new SymbolTable::SymbolInfo();

        while (!this->symbolTable.expressionStack.empty())
            this->symbolTable.expressionStack.pop();
    }

    void executeAction(int action, const Token *token);

    // Validation methods
    void validateExpressionType(SemanticTable::Types expectedType)
    {
        SymbolTable::ExpressionsEntry currentValue = this->symbolTable.expressionStack.top();
        this->symbolTable.expressionStack.pop();

        int compat = SemanticTable::atribType(expectedType, currentValue.entryType);
        if (compat == SemanticTable::ERR)
        {
            throw SemanticError(SemanticError::TypeMismatch(expectedType, static_cast<SemanticTable::Types>(currentValue.entryType)));
        }

        if (compat == SemanticTable::WAR)
        {
            this->reportWarning("Warning: possible data loss when assigning value of type " +
                                to_string(currentValue.entryType) +
                                " to variable of type " +
                                to_string(expectedType));
        }
    }

    void validateSymbolClassification(SymbolTable::SymbolInfo *symbol, SymbolTable::SymbolClassification classification)
    {
        if (symbol->symbolClassification != classification)
        {
            throw SemanticError(SemanticError::SymbolNotOfClassification(symbol->id));
        }
    }

    bool validateDuplicateSymbolInSameScope(SymbolTable::SymbolInfo *matchedSymbol)
    {
        if (matchedSymbol->scope == this->currentSymbol->scope)
        {
            throw SemanticError(SemanticError::DuplicateSymbol(matchedSymbol->id));
        }

        return true; // No duplicate symbol in the same scope
    }

    void validateReturnStatementScope(SymbolTable::SymbolInfo *currentScopeSymbol)
    {
        if (currentScopeSymbol == nullptr)
        {
            throw SemanticError(SemanticError::ReturnOutsideFunction());
        }
    }

    void validateIfVariableIsDeclared(SymbolTable::SymbolInfo *currentSymbol, string id)
    {
        if (!currentSymbol)
        {
            throw SemanticError(SemanticError::SymbolUndeclared(id));
        }
    }

    void validateIsVariable(SymbolTable::SymbolInfo *currentSymbol)
    {
        if (currentSymbol->symbolClassification != SymbolTable::VARIABLE)
        {
            throw SemanticError(SemanticError::InputNonVariable(currentSymbol->id));
        }
    }

    bool validateIsNumericOperator(SemanticTable::Types type)
    {
        return type == SemanticTable::Types::INT || type == SemanticTable::Types::FLOAT || type == SemanticTable::Types::DOUBLE;
    }

    void validateOneOfTypes(std::initializer_list<SemanticTable::Types> types)
    {

        if (symbolTable.expressionStack.empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        SymbolTable::ExpressionsEntry top = symbolTable.expressionStack.top();

        for (SemanticTable::Types t : types)
        {
            if (SemanticTable::atribType(t, top.entryType) != SemanticTable::ERR)
            {
                symbolTable.expressionStack.pop();
                return; // Valid type
            }
        }

        throw SemanticError("Expression type '" + to_string(top.entryType) +
                            "' not allowed here.");
    }
};

#endif
