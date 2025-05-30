#include <vector>
#include <string>

#include "Semantic.h"
#include "Assembly.hpp"

using namespace std;

void Assembly::addData(const string id, const string value = "0")
{
    data.push_back("\t" + id + " : " + value);
}

void Assembly::addData(const string id, const int arrayLength = 0)
{
    string values = "";
    for (size_t i = 0; i < arrayLength; i++)
    {
        if (i != 0)
            values += ", ";
        values += "0";
    }
    data.push_back("\t" + id + " : " + values);
}

void Assembly::addText(const string instruction, const string operand)
{
    text.push_back("\t" + instruction + " " + operand);
}

void Assembly::addBlankLine()
{
    text.push_back("");
}

void Assembly::addComment(const string &comment)
{
    text.push_back("\t# " + comment);
}

string Assembly::generateAssembly()
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

string Assembly::generateAssemblyLabel(const string &id, int scope)
{
    return id + "_" + to_string(scope);
}

void Assembly::emitLoad(SymbolTable &symTable,
                        const ExpressionController::ExpressionsEntry &entry,
                        Semantic *semantic = nullptr)
{
    // ! Guard clause to allow only INT type for now
    if (entry.entryType != SemanticTable::Types::INT)
        return;

    if (isNumber(entry.value, true))
    {
        addText("LDI", entry.value);
    }
    else
    {
        auto *symbol = symTable.getSymbol(entry.value);
        string label = generateAssemblyLabel(symbol->id, symbol->scope);

        if (symbol->arraySize.size())
        {
            addText("STO", "$indr");
            addText("LDV", label);
            return;
        }
        addText("LD", label);
    }
}

void Assembly::emitUnaryOp(SymbolTable &symTable,
                           const ExpressionController::ExpressionsEntry &op,
                           const ExpressionController::ExpressionsEntry &operand,
                           bool shouldLoad,
                           Semantic *semantic = nullptr)
{
    // ! Guard clause to allow only INT type for now
    if (operand.entryType != SemanticTable::Types::INT)
        return;

    if (shouldLoad)
    {
        emitLoad(symTable, operand);
    }

    if (op.unaryOperation == SemanticTable::OperationsUnary::BITWISE_NOT)
    {
        if (isNumber(operand.value, true))
        {
            if (!shouldLoad)
                addText("ADDI", operand.value);
            addText("NOT", "");
        }
        else
        {

            auto *symbol = symTable.getSymbol(operand.value);
            string label = generateAssemblyLabel(symbol->id, symbol->scope);

            if (symbol->arraySize.size())
            {
                addText("STO", "$indr");
                addText("LDV", label);
            }
            else if (!shouldLoad)
            {
                addText("ADD", label);
            }

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

void Assembly::emitBinaryOp(SymbolTable &symTable,
                            const ExpressionController::ExpressionsEntry &op,
                            const ExpressionController::ExpressionsEntry &left,
                            const ExpressionController::ExpressionsEntry &right,
                            bool shouldLoad,
                            Semantic *semantic)
{
    // ! Guard clause to allow only INT type for now
    if (left.entryType != SemanticTable::Types::INT || right.entryType != SemanticTable::Types::INT)
        return;

    if (!left.value.empty())
    {
        // We always load the left operand, and the rights operand are added
        emitLoad(symTable, left);
    }

    if (!right.value.empty())
    {
        bool isRightNum = isNumber(right.value, true);
        string operand = right.value;

        if (!isRightNum)
        {
            auto *symbol = symTable.getSymbol(right.value);
            operand = generateAssemblyLabel(symbol->id, symbol->scope);

            if (symbol->arraySize.size())
            {
                addText("STO", this->tempAccAddress); // Store the left operand in a temporary location
                semantic->reduceExpressionAndGetType();
                addText("STO", "$indr");
                addText("LDV", operand);
                addText("STO", this->tempValueAddress);
                operand = this->tempValueAddress; // Use the temporary location for the right operand
                addText("LD", this->tempAccAddress);
            }
        }

        if (op.binaryOperation == SemanticTable::OperationsBinary::SUM)
            addText(isRightNum ? "ADDI" : "ADD", operand);
        else if (op.binaryOperation == SemanticTable::OperationsBinary::SUBTRACTION)
            addText(isRightNum ? "SUBI" : "SUB", operand);
        else if (op.binaryOperation == SemanticTable::OperationsBinary::BITWISE)
        {
            if (op.value == "<<")
            {
                // Check if the right operand is a number for SLL
                // The BIP v4, doesnt allow labels as right operand for SLL
                if (!isRightNum)
                {
                    throw SemanticError("Right operand for SLL must be a number, and not variable or label.");
                }
                addText("SLL", operand);
            }
            else if (op.value == ">>")
            {
                // Check if the right operand is a number for SRL
                // The BIP v4, doesnt allow labels as right operand for SRL
                if (!isRightNum)
                {
                    throw SemanticError("Right operand for SRL must be a number, and not variable or label.");
                }
                addText("SRL", operand);
            }
            else if (op.value == "&")
                addText(isRightNum ? "ANDI" : "AND", operand);
            else if (op.value == "|")
                addText(isRightNum ? "ORI" : "OR", operand);
            else if (op.value == "^")
                addText(isRightNum ? "XORI" : "XOR", operand);
        }
    }
}
