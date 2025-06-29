#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <optional>
#include <algorithm>

#include "ExpressionController.hpp"
#include "Token.h"
#include "SemanticError.h"
#include "SemanticTable.hpp"
#include "SymbolTable.hpp"
#include "Assembly.hpp"

using namespace std;

using Entry = ExpressionController::ExpressionsEntry;

class Semantic
{
private:
    struct ExpressionScope
    {
        SymbolTable::SymbolInfo *symbol;           // For complex symbols like arrays or functions
        int arrayDepth;                            // Depth of the array access
        ExpressionController expressionController; // Controller for managing expressions

        ExpressionScope(SymbolTable::SymbolInfo *symbol = nullptr, int arrayDepth = 0)
            : symbol(symbol), arrayDepth(arrayDepth) {}
    };

    SymbolTable::SymbolInfo *currentSymbol = nullptr;     // Current symbol being processed
    SymbolTable::SymbolInfo *declarationSymbol = nullptr; // Declaration symbol being processed
    SymbolTable::SymbolInfo *functionSymbol = nullptr;    // Current function being processed

    vector<ExpressionScope> expressionScopeList = vector<ExpressionScope>({ExpressionScope()}); // Stack to manage symbols in expressions
    stack<int> expressionScopeIndexes = stack<int>({0});                                        // Index of the current expression scope
    int lastExpressionScopeIndex = 0;                                                           // Index of the last expression scope

    SemanticTable::Types pendingType = SemanticTable::Types::__NULL; // Type of the last identifier

    vector<vector<int>> valueArraySizes; // Array dimensions of the declaration array value
    stack<int> arrayLengthsStack;        // Array length of inner arrays in declaration array value
    int arrayDepth = -1;                 // Array depth of the last identifier
    bool hasToCleanAccumulator = true;   // Flag to indicate if the accumulator needs to be cleaned

    vector<int> functionArraySizes; // Array dimensions of the function array value

    int loopDepth = 0; // Loop depth for break and continue statements

    SemanticTable::Types switchResultType; // Type of the switch expression
    int switchDepth = 0;

    bool hasToCreateScope = true; // Flag to indicate if a new scope should be created
    bool returnInstructionAdded = false;

public:
    vector<SymbolTable::SymbolInfo *> symbolPointers; // Pointers to symbols for semantic analysis
    Assembly assembly;                                // Assembly object to generate assembly code
    enum JumpType
    {
        NONE = -1, // No specific type
        IF,        // Label for if statements
        WHILE,     // Label for while loops
        FOR,       // Label for for loops
        DO_WHILE,  // Label for do-while loops
        SWITCH,    // Label for switch statements
        FUNCTION,  // Label for function definitions
    };
    struct Label
    {
        string name;
        JumpType jumpType = NONE;
    };
    stack<Label> labelStack;            // Counter for if statements
    bool invertLogicalOperation = true; // Flag to invert logical operations (FOR DO WHILE)
    int mainInnerScope = -1;            // Inner scope of the main function, used to check if we are inside the main function, for the warning

    vector<string> warnings; // Warnings generated during semantic analysis
    void reportWarning(const string &msg)
    {
        warnings.push_back(msg);
    }

    SymbolTable symbolTable;

    void executeAction(int action, const Token *token);

    // Helper Functions
    string jumpTypeToString(JumpType type)
    {
        switch (type)
        {
        case IF:
            return "IF";
        case WHILE:
            return "WHILE";
        case FOR:
            return "FOR";
        case DO_WHILE:
            return "DO_WHILE";
        case SWITCH:
            return "SWITCH";
        default:
            return "UNKNOWN";
        }
    }

    ExpressionController::ExpressionsEntry getNextExpressionEntry()
    {
        if (expressionScopeList.empty() || expressionScopeList.front().expressionController.expressionStack.empty())
            throw SemanticError(SemanticError::ExpressionStackEmpty());

        ExpressionController::ExpressionsEntry entry = expressionScopeList.front().expressionController.expressionStack.top();
        return entry;
    };

