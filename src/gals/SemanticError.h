#ifndef SEMANTIC_ERROR_H
#define SEMANTIC_ERROR_H

#include "AnalysisError.h"
#include "SemanticTable.hpp"

#include <string>

class SemanticError : public AnalysisError
{
public:
  SemanticError(const std::string &msg, int position = -1)
      : AnalysisError(msg, position) {}

  // Static helpers for common errors
  static std::string TypeMismatch(SemanticTable::Types expected, SemanticTable::Types actual)
  {
    return "Type mismatch: expected '" + std::to_string(expected) + "' but got '" + std::to_string(actual) + "'";
  }

  static std::string SymbolNotOfClassification(const std::string &id)
  {
    return "Symbol '" + id + "' is not of the expected classification";
  }

  static std::string DuplicateSymbol(const std::string &id)
  {
    return "Symbol '" + id + "' already exists in this scope";
  }

  static std::string TypeAssignmentMismatch(const std::string &id, SemanticTable::Types expected)
  {
    return "Type mismatch: cannot assign to variable '" + id + "' of type '" + std::to_string(expected) + "'";
  }

  static std::string ReturnOutsideFunction()
  {
    return "Return statement outside of a function";
  }

  static std::string SymbolUndeclared(const std::string &id)
  {
    return "Variable '" + id + "' is not declared";
  }

  static std::string InputNonVariable(const std::string &id)
  {
    return "Cannot input into non-variable '" + id + "'";
  }

  static std::string ExpressionStackEmpty()
  {
    return "Expression stack is empty";
  }

  static std::string NonNumericOperator()
  {
    throw "Arithmetic operator requires numeric operands";
  }

  static std::string oneOperatorForBinaryExpression()
  {
    return "Binary expression requires two operands";
  }
};

#endif
