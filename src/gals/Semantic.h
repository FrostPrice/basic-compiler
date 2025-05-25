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
#include "Assembly.hpp"

using namespace std;

class Semantic
{
private:
    SymbolTable::SymbolInfo *currentSymbol = nullptr;     // Current symbol being processed
    SymbolTable::SymbolInfo *declarationSymbol = nullptr; // Declaration symbol being processed
    SymbolTable::SymbolInfo *functionSymbol = nullptr;    // Current function being processed

    ExpressionController expressionController; // Expression object to manage expressions

    stack<tuple<SymbolTable::SymbolInfo *, int, ExpressionController>> symbolEvaluateStack; // Stack to manage symbols in expressions

    SemanticTable::Types pendingType = SemanticTable::Types::__NULL; // Type of the last identifier

    vector<vector<int>> valueArraySizes; // Array dimensions of the declaration array value
    stack<int> arrayLengthsStack;        // Array length of inner arrays in declaration array value
    int arrayDepth = -1;                 // Array depth of the last identifier
    vector<string> arrayValues;          // Array values of the last identifier

    vector<int> functionArraySizes;    // Array dimensions of the function array value
    int parametersCountInFuncCall = 0; // Number of parameters in the function call

    int loopDepth = 0; // Loop depth for break and continue statements

    SemanticTable::Types switchResultType; // Type of the switch expression
    int switchDepth = 0;                   // Switch depth for break and continue statements

public:
    Assembly assembly; // Assembly object to generate assembly code

    vector<string> warnings; // Warnings generated during semantic analysis
    void reportWarning(const string &msg)
    {
        warnings.push_back(msg);
    }

    SymbolTable symbolTable;

    void executeAction(int action, const Token *token);

    // Helper Functions
    bool isNumber(const string &str, bool allowNegative = true)
    {
        if (str.empty())
            return false;

        size_t start = 0;
        if (allowNegative && str[0] == '-')
            start = 1;

        for (size_t i = start; i < str.size(); i++)
        {
            if (!isdigit(str[i]))
                return false;
        }
        return true;
    }

    int precedence(const ExpressionController::ExpressionsEntry &op)
    {
        using Kind = ExpressionController::ExpressionsEntry::Kind;

        if (op.kind == Kind::UNARY_OP)
        {
            // Highest precedence for unary operators (e.g., !, ~, ++, --, unary +, unary -)
            return 7;
        }

        if (op.kind == Kind::BINARY_OP)
        {
            // Multiplicative: *, /, %
            if (op.value == "*" || op.value == "/" || op.value == "%")
                return 6;
            // Additive: +, -
            if (op.value == "+" || op.value == "-")
                return 5;
            // Bitwise shift: <<, >>
            if (op.value == "<<" || op.value == ">>")
                return 4;
            // Relational: <, >, <=, >=
            if (op.value == "<" || op.value == ">" || op.value == "<=" || op.value == ">=")
                return 3;
            // Equality: ==, !=
            if (op.value == "==" || op.value == "!=")
                return 2;
            // Bitwise: AND &, XOR ^, OR |,
            if (op.value == "&" || op.value == "^" || op.value == "|")
                return 1;
            // Logical: AND &&, OR ||
            if (op.value == "&&" || op.value == "||")
                return 0;
            // Assignment (lowest precedence)
            if (op.value == "=" || op.value == "+=" || op.value == "-=" ||
                op.value == "*=" || op.value == "/=" || op.value == "%=")
                return -1;
        }

        // Default (unrecognized or miscategorized operator)
        return -1;
    }

    // Validation methods
    SemanticTable::Types reduceExpressionAndGetType(
        SemanticTable::Types expectedType = SemanticTable::Types::__NULL,
        bool validate = false,
        bool willBeParameter = false)
    {
        using Entry = ExpressionController::ExpressionsEntry;

        auto *orig = symbolEvaluateStack.empty()
                         ? &expressionController.expressionStack
                         : &get<2>(symbolEvaluateStack.top()).expressionStack;

        if (orig->empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        vector<Entry> tokens;
        tokens.reserve(orig->size());
        while (!orig->empty())
        {
            tokens.push_back(move(orig->top()));
            orig->pop();
        }
        reverse(tokens.begin(), tokens.end());

        // Convert to RPN using precedence
        vector<Entry> rpn;
        stack<Entry> opStack;
        for (auto &t : tokens)
        {
            if (t.kind == Entry::VALUE)
            {
                rpn.push_back(t);
            }
            else
            {
                while (!opStack.empty() &&
                       opStack.top().kind != Entry::VALUE &&
                       precedence(opStack.top()) >= precedence(t))
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

        stack<Entry> eval;
        for (auto &tok : rpn)
        {
            if (tok.kind == Entry::VALUE)
            {
                eval.push(tok);
                if (rpn.size() == 1)
                    assembly.emitLoad(this->symbolTable, tok, willBeParameter);
            }
            else if (tok.kind == Entry::UNARY_OP)
            {
                if (eval.empty())
                    throw SemanticError("Not enough operands for unary operator");

                auto opnd = eval.top();
                eval.pop();
                auto resultType = SemanticTable::unaryResultType(opnd.entryType, tok.unaryOperation);
                if (resultType == SemanticTable::ERR)
                    throw SemanticError("Invalid unary op on type " + SemanticError::typeToString(opnd.entryType));

                ExpressionController::ExpressionsEntry out;
                out.kind = ExpressionController::ExpressionsEntry::VALUE;
                out.entryType = static_cast<SemanticTable::Types>(resultType);
                eval.push(out);

                assembly.emitUnaryOp(this->symbolTable, tok, opnd);
            }
            else
            { // Binary op
                if (eval.size() < 2)
                    throw SemanticError("Not enough operands for binary operator");

                auto r = eval.top();
                eval.pop();
                auto l = eval.top();
                eval.pop();

                auto resultType = SemanticTable::resultBinaryType(l.entryType, r.entryType, tok.binaryOperation);
                if (resultType == SemanticTable::ERR)
                    throw SemanticError("Invalid binary op between types " +
                                        SemanticError::typeToString(l.entryType) + " and " +
                                        SemanticError::typeToString(r.entryType));

                ExpressionController::ExpressionsEntry out;
                out.kind = ExpressionController::ExpressionsEntry::VALUE;
                out.entryType = static_cast<SemanticTable::Types>(resultType);
                eval.push(out);

                assembly.emitBinaryOp(symbolTable, tok, l, r, willBeParameter);
            }
        }

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
                reportWarning("Warning: possible data loss converting " +
                              SemanticError::typeToString(result.entryType) + " to " +
                              SemanticError::typeToString(expectedType));
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
        stack<ExpressionController::ExpressionsEntry> expressionStack = symbolEvaluateStack.empty()
                                                                            ? expressionController.expressionStack
                                                                            : get<2>(symbolEvaluateStack.top()).expressionStack;

        if (expressionStack.empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        ExpressionController::ExpressionsEntry top = expressionStack.top();

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
