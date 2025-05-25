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
        this->assembly.addData(this->assembly.generateAssemblyLabel(this->currentSymbol->id, this->currentSymbol->scope), "0");

        break;
    }
    case 4:
    { // ASSIGNMENT VALUE
        SymbolTable::SymbolInfo *matchedSymbol =
            symbolTable.getSymbol(this->declarationSymbol->id);

        validateIfVariableIsDeclared(matchedSymbol, this->declarationSymbol->id);

        // * Assembly generation
        string label = this->assembly.generateAssemblyLabel(matchedSymbol->id, matchedSymbol->scope);
        this->assembly.addComment("Assign valute to " + label);
        this->assembly.addText("LDI", "0");

        reduceExpressionAndGetType(matchedSymbol->dataType, true);
        matchedSymbol->isInitialized = true;

        // * Assembly generation
        this->assembly.addText("STO", label);

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

        this->assembly.addData(this->assembly.generateAssemblyLabel(this->declarationSymbol->id, this->declarationSymbol->scope),
                               this->declarationSymbol->arraySize[0]);

        this->valueArraySizes.clear();
        break;
    }
    case 6: // ARRAY SIZE DECLARATION
    {
        if (this->expressionController.expressionStack.size() == 1)
        {
            ExpressionController::ExpressionsEntry entry = this->expressionController.expressionStack.top();
            if (entry.entryType == SemanticTable::INT)
            {
                int value = isNumber(entry.value, false) ? stoi(entry.value) : -1;

                this->declarationSymbol->arraySize.push_back(value);
                this->expressionController.expressionStack.pop();
            }
            else
            {
                throw SemanticError(SemanticError::InvalidValueForArrayLength());
            }
        }
        else
        {
            reduceExpressionAndGetType(SemanticTable::INT, true);
            this->declarationSymbol->arraySize.push_back(-1);
        }

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
    // * 11-20: Operators *
    case 11: // ASSIGN OP
    {
        SymbolTable::SymbolInfo *matchedSymbol =
            symbolTable.getSymbol(this->currentSymbol->id);

        validateIfVariableIsDeclared(matchedSymbol, this->currentSymbol->id);
        this->declarationSymbol = matchedSymbol;

        break;
    }
    case 12: // ARITHMETICAL ASSIGN OP (only for numbers)
    {
        if (lexeme == "-=")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            else
                get<2>(this->symbolEvaluateStack.top()).pushType(this->currentSymbol->dataType, this->currentSymbol->id);

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUBTRACTION);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::SUBTRACTION);
        }
        else if (lexeme == "*=")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            else
                get<2>(this->symbolEvaluateStack.top()).pushType(this->currentSymbol->dataType, this->currentSymbol->id);

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION);
        }
        else if (lexeme == "/=")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            else
                get<2>(this->symbolEvaluateStack.top()).pushType(this->currentSymbol->dataType, this->currentSymbol->id);

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::DIVISION);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::DIVISION);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 13: // ADD ASSIGN OP (for strings or numbers)

    {
        if (lexeme == "+=")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            else
                get<2>(this->symbolEvaluateStack.top()).pushType(this->currentSymbol->dataType, this->currentSymbol->id);

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUM);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::SUM);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 14: // REMAINDER ASSIGN OP (for integers)
    {
        if (lexeme == "%=")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            else
                get<2>(this->symbolEvaluateStack.top()).pushType(this->currentSymbol->dataType, this->currentSymbol->id);

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 15:
    { // NUMBER OP (for numbers)
        // Arithmetic operations
        if (lexeme == "+")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUM);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::SUM);
        }
        else if (lexeme == "-")
        {
            ExpressionController *expController = this->symbolEvaluateStack.empty()
                                                      ? &this->expressionController
                                                      : &get<2>(this->symbolEvaluateStack.top());
            if (expController->expressionStack.empty() || expController->expressionStack.top().kind == ExpressionController::ExpressionsEntry::BINARY_OP)
                expController->pushUnaryOp(SemanticTable::OperationsUnary::NEG);
            else
            {
                validateOneOfTypes({
                    SemanticTable::Types::INT,
                    SemanticTable::Types::FLOAT,
                    SemanticTable::Types::DOUBLE,
                });
                expController->pushBinaryOp(SemanticTable::OperationsBinary::SUBTRACTION);
            }
        }
        else if (lexeme == "*")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION);
        }
        else if (lexeme == "/")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::DIVISION);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::DIVISION);
        }
        // Comparisson operations
        else if (lexeme == "<")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        else if (lexeme == "<=")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        else if (lexeme == ">")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        else if (lexeme == ">=")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
                SemanticTable::Types::FLOAT,
                SemanticTable::Types::DOUBLE,
            });
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        // Increment and decrement operations (Don't validate, since the value appears after)
        else if (lexeme == "--")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT);
            else
                get<2>(this->symbolEvaluateStack.top()).pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT);
        }
        else if (lexeme == "++")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT);
            else
                get<2>(this->symbolEvaluateStack.top()).pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 16: // ADD OP (for numbers and strings)
    {
        validateOneOfTypes({
            SemanticTable::Types::INT,
            SemanticTable::Types::FLOAT,
            SemanticTable::Types::DOUBLE,
            SemanticTable::Types::STRING,
        });
        if (this->symbolEvaluateStack.empty())
            this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::SUM);
        else
            get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::SUM);

        break;
    }
    case 17:
    { // INTEGER OP (Bitwise and remainder, only for integers)

        if (lexeme == "&")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "|")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "^")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "~")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushUnaryOp(SemanticTable::OperationsUnary::BITWISE_NOT);
            else
                get<2>(this->symbolEvaluateStack.top()).pushUnaryOp(SemanticTable::OperationsUnary::BITWISE_NOT);
        }
        else if (lexeme == "<<")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == ">>")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::BITWISE, lexeme);
        }
        else if (lexeme == "%")
        {
            validateOneOfTypes({
                SemanticTable::Types::INT,
            });

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER);
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

            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::LOGICAL);
            else
                get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::LOGICAL);
        }
        else if (lexeme == "!")
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushUnaryOp(SemanticTable::OperationsUnary::NOT);
            else
                get<2>(this->symbolEvaluateStack.top()).pushUnaryOp(SemanticTable::OperationsUnary::NOT);
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

        if (this->symbolEvaluateStack.empty())
            this->expressionController.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_LOW);
        else
            get<2>(this->symbolEvaluateStack.top()).pushBinaryOp(SemanticTable::OperationsBinary::RELATION_LOW);

        break;

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
            if (this->expressionController.expressionStack.size() == 1)
            {
                SymbolTable::SymbolInfo *matchedSymbol = symbolTable.getSymbol(lexeme);

                if (!matchedSymbol)
                    throw SemanticError(SemanticError::SymbolUndeclared(lexeme));
                else if (matchedSymbol->arraySize.size() != this->functionSymbol->arraySize.size() || matchedSymbol->dataType != this->functionSymbol->dataType)
                    throw SemanticError("Invalid array for return of function " + this->functionSymbol->id);

                expressionController.expressionStack.pop();
                break;
            }
            throw SemanticError("Invalid return type for function " + this->functionSymbol->id);
        }

        if (this->expressionController.expressionStack.empty())
            throw SemanticError("Invalid return type for function " + this->functionSymbol->id);
        else
            reduceExpressionAndGetType(this->functionSymbol->dataType, true);

        break;
    }
    case 26: // INPUT
    {
        SymbolTable::SymbolInfo *matchedSymbol = this->symbolTable.getSymbol(this->currentSymbol->id);

        validateIfVariableIsDeclared(matchedSymbol, lexeme);
        validateIsVariable(matchedSymbol);
        validateSymbolClassification(matchedSymbol, SymbolTable::VARIABLE);

        this->expressionController.pushType(matchedSymbol->dataType, matchedSymbol->id);

        // Check if the expresion is valid
        validateOneOfTypes({SemanticTable::Types::CHAR,
                            SemanticTable::Types::STRING,
                            SemanticTable::Types::INT});

        // Since we are using this value only to check the type, we can pop it
        this->expressionController.expressionStack.pop();
        this->currentSymbol->isInitialized = true;

        // * Assembly generation
        this->assembly.addText("LD", "$in_port");
        this->assembly.addText("STO", this->assembly.generateAssemblyLabel(matchedSymbol->id, matchedSymbol->scope));

        break;
    }
    case 27: // OUTPUT
    {
        SemanticTable::Types result = reduceExpressionAndGetType(SemanticTable::Types ::__NULL, false, true);
        if (result == SemanticTable::Types::__NULL)
        {
            throw SemanticError("Invalid type in output expression");
        }

        // * Assembly generation
        this->assembly.addText("STO", "$out_port");

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

        SymbolTable::SymbolInfo *symbol = get<0>(this->symbolEvaluateStack.top());
        this->symbolEvaluateStack.pop();

        this->symbolEvaluateStack.empty()
            ? this->expressionController.pushType(symbol->dataType, symbol->id)
            : get<2>(this->symbolEvaluateStack.top()).pushType(symbol->dataType, symbol->id);

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
        if (this->symbolEvaluateStack.empty())
            this->expressionController.pushType(SemanticTable::INT, lexeme);
        else
            get<2>(this->symbolEvaluateStack.top()).pushType(SemanticTable::INT, lexeme);
        break;
    }
    case 32: // DECIMAL VALUE
    {
        if (lexeme[lexeme.length() - 1] == 'f')
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushType(SemanticTable::FLOAT, lexeme);
            else
                get<2>(this->symbolEvaluateStack.top()).pushType(SemanticTable::FLOAT, lexeme);
        }
        else
        {
            if (this->symbolEvaluateStack.empty())
                this->expressionController.pushType(SemanticTable::DOUBLE, lexeme);
            else
                get<2>(this->symbolEvaluateStack.top()).pushType(SemanticTable::DOUBLE, lexeme);
        }
        break;
    }
    case 33: // CHAR VALUE
    {
        if (this->symbolEvaluateStack.empty())
            this->expressionController.pushType(SemanticTable::CHAR, lexeme);
        else
            get<2>(this->symbolEvaluateStack.top()).pushType(SemanticTable::CHAR, lexeme);
        break;
    }
    case 34: // STRING VALUE
    {
        if (this->symbolEvaluateStack.empty())
            this->expressionController.pushType(SemanticTable::STRING, lexeme);
        else
            get<2>(this->symbolEvaluateStack.top()).pushType(SemanticTable::STRING, lexeme);
        break;
    }
    case 35: // BOOLEAN VALUE
    {
        if (this->symbolEvaluateStack.empty())
            this->expressionController.pushType(SemanticTable::BOOL, lexeme);
        else
            get<2>(this->symbolEvaluateStack.top()).pushType(SemanticTable::BOOL, lexeme);
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

        if (this->symbolEvaluateStack.empty())
            this->expressionController.pushType(symbol->dataType, lexeme);
        else
            get<2>(this->symbolEvaluateStack.top()).pushType(symbol->dataType, lexeme);

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

        this->symbolEvaluateStack.push(make_tuple(this->functionSymbol, 0, ExpressionController()));

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
            stack<ExpressionController::ExpressionsEntry> expStack = this->symbolEvaluateStack.empty()
                                                                         ? this->expressionController.expressionStack
                                                                         : get<2>(this->symbolEvaluateStack.top()).expressionStack;
            // if (expStack.size() == 1 && isNumber(expStack.top().value))
            //     this->arrayValues.push_back(expStack.top().value);
            // else
            //     this->arrayValues.push_back("-1");

            this->arrayValues.push_back("-1");

            // Set array index
            this->assembly.addText("LDI", to_string(arrayLengthsStack.top()));
            this->assembly.addText("STO", "$indr");

            // Set array value
            this->assembly.addText("LDI", "0");
            reduceExpressionAndGetType(this->pendingType, true);
            this->assembly.addText("STOV", this->assembly.generateAssemblyLabel(this->declarationSymbol->id, this->declarationSymbol->scope));

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
        auto [symbol, arrayDepth, expression] = this->symbolEvaluateStack.top();
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

                    get<2>(this->symbolEvaluateStack.top()).expressionStack.pop();
                }
                else
                {
                    throw SemanticError(SemanticError::InvalidValueForArrayLength());
                }
            }
            else
            {
                reduceExpressionAndGetType(SemanticTable::INT, true);
            }
            get<1>(this->symbolEvaluateStack.top())++;
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

        this->symbolEvaluateStack.push(make_tuple(matchedSymbol, 0, ExpressionController()));
        break;
    }
    case 56: // END ARRAY ACCESS
    {
        auto [symbol, arrayDepth, _] = this->symbolEvaluateStack.top();
        this->symbolEvaluateStack.pop();

        if (arrayDepth < symbol->arraySize.size())
            throw SemanticError("Invalid value from array access");

        this->symbolEvaluateStack.empty()
            ? this->expressionController.pushType(symbol->dataType, symbol->id)
            : get<2>(this->symbolEvaluateStack.top()).pushType(symbol->dataType, symbol->id);
        break;
    }
    case 61: // VALIDATE EXPRESSION
    {
        this->reduceExpressionAndGetType();
        break;
    }
    default:
        cout << "Ação não reconhecida: " << action << endl;
        break;
    }
}

// Criar pilha de escopo, usar contador numerico para controlar qual o escopo atual

// Ao encontrar um ID, adicionar esse simbolo na tabela de simbolos
// Ao encontrar um ID, verificar se ele já existe na tabela de simbolos

// Criar pilha de expressões
// Checar o operador da esquerda e da direita, e validar se eles são do mesmo tipo
// Ao encontrar um operando, coloca o tipo dele na pilha de expressões
// AO encontrar um operador, coloca ele na pilha de expressões

// Criar funcao auxiliar para verificar se o tipo é valido entre dois operadores/variaveis
// Permitir concatenacao entre strings e chars

// id = string
// type = enum
// scope = int
// was_used = bool
// was_initialized = bool
// array_dimensions = int
// array_size = int