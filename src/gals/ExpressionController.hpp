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
    };

    stack<ExpressionsEntry> expressionStack; // Stack to manage expressions

    void pushType(SemanticTable::Types type, const string &value)
    {
        ExpressionsEntry entry;
        entry.kind = ExpressionsEntry::VALUE;
        entry.entryType = type;
        entry.value = value;
        expressionStack.push(entry);
    }

    // Pushes a pending binary operator onto the expression stack
    void pushBinaryOp(SemanticTable::OperationsBinary op, string value)
    {
        ExpressionsEntry entry;
        entry.kind = ExpressionsEntry::BINARY_OP;
        entry.binaryOperation = op;
        entry.value = value;
        expressionStack.push(entry);
    }

    // Pushes a pending unary operator
    void pushUnaryOp(SemanticTable::OperationsUnary op)
    {
        ExpressionsEntry entry;
        entry.kind = ExpressionsEntry::UNARY_OP;
        entry.unaryOperation = op;
        expressionStack.push(entry);
    }
};

#endif // EXPRESSION_CONTROLLER_HPP