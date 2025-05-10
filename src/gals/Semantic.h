#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <vector>
#include <stack>
#include <optional>

#include "Token.h"
#include "SemanticError.h"
#include "SemanticTable.hpp"
#include "SymbolTable.hpp"

using namespace std;

class Semantic
{
private:
    SymbolTable::SymbolInfo *currentSymbol = nullptr;     // Current symbol being processed
    SymbolTable::SymbolInfo *declarationSymbol = nullptr; // Current symbol being processed

    stack<tuple<SymbolTable::SymbolInfo *, int>> symbolEvaluateStack; // Stack to manage symbols in expressions

    SemanticTable::Types pendingType = SemanticTable::Types::__NULL; // Type of the last identifier

    vector<vector<int>> valueArraySizes;   // Array dimensions of the declaration array value
    stack<int> arrayLengthsStack;          // Array length of inner arrays in declaration array value
    int arrayDepth = -1;                   // Array depth of the last identifier
    int parametersCountInFuncCall = 0;     // Number of parameters in the function call
    SemanticTable::Types switchResultType; // Type of the switch expression
    int loopDepth = 0;                     // Loop depth for break and continue statements
    int switchDepth = 0;                   // Switch depth for break and continue statements

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
            throw SemanticError(SemanticError::TypeMismatch(expectedType, currentValue.entryType));
        }

        if (compat == SemanticTable::WAR)
        {
            this->reportWarning("Warning: possible data loss when assigning value of type " +
                                to_string(currentValue.entryType) +
                                " to variable of type " +
                                to_string(expectedType));
        }
    }
    SemanticTable::Types validateGeneralExpression(SemanticTable::Types expectedType)
    {
        if (symbolTable.expressionStack.empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        size_t stackSize = symbolTable.expressionStack.size();

        SemanticTable::Types resultType = SemanticTable::Types::__NULL;

        if (stackSize >= 3) // Try binary: operand1, op, operand2
        {
            SymbolTable::ExpressionsEntry right = symbolTable.expressionStack.top();
            symbolTable.expressionStack.pop();

            SymbolTable::ExpressionsEntry operation = symbolTable.expressionStack.top();
            symbolTable.expressionStack.pop();

            SymbolTable::ExpressionsEntry left = symbolTable.expressionStack.top();
            symbolTable.expressionStack.pop();

            if (left.entryType == SemanticTable::Types::__NULL || right.entryType == SemanticTable::Types::__NULL)
            {
                throw SemanticError("Invalid binary operation between types " +
                                    std::to_string(left.entryType) + " and " +
                                    std::to_string(right.entryType) + " using op " +
                                    std::to_string(operation.binaryOperation));
            }

            int result = SemanticTable::resultBinaryType(left.entryType, right.entryType, operation.binaryOperation);

            if (result == SemanticTable::ERR)
            {
                throw SemanticError("Invalid binary operation between types " +
                                    std::to_string(left.entryType) + " and " +
                                    std::to_string(right.entryType) + " using op " +
                                    std::to_string(operation.binaryOperation));
            }

            resultType = static_cast<SemanticTable::Types>(result);
        }
        else if (stackSize >= 2) // Try unary: op, operand
        {
            SymbolTable::ExpressionsEntry operand = symbolTable.expressionStack.top();
            symbolTable.expressionStack.pop();

            SymbolTable::ExpressionsEntry operation = symbolTable.expressionStack.top();
            symbolTable.expressionStack.pop();

            int result = SemanticTable::unaryResultType(operand.entryType, operation.unaryOperation);

            if (result == SemanticTable::ERR)
            {
                throw SemanticError("Invalid unary operation on type " +
                                    std::to_string(operand.entryType) +
                                    " with operation " + std::to_string(operation.unaryOperation));
            }

            resultType = static_cast<SemanticTable::Types>(result);
        }
        else
        {
            throw SemanticError("Insufficient elements in expression stack for unary or binary expression.");
        }

        // Validate result type against expected
        int compat = SemanticTable::atribType(expectedType, resultType);

        if (compat == SemanticTable::ERR)
        {
            throw SemanticError(SemanticError::TypeMismatch(expectedType, resultType));
        }

        if (compat == SemanticTable::WAR)
        {
            this->reportWarning("Warning: possible data loss when assigning value of type " +
                                std::to_string(resultType) +
                                " to expected type " +
                                std::to_string(expectedType));
        }

        return resultType;
    }
    SemanticTable::Types reduceExpressionAndGetType(SemanticTable::Types expectedType = SemanticTable::Types::__NULL, bool validate = false)
    {
        if (symbolTable.expressionStack.empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        std::stack<SymbolTable::ExpressionsEntry> reverseStack;

        while (!symbolTable.expressionStack.empty())
        {
            reverseStack.push(symbolTable.expressionStack.top());
            symbolTable.expressionStack.pop();
        }

        SymbolTable::ExpressionsEntry result;

        while (reverseStack.size() >= 3)
        {
            SymbolTable::ExpressionsEntry right = reverseStack.top();
            reverseStack.pop();
            SymbolTable::ExpressionsEntry op = reverseStack.top();
            reverseStack.pop();
            SymbolTable::ExpressionsEntry left = reverseStack.top();
            reverseStack.pop();

            int typeResult = SemanticTable::resultBinaryType(left.entryType, right.entryType, op.binaryOperation);
            if (typeResult == SemanticTable::ERR)
            {
                throw SemanticError("Invalid binary operation between types " +
                                    std::to_string(left.entryType) + " and " +
                                    std::to_string(right.entryType) + " using op " +
                                    std::to_string(op.binaryOperation));
            }

            result.entryType = static_cast<SemanticTable::Types>(typeResult);
            reverseStack.push(result);
        }

        if (reverseStack.size() == 2)
        {
            SymbolTable::ExpressionsEntry operand = reverseStack.top();
            reverseStack.pop();
            SymbolTable::ExpressionsEntry op = reverseStack.top();
            reverseStack.pop();

            int typeResult = SemanticTable::unaryResultType(operand.entryType, op.unaryOperation);
            if (typeResult == SemanticTable::ERR)
            {
                throw SemanticError("Invalid unary operation on type " +
                                    std::to_string(operand.entryType) + " with op " +
                                    std::to_string(op.unaryOperation));
            }

            result.entryType = static_cast<SemanticTable::Types>(typeResult);
        }
        else if (reverseStack.size() == 1)
        {
            result = reverseStack.top();
            reverseStack.pop();
        }
        else
        {
            throw SemanticError("Malformed expression or unsupported expression format.");
        }

        if (validate)
        {
            int compat = SemanticTable::atribType(expectedType, result.entryType);
            if (compat == SemanticTable::ERR)
                throw SemanticError(SemanticError::TypeMismatch(expectedType, result.entryType));

            if (compat == SemanticTable::WAR)
                this->reportWarning("Warning: possible data loss converting " +
                                    std::to_string(result.entryType) +
                                    " to " + std::to_string(expectedType));
        }

        return result.entryType;
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

        // Make a copy of the expression stack
        std::stack<SymbolTable::ExpressionsEntry> copyStack = symbolTable.expressionStack;

        SymbolTable::ExpressionsEntry top = copyStack.top();

        for (SemanticTable::Types t : types)
        {
            if (SemanticTable::atribType(t, top.entryType) != SemanticTable::ERR)
            {
                return; // Valid type found, original stack untouched
            }
        }

        throw SemanticError("Expression type '" + std::to_string(top.entryType) +
                            "' not allowed here.");
    }

    void validateFunctionParamCount(SymbolTable::SymbolInfo *functionSymbol)
    {
        if (functionSymbol->functionParams != parametersCountInFuncCall)
        {
            throw SemanticError(SemanticError::WrongArgumentCount(
                functionSymbol->id,
                functionSymbol->functionParams,
                parametersCountInFuncCall));
        }
    }
};

#endif
