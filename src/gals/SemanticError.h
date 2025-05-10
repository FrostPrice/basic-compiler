#ifndef SEMANTIC_ERROR_H
#define SEMANTIC_ERROR_H

#include "AnalysisError.h"
#include "SemanticTable.hpp"

#include <string>

using namespace std;

class SemanticError : public AnalysisError
{
public:
    SemanticError(const string &msg, int position = -1)
        : AnalysisError(msg, position) {}

    // Static helpers for common errors
    static string TypeMismatch(SemanticTable::Types expected, SemanticTable::Types actual)
    {
        return "Type mismatch: expected '" + typeToString(expected) + "' but got '" + typeToString(actual) + "'";
    }

    static string SymbolNotOfClassification(const string &id)
    {
        return "Symbol '" + id + "' is not of the expected classification";
    }

    static string DuplicateSymbol(const string &id)
    {
        return "Symbol '" + id + "' already exists in this scope";
    }

    static string TypeAssignmentMismatch(const string &id, SemanticTable::Types expected)
    {
        return "Type mismatch: cannot assign to variable '" + id + "' of type '" + to_string(expected) + "'";
    }

    static string ReturnOutsideFunction()
    {
        return "Return statement outside of a function";
    }

    static string SymbolUndeclared(const string &id)
    {
        return "Symbol '" + id + "' is not declared";
    }

    static string SymbolNotInitialized(const string &id)
    {
        return "Symbol '" + id + "' was not initialized";
    }

    static string InputNonVariable(const string &id)
    {
        return "Cannot input into non-variable '" + id + "'";
    }

    static string ExpressionStackEmpty()
    {
        return "Expression stack is empty";
    }

    static string NonNumericOperator()
    {
        return "Arithmetic operator requires numeric operands";
    }

    static string oneOperatorForBinaryExpression()
    {
        return "Binary expression requires two operands";
    }

    static string WrongArgumentCount(const string &id, int expected, int actual)
    {
        return "Function '" + id + "' expects " + to_string(expected) + " argument(s), but got " + to_string(actual);
    }

    static string InvalidValueForArrayLength()
    {
        return "Invalid value for array length";
    }

    static string FunctionNotCalled(const string &id)
    {
        return "Function '" + id + "' was not called";
    }

    static string typeToString(SemanticTable::Types type)
    {
        if (type == SemanticTable::Types::__NULL)
            return "__NULL";
        else if (type == SemanticTable::Types::INT)
            return "INT";
        else if (type == SemanticTable::Types::FLOAT)
            return "FLOAT";
        else if (type == SemanticTable::Types::DOUBLE)
            return "DOUBLE";
        else if (type == SemanticTable::Types::CHAR)
            return "CHAR";
        else if (type == SemanticTable::Types::STRING)
            return "STRING";
        else if (type == SemanticTable::Types::BOOL)
            return "BOOL";
    }
};

#endif
