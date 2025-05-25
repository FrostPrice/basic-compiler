#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <vector>
#include <string>

using namespace std;

class Assembly
{
private:
    vector<string> data;
    vector<string> text;

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

public:
    void addData(const string id, const string value = "0")
    {
        data.push_back("\t" + id + " : " + value);
    }

    void addData(const string id, const int arrayLength = 0)
    {
        string values = "";
        for (size_t i = 0; i < arrayLength; i++)
        {
            if (i != 0)
                values += ", ";
            values += "-1";
        }
        data.push_back("\t" + id + " : " + values);
    }

    void addText(const string instruction, const string operand)
    {
        text.push_back("\t" + instruction + " " + operand);
    }

    void addBlankLine()
    {
        text.push_back("");
    }

    void addComment(const string &comment)
    {
        text.push_back("\t# " + comment);
    }

    string generateAssembly()
    {
        string assemblyCode;
        assemblyCode += ".data\n";
        for (const auto &d : data)
        {
            assemblyCode += d + "\n";
        }
        assemblyCode += "\n.text\n";
        for (const auto &t : text)
        {
            assemblyCode += t + "\n";
        }
        return assemblyCode;
    }

    string generateAssemblyLabel(const string &id, int scope)
    {
        return id + "_" + to_string(scope);
    }

    void emitLoad(SymbolTable &symTable,
                  const ExpressionController::ExpressionsEntry &entry,
                  bool willBeParameter)
    {
        // ! Guard clause to allow only INT type for now
        if (entry.entryType != SemanticTable::Types::INT)
            return;

        if (isNumber(entry.value, true))
        {
            addText(willBeParameter ? "LDI" : "ADDI", entry.value);
        }
        else
        {
            auto *symbol = symTable.getSymbol(entry.value);
            string label = generateAssemblyLabel(symbol->id, symbol->scope);
            addText(willBeParameter ? "LD" : "ADD", label);
        }
    }

    void emitUnaryOp(SymbolTable &symTable,
                     const ExpressionController::ExpressionsEntry &op,
                     const ExpressionController::ExpressionsEntry &operand)
    {
        // ! Guard clause to allow only INT type for now
        if (operand.entryType != SemanticTable::Types::INT)
            return;

        if (op.unaryOperation == SemanticTable::OperationsUnary::BITWISE_NOT)
        {
            if (isNumber(operand.value, true))
            {
                addText("ADDI", operand.value);
                addText("NOT", "");
            }
            else
            {
                auto *symbol = symTable.getSymbol(operand.value);
                string label = generateAssemblyLabel(symbol->id, symbol->scope);
                addText("ADD", label);
                addText("NOT", "");
            }
        }
        else if (op.unaryOperation == SemanticTable::OperationsUnary::NEG)
        {
            if (isNumber(operand.value, true))
            {
                addText("SUBI", operand.value);
            }
            else
            {
                auto *symbol = symTable.getSymbol(operand.value);
                string label = generateAssemblyLabel(symbol->id, symbol->scope);
                addText("SUB", label);
            }
        }
    }

    void emitBinaryOp(SymbolTable &symTable,
                      const ExpressionController::ExpressionsEntry &op,
                      const ExpressionController::ExpressionsEntry &left,
                      const ExpressionController::ExpressionsEntry &right,
                      bool willBeParameter)
    {
        // ! Guard clause to allow only INT type for now
        if (left.entryType != SemanticTable::Types::INT || right.entryType != SemanticTable::Types::INT)
            return;

        if (!left.value.empty())
        {
            emitLoad(symTable, left, willBeParameter);
        }

        if (!right.value.empty())
        {
            bool isRightNum = isNumber(right.value, true);
            string operand = right.value;

            if (!isRightNum)
            {
                auto *symbol = symTable.getSymbol(right.value);
                operand = generateAssemblyLabel(symbol->id, symbol->scope);
            }

            if (op.binaryOperation == SemanticTable::OperationsBinary::SUM)
                addText(isRightNum ? "ADDI" : "ADD", operand);
            else if (op.binaryOperation == SemanticTable::OperationsBinary::SUBTRACTION)
                addText(isRightNum ? "SUBI" : "SUB", operand);
            else if (op.binaryOperation == SemanticTable::OperationsBinary::BITWISE)
            {
                if (op.value == "<<")
                    addText("SLL", operand);
                else if (op.value == ">>")
                    addText("SRL", operand);
                else if (op.value == "&")
                    addText(isRightNum ? "ANDI" : "AND", operand);
                else if (op.value == "|")
                    addText(isRightNum ? "ORI" : "OR", operand);
                else if (op.value == "^")
                    addText(isRightNum ? "XORI" : "XOR", operand);
            }
        }
    }
};

#endif // ASSEMBLY_H