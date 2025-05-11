#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <vector>
#include <stack>
#include <optional>
#include <algorithm>

#include "ExpressionController.hpp"
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

    ExpressionController expressionController; // Expression object to manage expressions

    stack<tuple<SymbolTable::SymbolInfo *, int, ExpressionController>> symbolEvaluateStack; // Stack to manage symbols in expressions

    SemanticTable::Types pendingType = SemanticTable::Types::__NULL; // Type of the last identifier

    vector<vector<int>> valueArraySizes;   // Array dimensions of the declaration array value
    stack<int> arrayLengthsStack;          // Array length of inner arrays in declaration array value
    int arrayDepth = -1;                   // Array depth of the last identifier
    int parametersCountInFuncCall = 0;     // Number of parameters in the function call
    SemanticTable::Types switchResultType; // Type of the switch expression
    int loopDepth = 0;                     // Loop depth for break and continue statements
    int switchDepth = 0;                   // Switch depth for break and continue statements

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

        while (!this->expressionController.expressionStack.empty())
            this->expressionController.expressionStack.pop();
    }

    void executeAction(int action, const Token *token);

    // Validation methods
    SemanticTable::Types reduceExpressionAndGetType(SemanticTable::Types expectedType = SemanticTable::Types::__NULL, bool validate = false)
    {
        auto *orig = symbolEvaluateStack.empty()
                         ? &expressionController.expressionStack
                         : &get<2>(symbolEvaluateStack.top()).expressionStack;

        if (orig->empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        vector<ExpressionController::ExpressionsEntry> tokens;
        tokens.reserve(orig->size());
        while (!orig->empty())
        {
            tokens.push_back(move(orig->top()));
            orig->pop();
        }
        reverse(tokens.begin(), tokens.end());

        vector<ExpressionController::ExpressionsEntry> rpn;
        stack<ExpressionController::ExpressionsEntry> opStack;
        for (auto &t : tokens)
        {
            if (t.kind == ExpressionController::ExpressionsEntry::VALUE)
            {
                rpn.push_back(t);
            }
            else
            {
                while (!opStack.empty() &&
                       opStack.top().kind != ExpressionController::ExpressionsEntry::VALUE)
                {
                    rpn.push_back(opStack.top());
                    opStack.pop();
                }
                opStack.push(t);
            }
        }
        while (!opStack.empty())
        {
            rpn.push_back(opStack.top());
            opStack.pop();
        }

        stack<ExpressionController::ExpressionsEntry> eval;
        for (auto &tok : rpn)
        {
            if (tok.kind == ExpressionController::ExpressionsEntry::VALUE)
            {
                eval.push(tok);
            }
            else if (tok.kind == ExpressionController::ExpressionsEntry::UNARY_OP)
            {
                if (eval.empty())
                    throw SemanticError("Not enough operands for unary operator");
                auto opnd = eval.top();
                eval.pop();
                int rt = SemanticTable::unaryResultType(opnd.entryType, tok.unaryOperation);
                if (rt == SemanticTable::ERR)
                    throw SemanticError("Invalid unary op on type " + to_string(opnd.entryType));
                ExpressionController::ExpressionsEntry out;
                out.kind = ExpressionController::ExpressionsEntry::VALUE;
                out.entryType = static_cast<SemanticTable::Types>(rt);
                eval.push(out);
            }
            else
            { // BINARY_OP
                if (eval.size() < 2)
                    throw SemanticError("Not enough operands for binary operator");
                auto r = eval.top();
                eval.pop();
                auto l = eval.top();
                eval.pop();
                int rt = SemanticTable::resultBinaryType(l.entryType, r.entryType, tok.binaryOperation);
                if (rt == SemanticTable::ERR)
                    throw SemanticError("Invalid binary op between types " + to_string(l.entryType) + " and " + to_string(r.entryType));
                ExpressionController::ExpressionsEntry out;
                out.kind = ExpressionController::ExpressionsEntry::VALUE;
                out.entryType = static_cast<SemanticTable::Types>(rt);
                eval.push(out);
            }
        }

        // Must end with exactly one
        if (eval.size() != 1)
            throw SemanticError("Malformed expression or unsupported expression format");
        auto result = eval.top();

        // Optional validation
        if (validate)
        {
            int comp = SemanticTable::atribType(expectedType, result.entryType);
            if (comp == SemanticTable::ERR)
                throw SemanticError(SemanticError::TypeMismatch(expectedType, result.entryType));
            if (comp == SemanticTable::WAR)
                reportWarning("Warning: possible data loss converting " + to_string(result.entryType) + " to " + to_string(expectedType));
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
        if (expressionController.expressionStack.empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        // Make a copy of the expression stack
        std::stack<ExpressionController::ExpressionsEntry> copyStack = expressionController.expressionStack;

        ExpressionController::ExpressionsEntry top = copyStack.top();

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
