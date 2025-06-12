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
                        Semantic *semantic,
                        bool shouldLoad = true)
{
    auto *symbol = symTable.getSymbol(entry.value);

    if (isNumber(entry.value, false))
    {
        addText("LDI", entry.value);
    }
    else if (isBoolean(entry.value))
    {
        if (entry.value == "true")
        {
            addText("LDI", "1");
        }
        else if (entry.value == "false")
        {
            addText("LDI", "0");
        }
    }
    else if (entry.hasOwnScope)
    {
        if (entry.kind == ExpressionController::ExpressionsEntry::UNARY_OP)
        {
            emitUnaryOp(symTable, entry, semantic);
        }
        else
        {
            if (symbol->arraySize.size())
            {
                semantic->reduceExpressionAndGetType();
                if (shouldLoad)
                {
                    string label = generateAssemblyLabel(symbol->id, symbol->scope);
                    addText("STO", "$indr");
                    addText("LDV", label);
                }
            }
        }
    }
    else if (entry.entryType != SemanticTable::Types::INT)
        return;
    else
    {
        string label = generateAssemblyLabel(symbol->id, symbol->scope);
        addText("LD", label);
    }
}

void Assembly::emitUnaryOp(SymbolTable &symTable, ExpressionController::ExpressionsEntry &op, Semantic *semantic)
{
    // Get the right operand for the unary operation before reducing the stack
    auto right = semantic->getNextExpressionEntry();

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
    else if (op.unaryOperation == SemanticTable::OperationsUnary::INCREMENT)
    {
        if (op.value == "++")
        {
            addText("ADDI", "1");
        }
        else if (op.value == "--")
        {
            addText("SUBI", "1");
        }

        auto *symbol = symTable.getSymbol(right.value);
        string label = generateAssemblyLabel(symbol->id, symbol->scope);

        if (symbol->arraySize.size())
            addText("STOV", label);
        else
            addText("STO", label);
    }
}

void Assembly::emitBinaryOp(SymbolTable &symTable,
                            const ExpressionController::ExpressionsEntry &op,
                            ExpressionController::ExpressionsEntry &left,
                            ExpressionController::ExpressionsEntry &right,
                            Semantic *semantic)
{
    if (!left.value.empty())
    {
        // We always load the left operand, and the rights operand are added
        emitLoad(symTable, left, semantic);
    }

    if (!right.value.empty())
    {
        bool isRightNum = isNumber(right.value, false);
        bool isRightBool = isBoolean(right.value);
        string operand = right.value;

        if (!(isRightNum || isRightBool))
        {
            SymbolTable::SymbolInfo *symbol = symTable.getSymbol(right.value);

            if (right.hasOwnScope)
            {
                addText("STO", this->getTempAccAddress());
                this->addTempAccAddress();

                if (right.kind == ExpressionController::ExpressionsEntry::UNARY_OP)
                {
                    emitUnaryOp(symTable, right, semantic);
                }
                else if (symbol->arraySize.size())
                {
                    operand = generateAssemblyLabel(symbol->id, symbol->scope);
                    semantic->reduceExpressionAndGetType();
                    addText("STO", "$indr");
                    addText("LDV", operand);
                }

                addText("STO", this->tempValueAddress);
                operand = this->tempValueAddress;

                this->popTempAccAddress();
                addText("LD", this->getTempAccAddress());
            }
            else if (right.entryType != SemanticTable::Types::INT && right.entryType != SemanticTable::Types::BOOL)
                return;
            else
                operand = generateAssemblyLabel(symbol->id, symbol->scope);
        }

        if (op.binaryOperation == SemanticTable::OperationsBinary::SUM)
            addText(isRightNum ? "ADDI" : "ADD", operand);
        else if (op.binaryOperation == SemanticTable::OperationsBinary::SUBTRACTION)
            addText(isRightNum ? "SUBI" : "SUB", operand);
        else if (op.binaryOperation == SemanticTable::OperationsBinary::RELATION_HIGH)
        {
            addText(isRightNum ? "SUBI" : "SUB", operand);
            string label = generateAssemblyLabel("ELSE", semantic->symbolTable.currentScope);
            semantic->labelStack.push(label);
            if (op.value == ">")
            {
                addText("BLE", label);
            }
            else if (op.value == ">=")
            {
                addText("BLT", label);
            }
            else if (op.value == "<")
            {
                addText("BGE", label);
            }
            else if (op.value == "<=")
            {
                addText("BGT", label);
            }
        }
        else if (op.binaryOperation == SemanticTable::OperationsBinary::RELATION_LOW)
        {
            addText(isRightNum || isRightBool ? "SUBI" : "SUB", operand);
            string label = generateAssemblyLabel("ELSE", semantic->symbolTable.currentScope);
            semantic->labelStack.push(label);

            if (op.value == "==")
            {
                addText("BNE", label);
            }
            else if (op.value == "!=")
            {
                addText("BEQ", label);
            }
        }
        else if (op.binaryOperation == SemanticTable::OperationsBinary::LOGICAL)
        {
            if (op.value == "&&")
            {
                if (isRightBool)
                    addText("ANDI", operand == "true" ? "1" : "0");
                else
                    addText("AND", operand);
            }
            else if (op.value == "||")
            {
                if (isRightBool)
                    addText("ORI", operand == "true" ? "1" : "0");
                else
                    addText("OR", operand);
            }
        }
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

void Assembly::emitArrayAssignment(Semantic *semantic)
{
    semantic->reduceExpressionAndGetType(SemanticTable::__NULL, false, false);
    addText("STO", this->arrayIndexAddress);
}
