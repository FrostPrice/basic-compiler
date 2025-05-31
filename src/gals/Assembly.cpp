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
                        ExpressionController::ExpressionsEntry &entry,
                        Semantic *semantic = nullptr)
{
    if (isNumber(entry.value, false))
    {
        addText("LDI", entry.value);
    }
    else if (entry.hasOwnScope)
    {
        if (entry.kind == ExpressionController::ExpressionsEntry::UNARY_OP)
        {
            emitUnaryOp(entry, semantic);
        }
        else
        {
            auto *symbol = symTable.getSymbol(entry.value);
            string label = generateAssemblyLabel(symbol->id, symbol->scope);
            if (symbol->arraySize.size())
            {
                semantic->reduceExpressionAndGetType();
                addText("STO", "$indr");
                addText("LDV", label);
            }
            else
            {
                addText("LD", label);
            }
        }
    }
}

void Assembly::emitUnaryOp(ExpressionController::ExpressionsEntry &op, Semantic *semantic)
{
    op.entryType = semantic->reduceExpressionAndGetType();

    if (op.unaryOperation == SemanticTable::OperationsUnary::BITWISE_NOT || op.unaryOperation == SemanticTable::OperationsUnary::NOT)
    {
        addText("NOT", "");
    }
    else if (op.unaryOperation == SemanticTable::OperationsUnary::NEG)
    {
        addText("NOT", "");
        addText("ADDI", "1");
    }
}

void Assembly::emitBinaryOp(SymbolTable &symTable,
                            const ExpressionController::ExpressionsEntry &op,
                            ExpressionController::ExpressionsEntry &left,
                            ExpressionController::ExpressionsEntry &right,
                            bool shouldLoad,
                            Semantic *semantic)
{
    if (left.kind == ExpressionController::ExpressionsEntry::UNARY_OP)
    {
        // If the left operand is a unary operation, we need to emit it first
        emitUnaryOp(left, semantic);
    }
    else if (!left.value.empty())
    {
        // We always load the left operand, and the rights operand are added
        emitLoad(symTable, left, semantic);
    }

    if (!right.value.empty())
    {
        bool isRightNum = isNumber(right.value, false);
        string operand = right.value;

        if (!isRightNum)
        {
            SymbolTable::SymbolInfo *symbol = symTable.getSymbol(right.value);

            if (right.hasOwnScope)
            {
                addText("STO", to_string(this->tempAccAddressStack.top()));
                this->tempAccAddressStack.push(this->tempAccAddressStack.top() + 1);

                if (right.kind == ExpressionController::ExpressionsEntry::UNARY_OP)
                {
                    emitUnaryOp(right, semantic);
                }
                else if (symbol && symbol->arraySize.size())
                {
                    semantic->reduceExpressionAndGetType();
                    addText("STO", "$indr");
                    addText("LDV", operand);
                }

                addText("STO", this->tempValueAddress);
                operand = this->tempValueAddress;

                this->tempAccAddressStack.pop();
                addText("LD", to_string(this->tempAccAddressStack.top()));
            }
            else
            {
                operand = generateAssemblyLabel(symbol->id, symbol->scope);
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