    void clearSymbolPointer()
    {
        for (SymbolTable::SymbolInfo *symbol : symbolPointers)
        {
            delete symbol;
        }

        symbolPointers.clear();
    }

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
        bool shouldLoad = true)
    {
        this->lastExpressionScopeIndex = 0;

        stack<Entry> *orig = &expressionScopeList.front().expressionController.expressionStack;

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

        if (expressionScopeList.size() > 1)
            expressionScopeList.erase(expressionScopeList.begin());

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
        for (auto tok : rpn)
        {
            if (tok.kind == Entry::VALUE || tok.kind == Entry::UNARY_OP)
            {
                if (rpn.size() == 1)
                    assembly.emitLoad(this->symbolTable, tok, this, shouldLoad);
                eval.push(tok);
            }
            else
            { // Binary op
                if (eval.size() < 2)
                    throw SemanticError("Not enough operands for binary operator");

                auto r = eval.top();
                eval.pop();
                auto l = eval.top();
                eval.pop();

                assembly.emitBinaryOp(symbolTable, tok, l, r, this);

                auto resultType = SemanticTable::resultBinaryType(l.entryType, r.entryType, tok.binaryOperation);
                if (resultType == SemanticTable::ERR)
                    throw SemanticError("Invalid binary op between types " +
                                        SemanticError::typeToString(l.entryType) + " and " +
                                        SemanticError::typeToString(r.entryType));

                ExpressionController::ExpressionsEntry out;
                out.kind = ExpressionController::ExpressionsEntry::VALUE;
                out.entryType = static_cast<SemanticTable::Types>(resultType);
                eval.push(out);
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

    void reduceParameters(SymbolTable::SymbolInfo *functionSymbol)
    {
        stack<Entry> *orig = &expressionScopeList.front().expressionController.expressionStack;

        vector<Entry> tokens;
        tokens.reserve(orig->size());
        while (!orig->empty())
        {
            tokens.push_back(move(orig->top()));
            orig->pop();
        }
        reverse(tokens.begin(), tokens.end());

        vector<SymbolTable::SymbolInfo *> params = functionSymbol->functionParams;

        if (expressionScopeList.size() > 1)
            expressionScopeList.erase(expressionScopeList.begin());

        this->assembly.addComment("Function parameters for " + functionSymbol->id);

        if (tokens.size() != params.size())
            throw SemanticError(SemanticError::WrongArgumentCount(functionSymbol->id, params.size(), tokens.size()));

        for (int i = 0; i < tokens.size(); i++)
        {
            reduceExpressionAndGetType(params[i]->dataType, true, true);
            this->assembly.addText("STO", this->assembly.generateAssemblyLabel("FUNC_" + params[i]->id, params[i]->scope));
        }

        this->assembly.addBlankLine();
    }

    void createExpressionScope(SymbolTable::SymbolInfo *symbol = nullptr)
    {
        this->expressionScopeList.push_back(ExpressionScope(symbol));
        this->expressionScopeIndexes.push(++this->lastExpressionScopeIndex);
    }

    void closeUnaryScopeIfNeeded()
    {
        if (this->expressionScopeIndexes.size() > 1)
        {
            int lastScopeIndex = this->expressionScopeIndexes.top();
            this->expressionScopeIndexes.pop();
            ExpressionScope lastExpressionScope = this->expressionScopeList[this->expressionScopeIndexes.top()];
            if (lastExpressionScope.expressionController.expressionStack.top().kind != ExpressionController::ExpressionsEntry::UNARY_OP)
                this->expressionScopeIndexes.push(lastScopeIndex);
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
        if (currentSymbol->symbolClassification != SymbolTable::VARIABLE && currentSymbol->symbolClassification != SymbolTable::ARRAY)
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
        stack<ExpressionController::ExpressionsEntry> expressionStack = expressionScopeList.front().expressionController.expressionStack;

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
};

#endif
