#ifndef EXPRESSION_CONTROLLER_HPP
#define EXPRESSION_CONTROLLER_HPP

#include <stack>
#include <variant>

#include "SymbolTable.hpp"

using namespace std;

using Operations = variant<SemanticTable::OperationsBinary, SemanticTable::OperationsUnary>;

class ExpressionController
{
public:
    struct ExpressionsEntry
    {
        enum Kind
        {
            BINARY_OP,
            UNARY_OP,
            VALUE
        } kind; // Type of the entry (binary operator, unary operator, or value)

        SemanticTable::Types entryType = SemanticTable::Types::__NULL; // Type of the entry
        SemanticTable::OperationsBinary binaryOperation;               // Binary operation
        SemanticTable::OperationsUnary unaryOperation;                 // Unary operation
        string value;
        bool hasOwnScope; // Indicates if the entry has its own scope (arrays and functions)
    };

    stack<ExpressionsEntry> expressionStack; // Stack to manage expressions

    void pushType(SemanticTable::Types type, const string &value, bool hasOwnScope = false)
    {
        ExpressionsEntry entry;
        entry.kind = ExpressionsEntry::VALUE;
        entry.entryType = type;
        entry.value = value;
        entry.hasOwnScope = hasOwnScope; // Set if the entry has its own scope
        expressionStack.push(entry);
    }

    // Pushes a pending binary operator onto the expression stack
    void pushBinaryOp(SemanticTable::OperationsBinary op, string value)
    {
        ExpressionsEntry entry;
        entry.kind = ExpressionsEntry::BINARY_OP;
        entry.binaryOperation = op;
        entry.value = value;
        entry.hasOwnScope = false;
        expressionStack.push(entry);
    }

    // Pushes a pending unary operator
    void pushUnaryOp(SemanticTable::OperationsUnary op, string value)
    {
        ExpressionsEntry entry;
        entry.kind = ExpressionsEntry::UNARY_OP;
        entry.unaryOperation = op;
        entry.value = value;
        entry.hasOwnScope = false;
        expressionStack.push(entry);
    }
};

#endif // EXPRESSION_CONTROLLER_HPP