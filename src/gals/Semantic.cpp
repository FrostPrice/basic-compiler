#include "Semantic.h"
#include "Constants.h"
#include "ExpressionController.hpp"

#include <iostream>
#include <string>

using namespace std;

void Semantic::executeAction(int action, const Token *token)
{
    string lexeme = token->getLexeme();

    cout << "Ação: " << action << ", Token: " << token->getId()
         << ", Lexema: " << lexeme << endl;

    switch (action)
    {
        // * 1-10: Declarations and assignments *
    case 1: // ID
    {
        this->currentSymbol = new SymbolTable::SymbolInfo(
            lexeme,                             // id
            this->pendingType,                  // type
            this->symbolTable.getCurrentScope() // scope
        );
        break;
    }
    case 2: // TYPE
    {
        if (lexeme == "int")
            this->pendingType = SemanticTable::INT;
        else if (lexeme == "float")
            this->pendingType = SemanticTable::FLOAT;
        else if (lexeme == "double")
            this->pendingType = SemanticTable::DOUBLE;
        else if (lexeme == "char")
            this->pendingType = SemanticTable::CHAR;
        else if (lexeme == "string")
            this->pendingType = SemanticTable::STRING;
        else if (lexeme == "bool")
            this->pendingType = SemanticTable::BOOL;
        else if (lexeme == "void")
            this->pendingType = SemanticTable::__NULL;
        else
            throw SemanticError("Invalid type: " + lexeme);

        break;
    }
    case 3: // DECLARATION
    {
        SymbolTable::SymbolInfo *alreadyDeclared =
            symbolTable.getSymbolInScope(this->currentSymbol->id, this->symbolTable.getCurrentScope());

        if (alreadyDeclared != nullptr)
        {
            validateDuplicateSymbolInSameScope(alreadyDeclared);
        }

        this->currentSymbol->symbolClassification = SymbolTable::VARIABLE;
        this->declarationSymbol = this->currentSymbol;
        this->symbolTable.addSymbol(*this->currentSymbol);

        // * Assembly generation
        this->assembly.addData(this->assembly.generateAssemblyLabel(this->currentSymbol->id, this->currentSymbol->scope, false), "0");

        break;
    }
    case 4:
    { // ASSIGNMENT VALUE
        SymbolTable::SymbolInfo *matchedSymbol =
            symbolTable.getSymbol(this->declarationSymbol->id);

        validateIfVariableIsDeclared(matchedSymbol, this->declarationSymbol->id);

        // * Assembly generation
        string label = this->assembly.generateAssemblyLabel(matchedSymbol->id, matchedSymbol->scope, false);
        this->assembly.addComment("Assigning value to " + label);

        reduceExpressionAndGetType(matchedSymbol->dataType, true, true);
        matchedSymbol->isInitialized = true;

        // * Assembly generation
        if (matchedSymbol->arraySize.size())
        {
            this->assembly.addText("STO", this->assembly.tempValueAddress);
            this->assembly.addText("LD", this->assembly.arrayIndexAddress);
            this->assembly.addText("STO", "$indr");
            this->assembly.addText("LD", this->assembly.tempValueAddress);
            this->assembly.addText("STOV", label);
        }
        else
        {
            this->assembly.addText("STO", label);
        }
        this->assembly.addBlankLine();

        this->declarationSymbol = nullptr;

        break;
    }
    case 5: // ASSIGNMENT ARRAY VALUE
    {
        if (this->valueArraySizes.size())
        {
            for (int i = 0; i < this->declarationSymbol->arraySize.size(); i++)
            {
                int symbolArraySize = this->declarationSymbol->arraySize[i];

                if (this->valueArraySizes[i].size() == 0)
                {
                    throw SemanticError(to_string(this->declarationSymbol->arraySize.size()) +
                                        " dimensions expected for array value, but got " +
                                        to_string(i));
                }

                for (int arraySize : this->valueArraySizes[i])
                {
                    if (symbolArraySize != arraySize && symbolArraySize != -1)
                    {
                        throw SemanticError("Array size mismatch: expected " + to_string(symbolArraySize) + " but got " + to_string(arraySize));
                    }
                }
            }
        }

        this->declarationSymbol->isInitialized = true;
        this->declarationSymbol->symbolClassification = SymbolTable::ARRAY;

        this->symbolTable.addSymbol(*this->declarationSymbol);

        this->assembly.addData(this->assembly.generateAssemblyLabel(this->declarationSymbol->id, this->declarationSymbol->scope, false),
                               this->declarationSymbol->arraySize[0]);

        this->valueArraySizes.clear();

        this->declarationSymbol = nullptr;
        break;
    }
    case 6: // ARRAY SIZE DECLARATION
    {
        int arraySize = stoi(lexeme);

        if (arraySize <= 1)
        {
            throw SemanticError("Bip architecture does not support arrays of size less than 2");
        }

        this->declarationSymbol->arraySize.push_back(arraySize);

        break;
    }
    case 7: // ARRAY ASSIGNMENT VALUE
    {
        this->arrayDepth = -1;

        for (int i = 0; i < this->declarationSymbol->arraySize.size(); i++)
        {
            this->valueArraySizes.push_back(vector<int>());
        }

        while (!this->arrayLengthsStack.empty())
        {
            this->arrayLengthsStack.pop();
        }
        break;
    }
    case 8:
    {
        SymbolTable::SymbolInfo *alreadyDeclared =
            symbolTable.getSymbolInScope(this->currentSymbol->id, this->symbolTable.getCurrentScope());

        if (alreadyDeclared != nullptr)
        {
            validateDuplicateSymbolInSameScope(alreadyDeclared);
        }

        this->declarationSymbol = this->currentSymbol;

        break;
    }
    case 9:
    {
        SymbolTable::SymbolInfo *array =
            symbolTable.getSymbolInScope(this->currentSymbol->id, this->symbolTable.getCurrentScope());

        array->isUsed = true; // Mark the array as used
    }
    // * 11-20: Operators *
    case 11: // ASSIGN OP
    {
        SymbolTable::SymbolInfo *matchedSymbol;
        if (!this->declarationSymbol)
            matchedSymbol = symbolTable.getSymbol(this->currentSymbol->id);
        else
            matchedSymbol = this->declarationSymbol;

        validateIfVariableIsDeclared(matchedSymbol, this->currentSymbol->id);
        this->declarationSymbol = matchedSymbol;

        break;
    }
    case 12: // ARITHMETICAL ASSIGN OP (only for numbers)
    {
        SymbolTable::SymbolInfo *matchedSymbol;
        if (!this->declarationSymbol)
            matchedSymbol = symbolTable.getSymbol(this->currentSymbol->id);
        else
            matchedSymbol = this->declarationSymbol;

        validateIfVariableIsDeclared(matchedSymbol, this->currentSymbol->id);
        this->declarationSymbol = matchedSymbol;

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);

        if (lexeme == "-=")
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUBTRACTION, lexeme);
        else if (lexeme == "*=")
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION, lexeme);
        else if (lexeme == "/=")
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::DIVISION, lexeme);
        else
            throw SemanticError("Invalid operator: " + lexeme);

        break;
    }
    case 13: // ADD ASSIGN OP (for strings or numbers)
    {        // +=
        SymbolTable::SymbolInfo *matchedSymbol;
        if (!this->declarationSymbol)
            matchedSymbol = symbolTable.getSymbol(this->currentSymbol->id);
        else
            matchedSymbol = this->declarationSymbol;

        validateIfVariableIsDeclared(matchedSymbol, this->currentSymbol->id);
        this->declarationSymbol = matchedSymbol;

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUM, lexeme);

        break;
    }
    case 14: // REMAINDER ASSIGN OP (for integers)
    {        // %=
        SymbolTable::SymbolInfo *matchedSymbol;
        if (!this->declarationSymbol)
            matchedSymbol = symbolTable.getSymbol(this->currentSymbol->id);
        else
            matchedSymbol = this->declarationSymbol;

        validateIfVariableIsDeclared(matchedSymbol, this->currentSymbol->id);
        this->declarationSymbol = matchedSymbol;

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER, lexeme);

        break;
    }
    case 15:
    { // NUMBER OP (for numbers)
        // Arithmetic operations
        if (lexeme == "-")
        {
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUBTRACTION, lexeme);
        }
        else if (lexeme == "*")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION, lexeme);
        }
        else if (lexeme == "/")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::DIVISION, lexeme);
        }
        // Comparisson operations
        else if (lexeme == "<")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH, lexeme);
        }
        else if (lexeme == "<=")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH, lexeme);
        }
        else if (lexeme == ">")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH, lexeme);
        }
        else if (lexeme == ">=")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH, lexeme);
        }
        // Increment and decrement operations (Don't validate, since the value appears after)
        else if (lexeme == "--")
        {
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT, lexeme);
            this->createExpressionScope();
        }
        else if (lexeme == "++")
        {
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT, lexeme);
            this->createExpressionScope();
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 16: // ADD OP (for numbers and strings)
    {
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUM, lexeme);

        break;
    }
    case 17:
    { // INTEGER OP (Bitwise and remainder, only for integers)

        if (lexeme == "&")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "|")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "^")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "~")
        {
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushUnaryOp(SemanticTable::OperationsUnary::BITWISE_NOT, lexeme);

            this->createExpressionScope();
        }
        else if (lexeme == "<<")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == ">>")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "%")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER, lexeme);
        }
        else // Fallback em caso de esquecermos alguma operacao
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 18: // BOOLEAN OP (for booleans)
        if (lexeme == "&&" || lexeme == "||")
        {

            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::LOGICAL, lexeme);
        }
        else if (lexeme == "!")
        {
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushUnaryOp(SemanticTable::OperationsUnary::NOT, lexeme);
            this->createExpressionScope();
        }
        else
        {
            throw SemanticError("Invalid boolean operator: " + lexeme);
        }
        break;
    case 19: // EQ NE OP (For everything)
        validateOneOfTypes({
            SemanticTable::Types::INT,
            SemanticTable::Types::FLOAT,
            SemanticTable::Types::DOUBLE,
            SemanticTable::Types::STRING,
            SemanticTable::Types::BOOL,
        });

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_LOW, lexeme);

        break;

    case 20: // UNARY NEGATION OP
    {
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushUnaryOp(SemanticTable::OperationsUnary::NEG, lexeme);

        createExpressionScope();

        break;
    }
    // * 21-30: Functions, blocks, I/O *
    case 21: // FUNCTION_DEF_PARAMETER
    {
        int newScope = this->symbolTable.getNextScope(); // Predict the next scope
        this->currentSymbol->scope = newScope;

        // Validation that a function already exists is made in the sintactic
        // You cannot have a parameter without a function
        SymbolTable::SymbolInfo *functionSymbol = this->symbolTable.getFunctionInScope();
        functionSymbol->functionParams++;

        this->currentSymbol->symbolClassification = SymbolTable::PARAM;
        this->currentSymbol->functionId = functionSymbol->id;
        this->currentSymbol->isInitialized = true;
        this->currentSymbol->arraySize = this->functionArraySizes;
        this->symbolTable.addSymbol(*this->currentSymbol);

        this->functionArraySizes.clear();

        break;
    }
    case 22: // FUNCTION_CALL_PARAMETER
    {
        if (!this->functionSymbol)
            throw SemanticError("Function not found in scope");

        vector<SymbolTable::SymbolInfo *> params = this->symbolTable.getFunctionParams(this->functionSymbol->scope + 1);

        SymbolTable::SymbolInfo *currentParam = params[this->parametersCountInFuncCall];

        if (!currentParam)
            throw SemanticError("Too many parameters in function call");

        reduceExpressionAndGetType(currentParam->dataType, true);

        this->parametersCountInFuncCall++;

        break;
    }
    case 23: // BLOCK_INIT
    {
        this->symbolTable.enterScope();

        break;
    }

    case 24: // BLOCK_END
    {
        this->symbolTable.exitScope();
        if (this->functionSymbol != nullptr)
        {
            if (this->functionSymbol->scope == this->symbolTable.getCurrentScope())
            {
                if (!this->functionSymbol->hasReturn)
                    throw SemanticError("Function " + this->functionSymbol->id + " has no return statement");
                this->functionSymbol = nullptr;
            }
        }
        break;
    }
    case 25: // RETURN
    {
        this->functionSymbol = symbolTable.getEnclosingFunction(this->symbolTable.getCurrentScope());
        this->functionSymbol->hasReturn = true;
        if (!this->functionSymbol)
            throw SemanticError("Function not found in scope");

        if (this->functionSymbol->dataType == SemanticTable::Types::__NULL)
        {
            if (lexeme != "return")
                throw SemanticError("Function " + this->functionSymbol->id + " has no return type");
            break;
        }

        if (!this->functionSymbol->arraySize.empty())
        {
            if (this->expressionScopeList.front().expressionController.expressionStack.size() == 1)
            {
                SymbolTable::SymbolInfo *matchedSymbol = symbolTable.getSymbol(lexeme);

                if (!matchedSymbol)
                    throw SemanticError(SemanticError::SymbolUndeclared(lexeme));
                else if (matchedSymbol->arraySize.size() != this->functionSymbol->arraySize.size() || matchedSymbol->dataType != this->functionSymbol->dataType)
                    throw SemanticError("Invalid array for return of function " + this->functionSymbol->id);

                expressionScopeList.front().expressionController.expressionStack.pop();
                break;
            }
            throw SemanticError("Invalid return type for function " + this->functionSymbol->id);
        }

        if (this->expressionScopeList.front().expressionController.expressionStack.empty())
            throw SemanticError("Invalid return type for function " + this->functionSymbol->id);
        else
            reduceExpressionAndGetType(this->functionSymbol->dataType, true);

        break;
    }
    case 26: // INPUT
    {
        SymbolTable::SymbolInfo *matchedSymbol;
        if (!this->declarationSymbol)
            matchedSymbol = symbolTable.getSymbol(this->currentSymbol->id);
        else
            matchedSymbol = this->declarationSymbol;

        validateIfVariableIsDeclared(matchedSymbol, lexeme);
        validateIsVariable(matchedSymbol);

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(matchedSymbol->dataType, matchedSymbol->id);

        // Check if the expresion is valid
        validateOneOfTypes({SemanticTable::Types::CHAR,
                            SemanticTable::Types::STRING,
                            SemanticTable::Types::INT});

        // Since we are using this value only to check the type, we can pop it
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.expressionStack.pop();
        this->currentSymbol->isInitialized = true;

        // * Assembly generation
        string label = this->assembly.generateAssemblyLabel(matchedSymbol->id, matchedSymbol->scope, false);
        this->assembly.addComment("Getting input for " + label);
        if (matchedSymbol->arraySize.size())
        {
            reduceExpressionAndGetType(matchedSymbol->dataType, true, false);
            this->assembly.addText("STO", "$indr");
            this->assembly.addText("LD", "$in_port");
            this->assembly.addText("STOV", label);
        }
        else
        {
            this->assembly.addText("LD", "$in_port");
            this->assembly.addText("STO", label);
        }

        this->assembly.addBlankLine();

        break;
    }
    case 27: // OUTPUT
    {
        // * Assembly generation
        this->assembly.addComment("Printing value");

        SemanticTable::Types result = reduceExpressionAndGetType(SemanticTable::Types ::__NULL, false, true);
        if (result == SemanticTable::Types::__NULL)
        {
            throw SemanticError("Invalid type in output expression");
        }

        // * Assembly generation
        this->assembly.addText("STO", "$out_port");
        this->assembly.addBlankLine();

        break;
    }
    case 28: // FUNCTION DEF
    {
        SymbolTable::SymbolInfo *matchedSymbol =
            symbolTable.getSymbolInScope(this->currentSymbol->id, this->symbolTable.getCurrentScope());

        if (matchedSymbol != nullptr)
        {
            validateDuplicateSymbolInSameScope(matchedSymbol);
        }

        // Check if is inside a function
        SymbolTable::SymbolInfo *functionSymbol = this->symbolTable.getFunctionInScope();
        if (functionSymbol != nullptr && this->symbolTable.getCurrentScope() != 0)
        {
            throw SemanticError("Function cannot be declared inside another function");
        }

        if (this->currentSymbol->dataType == SemanticTable::Types::__NULL)
            this->currentSymbol->hasReturn = true;

        this->currentSymbol->symbolClassification = SymbolTable::FUNCTION;
        this->currentSymbol->isInitialized = true;
        this->currentSymbol->functionParams = 0; // Starts with 0 parameters
        this->currentSymbol->arraySize = this->functionArraySizes;
        this->symbolTable.addSymbol(*this->currentSymbol);

        this->functionSymbol = this->currentSymbol;

        this->functionArraySizes.clear();

        break;
    }
    case 29: // FUNCTION CALL
    {
        if (!this->functionSymbol)
            throw SemanticError("Function not found in scope");
        validateFunctionParamCount(this->functionSymbol);

        this->parametersCountInFuncCall = 0;

        SymbolTable::SymbolInfo *symbol = this->expressionScopeList[this->expressionScopeIndexes.top()].symbol;

        this->expressionScopeIndexes.pop();

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(symbol->dataType, symbol->id);

        break;
    }
    case 30: // FUNCTION_DEF_ARRAY
    {
        this->functionArraySizes.push_back(-1);
        break;
    }

    // * 31-40: Primitive values *
    case 31: // INT VALUE
    {
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(SemanticTable::INT, lexeme);
        this->closeUnaryScopeIfNeeded();

        break;
    }
    case 32: // DECIMAL VALUE
    {
        if (lexeme[lexeme.length() - 1] == 'f')
        {
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(SemanticTable::FLOAT, lexeme);
        }
        else
        {
            this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(SemanticTable::DOUBLE, lexeme);
        }
        break;
    }
    case 33: // CHAR VALUE
    {
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(SemanticTable::CHAR, lexeme);
        break;
    }
    case 34: // STRING VALUE
    {
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(SemanticTable::STRING, lexeme);
        break;
    }
    case 35: // BOOLEAN VALUE
    {
        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(SemanticTable::BOOL, lexeme);
        break;
    }
    case 36: // SYMBOL VALUE
    {
        SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(this->currentSymbol->id);
        if (symbol == nullptr)
            throw SemanticError(SemanticError::SymbolUndeclared(this->currentSymbol->id));
        // ! Nao podemos restringir o uso de variaveis nao inicializadas, apenas jogar o warning
        else if (!symbol->isInitialized)
            Semantic::warnings.push_back("Warning: Symbol '" + symbol->id + "' was not initialized but was used.");
        else if (symbol->symbolClassification == SymbolTable::FUNCTION)
            throw SemanticError(SemanticError::FunctionNotCalled(this->currentSymbol->id));

        symbol->isUsed = true;

        if (symbol->symbolClassification == SymbolTable::ARRAY || symbol->arraySize.size() > 0)
            break;

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(symbol->dataType, lexeme);
        this->closeUnaryScopeIfNeeded();

        break;
    }
    case 37: // FUNCTION RETURN VALUE
    {
        SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(this->currentSymbol->id);

        if (symbol == nullptr)
        {
            throw SemanticError("Function not found in scope");
        }

        symbol->isUsed = true;
        this->functionSymbol = symbol;

        this->createExpressionScope(this->functionSymbol);

        break;
    }

    // * 41-50: Conditionals and loops *
    case 41: // IF CONDITION
    {
        reduceExpressionAndGetType(SemanticTable::Types::BOOL, true);

        break;
    }
    case 42: // SWITCH EXPRESSION
    {
        validateOneOfTypes({
            SemanticTable::Types::INT,
            SemanticTable::Types::FLOAT,
            SemanticTable::Types::DOUBLE,
            SemanticTable::Types::CHAR,
            SemanticTable::Types::STRING,
            SemanticTable::Types::BOOL,
        });

        this->switchResultType = reduceExpressionAndGetType();

        break;
    }
    case 43: // CASE VALUE
    {
        reduceExpressionAndGetType(this->switchResultType, true);

        break;
    }
    case 44: // WHILE CONDITION
    {
        reduceExpressionAndGetType();

        loopDepth++; // Entering a loop

        break;
    }
    case 45: // DO WHILE CONDITION
    {
        reduceExpressionAndGetType();
        loopDepth++; // Entering a loop
        break;
    }
    case 46: // FOR ASSIGNMENT OR DECLARATION
    {
        loopDepth++;
        break;
    }
    case 47: // FOR CONDITION
    {
        SemanticTable::Types condType = reduceExpressionAndGetType();
        if (condType != SemanticTable::Types::BOOL && condType != SemanticTable::Types::INT)
            throw SemanticError("Invalid FOR condition expression type.");
        break;
    }
    case 48: // FOR INCREMENT
    {
        SymbolTable::SymbolInfo *sym = symbolTable.getSymbol(currentSymbol->id);
        validateIfVariableIsDeclared(sym, currentSymbol->id);
        reduceExpressionAndGetType();
        break;
    }
    case 49: // BREAK
    {
        if (loopDepth == 0 && switchDepth == 0)
            throw SemanticError("BREAK used outside of a loop or switch.");

        break;
    }
    case 50: // CONTINUE
    {
        if (loopDepth == 0)
            throw SemanticError("CONTINUE used outside of a loop.");
        break;
    }
    case 51: // ARRAY VALUE
    {
        if (this->arrayDepth == this->declarationSymbol->arraySize.size() - 1)
        {
            string label = this->assembly.generateAssemblyLabel(this->declarationSymbol->id, this->declarationSymbol->scope, false);
            this->assembly.addComment("Assigning value to " + label + "[" + to_string(arrayLengthsStack.top()) + "]");

            // Set array index
            this->assembly.addText("LDI", to_string(arrayLengthsStack.top()));
            this->assembly.addText("STO", "$indr");

            // Set array value
            reduceExpressionAndGetType(this->pendingType, true);
            this->assembly.addText("STOV", label);

            this->assembly.addBlankLine();
        }
        this->arrayLengthsStack.top()++;
        break;
    }
    case 52: // ARRAY DEPTH IN
    {
        this->arrayDepth++;

        if (this->arrayDepth >= this->declarationSymbol->arraySize.size())
        {
            throw SemanticError("Array length exceeded");
        }

        this->arrayLengthsStack.push(0);
        break;
    }
    case 53: // ARRAY DEPTH OUT
    {
        int arraySize = this->declarationSymbol->arraySize[this->arrayDepth];
        if (this->arrayLengthsStack.top() > arraySize && arraySize != -1)
            throw SemanticError("Array length exceeded");

        this->valueArraySizes[this->arrayDepth].push_back(this->arrayLengthsStack.top());

        this->arrayDepth--;
        this->arrayLengthsStack.pop();
        break;
    }
    case 54: // ARRAY ACCESS
    {
        auto [symbol, arrayDepth, expression] = this->expressionScopeList[this->expressionScopeIndexes.top()];
        if (arrayDepth < (int)symbol->arraySize.size())
        {
            if (expression.expressionStack.size() == 1)
            {
                ExpressionController::ExpressionsEntry entry = expression.expressionStack.top();
                if (entry.entryType == SemanticTable::INT)
                {
                    int value = isNumber(entry.value, false) ? stoi(entry.value) : -1;

                    if (value >= symbol->arraySize[arrayDepth] && symbol->arraySize[arrayDepth] != -1)
                        throw SemanticError("Array size exceeded");
                }
                else
                {
                    throw SemanticError(SemanticError::InvalidValueForArrayLength());
                }
            }

            this->expressionScopeList[this->expressionScopeIndexes.top()].arrayDepth++;
        }
        else
        {
            throw SemanticError("Array dimension exceeded");
        }
        break;
    }
    case 55: // ARRAY SYMBOL
    {
        SymbolTable::SymbolInfo *matchedSymbol = symbolTable.getSymbol(this->currentSymbol->id);
        validateIfVariableIsDeclared(matchedSymbol, this->currentSymbol->id);

        if (matchedSymbol->symbolClassification != SymbolTable::ARRAY && matchedSymbol->arraySize.empty())
        {
            throw SemanticError(SemanticError::SymbolNotOfClassification(matchedSymbol->id));
        }

        this->expressionScopeList[this->expressionScopeIndexes.top()].expressionController.pushType(matchedSymbol->dataType, matchedSymbol->id, true);

        this->createExpressionScope(matchedSymbol);

        if (!this->declarationSymbol)
            this->declarationSymbol = matchedSymbol;

        break;
    }
    case 56: // END ARRAY ACCESS
    {
        auto [symbol, arrayDepth, _] = this->expressionScopeList[this->expressionScopeIndexes.top()];

        if (arrayDepth < (int)symbol->arraySize.size())
            throw SemanticError("Invalid value from array access");

        this->expressionScopeIndexes.pop();

        this->closeUnaryScopeIfNeeded();
        break;
    }
    case 57: // ARRAY VALUE ASSIGNMENT
    {
        SymbolTable::SymbolInfo *matchedSymbol = symbolTable.getSymbol(this->declarationSymbol->id);
        if (matchedSymbol->arraySize.size())
        {
            string label = this->assembly.generateAssemblyLabel(matchedSymbol->id, matchedSymbol->scope, false);
            this->assembly.addComment("Storing array index for " + label);

            this->assembly.emitArrayAssignment(this);
            this->assembly.addBlankLine();
        }
        break;
    }
    case 61: // VALIDATE EXPRESSION
    {
        this->reduceExpressionAndGetType();
        break;
    }
    case 64: // IF END BLOCK
    {
        string label = this->labelStack.top();
        this->labelStack.pop();
        this->assembly.addText(label, "");

        break;
    }
    case 65: // While End Block
    {
        string label = this->labelStack.top();
        this->labelStack.pop();
        this->assembly.addText(label, "");

        break;
    }
    case 66: // DI WHILE INIT BLOCK
    {
        break;
    }
    default:
        cout << "Ação não reconhecida: " << action << endl;
        break;
    }
}
