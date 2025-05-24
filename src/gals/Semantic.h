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

    // Assembly Related
    string generateAssemblyLabel(const string &id, int scope)
    {
        return id + "_" + to_string(scope);
    }

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

                // * Assembly generation
                if (rpn.size() == 1)
                {
                    if (isNumber(tok.value, true)) // If the lexeme is a number, use imidiate instructions
                    {
                        this->assembly.addText("ADDI", tok.value);
                    }
                    else // If the lexeme is not a number, use the label of the variable
                    {
                        SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(tok.value);
                        string generatedLabel = generateAssemblyLabel(symbol->id, symbol->scope);

                        this->assembly.addText("ADD", generatedLabel);
                    }
                }
            }
            else if (tok.kind == ExpressionController::ExpressionsEntry::UNARY_OP)
            {
                if (eval.empty())
                    throw SemanticError("Not enough operands for unary operator");
                auto opnd = eval.top();
                eval.pop();
                int rt = SemanticTable::unaryResultType(opnd.entryType, tok.unaryOperation);
                if (rt == SemanticTable::ERR)
                    throw SemanticError("Invalid unary op on type " + SemanticError::typeToString(opnd.entryType));
                ExpressionController::ExpressionsEntry out;
                out.kind = ExpressionController::ExpressionsEntry::VALUE;
                out.entryType = static_cast<SemanticTable::Types>(rt);
                eval.push(out);

                // * Assembly generation
                if (isNumber(opnd.value, true)) // If the lexeme is a number, use imidiate instructions
                {
                    if (tok.unaryOperation == SemanticTable::OperationsUnary::BITWISE_NOT)
                    {
                        this->assembly.addText("ADDI", opnd.value);
                        this->assembly.addText("NOT", ""); // * The NOT instruction does not require an operand, since in BIP it will negate the value in the register (ACC)
                    }
                }
                else // If the lexeme is not a number, use the label of the variable
                {
                    SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(opnd.value);
                    string generatedLabel = generateAssemblyLabel(symbol->id, symbol->scope);

                    if (tok.unaryOperation == SemanticTable::OperationsUnary::BITWISE_NOT)
                    {
                        this->assembly.addText("ADDI", generatedLabel);
                        this->assembly.addText("NOT", ""); // * The NOT instruction does not require an operand, since in BIP it will negate the value in the register (ACC)
                    }
                }
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
                    throw SemanticError("Invalid binary op between types " + SemanticError::typeToString(l.entryType) + " and " + SemanticError::typeToString(r.entryType));
                ExpressionController::ExpressionsEntry out;
                out.kind = ExpressionController::ExpressionsEntry::VALUE;
                out.entryType = static_cast<SemanticTable::Types>(rt);
                eval.push(out);

                // * Assembly generation
                // ? Left Operand
                // We always add the first operand
                // ! Only the first ocurency of the left operand is used, in the rest of the expression, will be only the right operand
                // TODO: There may be a better way to validate the left operand without this shit
                if (l.value != "")
                {
                    if (isNumber(l.value, true)) // If the lexeme is a number, use imidiate instructions
                    {
                        this->assembly.addText("ADDI", l.value);
                    }
                    else // If the lexeme is not a number, use the label of the variable
                    {
                        SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(l.value);
                        string generatedLabel = generateAssemblyLabel(symbol->id, symbol->scope);

                        this->assembly.addText("ADD", generatedLabel);
                    }
                }

                // ? Right Operand
                if (isNumber(r.value, true)) // If the lexeme is a number, use imidiate instructions
                {
                    if (tok.binaryOperation == SemanticTable::OperationsBinary::SUM)
                    {
                        this->assembly.addText("ADDI", r.value);
                    }
                    else if (tok.binaryOperation == SemanticTable::OperationsBinary::SUBTRACTION)
                    {
                        this->assembly.addText("SUBI", r.value);
                    }
                    else if (tok.binaryOperation == SemanticTable::OperationsBinary::BITWISE)
                    {
                        // TODO: There must be a better way for this
                        if (tok.value == "<<")
                        {
                            this->assembly.addText("SLL", r.value);
                        }
                        else if (tok.value == ">>")
                        {
                            this->assembly.addText("SRL", r.value);
                        }
                        else if (tok.value == "&")
                        {
                            this->assembly.addText("ANDI", r.value);
                        }
                        else if (tok.value == "|")
                        {
                            this->assembly.addText("ORI", r.value);
                        }
                        else if (tok.value == "^")
                        {
                            this->assembly.addText("XORI", r.value);
                        }
                    }
                }
                else // If the lexeme is not a number, use the label of the variable
                {
                    SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(r.value);
                    string generatedLabel = generateAssemblyLabel(symbol->id, symbol->scope);

                    if (tok.binaryOperation == SemanticTable::OperationsBinary::SUM)
                    {
                        this->assembly.addText("ADD", generatedLabel);
                    }
                    else if (tok.binaryOperation == SemanticTable::OperationsBinary::SUBTRACTION)
                    {
                        this->assembly.addText("SUB", generatedLabel);
                    }
                    else if (tok.binaryOperation == SemanticTable::OperationsBinary::BITWISE)
                    {
                        // TODO: There must be a better way for this
                        if (tok.value == "<<")
                        {
                            this->assembly.addText("SLL", generatedLabel);
                        }
                        else if (tok.value == ">>")
                        {
                            this->assembly.addText("SRL", generatedLabel);
                        }
                        else if (tok.value == "&")
                        {
                            this->assembly.addText("AND", generatedLabel);
                        }
                        else if (tok.value == "|")
                        {
                            this->assembly.addText("OR", generatedLabel);
                        }
                        else if (tok.value == "^")
                        {
                            this->assembly.addText("XOR", generatedLabel);
                        }
                    }
                }
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
                reportWarning("Warning: possible data loss converting " + SemanticError::typeToString(result.entryType) + " to " + SemanticError::typeToString(expectedType));
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
