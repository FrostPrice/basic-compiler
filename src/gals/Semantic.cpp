#include "Semantic.h"
#include "Constants.h"

#include <iostream>
#include <string>

using namespace std;

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
        if (this->isDeclarating)
        {
            // DECLARATION — create the symbol
            this->currentSymbol = new SymbolTable::SymbolInfo(
                lexeme,                // id
                this->pendingType,     // type
                this->declarationScope // scope
            );

            SymbolTable::SymbolInfo *alreadyDeclared =
                symbolTable.getSymbolInCurrentScope(this->currentSymbol->id);

            if (alreadyDeclared != nullptr)
            {
                if (alreadyDeclared->symbolClassification == SymbolTable::PARAM)
                    return;

                validateDuplicateSymbolInSameScope(alreadyDeclared);
            }

            this->currentSymbol = this->symbolTable.addSymbol(*this->currentSymbol);
        }
        else
        {
            // USAGE or ASSIGNMENT — fetch from any valid scope
            currentSymbol = symbolTable.getSymbol(lexeme);
            validateIfVariableIsDeclared(currentSymbol, lexeme);
            this->currentSymbol->isUsed = true; // Mark as used
        }

        break;
    }
    case 2: // TYPE
    {
        this->isDeclarating = true;
        this->declarationScope = symbolTable.getCurrentScope();

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

        break;
    }
    case 3: // DECLARATION
    {
        this->isDeclarating = false;
        this->declarationScope = -1;
        break;
    }
    case 4:
    { // ASSIGNMENT VALUE
        validateIfVariableIsDeclared(this->currentSymbol, lexeme);
        // assign value to variable
        validateExpressionType(this->currentSymbol->dataType);
        this->currentSymbol->isInitialized = true;
        this->currentSymbol->symbolClassification = SymbolTable::VARIABLE;

        break;
    }
    case 5: // ASSIGNMENT ARRAY VALUE
    {
        for (int i = 0; i < this->currentSymbol->arraySize.size(); i++)
        {
            int arraySize = this->currentSymbol->arraySize[i];
            if (arraySize != this->valueArraySizes[i] && arraySize != -1)
            {
                throw SemanticError("Array size mismatch: expected " + to_string(arraySize) + " but got " + to_string(this->valueArraySizes[i]));
            }
        }
        this->currentSymbol->isInitialized = true;
        this->currentSymbol->symbolClassification = SymbolTable::ARRAY;

        this->symbolTable.addSymbol(*this->currentSymbol);
        break;
    }
    case 6: // ARRAY SIZE DECLARATION
    {
        // TODO validate array size for negative values
        if (/*this->isRawValue && */ lexeme != "0" && isNumber(lexeme, false))
        {
            this->currentSymbol->arraySize.push_back(stoi(lexeme));
        }
        else if (/*this->validateExpressionType(SemanticTable::INT)*/ true)
        {
            this->currentSymbol->arraySize.push_back(-1);
        }
        else
        {
            throw SemanticError("Invalid array size");
        }
        break;
    }
    case 7: // ARRAY ASSIGNMENT VALUE
    {
        this->arrayDepth = -1;
        this->valueArraySizes.clear();
        for (int i = 0; i < this->currentSymbol->arraySize.size(); i++)
        {
            this->valueArraySizes.push_back(0);
        }
        break;

        while (!this->arrayLengthsStack.empty())
        {
            this->arrayLengthsStack.pop();
        }
    }
    // * 11-20: Operators *
    // TODO: COMO FAZER ESSAS EXPRESSOES DE ASSIGNMENT (+=, -=, etc)?
    // ! PRECISA DESSE ASSIGNEMNT, tendo em vista que o operador de atribuicao eh o '=' ?
    // case 11: // ASSIGN OP
    // {

    //     break;
    // }
    case 12: // ARITHMETICAL ASSIGN OP (only for numbers)
    {
        // a -= 2; -> a = a - 2;
        // No momento que essa acao eh disparada, nao teremos o valor 2
        if (lexeme == "-=")
        {
            this->symbolTable.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::SUBTRACTION);
        }
        else if (lexeme == "*=")
        {
            // a *= 2; -> a = a * 2;
            // No momento que essa acao eh disparada, nao teremos o valor 2

            this->symbolTable.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION);
        }
        else if (lexeme == "/=")
        {
            // a /= 2; -> a = a / 2;
            // No momento que essa acao eh disparada, nao teremos o valor 2

            this->symbolTable.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::DIVISION);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 13: // ADD ASSIGN OP (for strings or numbers)

    {
        // a += 2; -> a = a + 2;
        // No momento que essa acao eh disparada, nao teremos o valor 2
        if (lexeme == "+=")
        {
            this->symbolTable.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::SUM);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 14: // REMAINDER ASSIGN OP (for integers)
    {
        // a %= 2; -> a = a % 2;
        // No momento que essa acao eh disparada, nao teremos o valor
        if (lexeme == "%=")
        {
            this->symbolTable.pushType(this->currentSymbol->dataType, this->currentSymbol->id);
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }

        break;
    }
    case 15:
    { // NUMBER OP (for numbers)
        validateOneOfTypes({
            SemanticTable::Types::INT,
            SemanticTable::Types::FLOAT,
            SemanticTable::Types::DOUBLE,
        });

        // Arithmetic operations
        if (lexeme == "+")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::SUM);
        }
        else if (lexeme == "-")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::SUBTRACTION);
        }
        else if (lexeme == "*")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::MULTIPLICATION);
        }
        else if (lexeme == "/")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::DIVISION);
        }
        // Comparisson operations
        else if (lexeme == "<")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        else if (lexeme == "<=")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        else if (lexeme == ">")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        else if (lexeme == ">=")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_HIGH);
        }
        // Increment and decrement operations
        else if (lexeme == "--")
        {
            this->symbolTable.pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT);
        }
        else if (lexeme == "++")
        {
            this->symbolTable.pushUnaryOp(SemanticTable::OperationsUnary::INCREMENT);
        }
        else
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }
        // TODO: Tem mais operacoes aqui?

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
        this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::SUM);

        break;
    }
    case 17: // INTEGER OP (Bitwise and remainder, only for integers)
        validateOneOfTypes({
            SemanticTable::Types::INT,
        });

        if (lexeme == "&")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE);
        }
        else if (lexeme == "|")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE);
        }
        else if (lexeme == "^")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE);
        }
        else if (lexeme == "~")
        {
            this->symbolTable.pushUnaryOp(SemanticTable::OperationsUnary::BITWISE_NOT);
        }
        else if (lexeme == "<<")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE);
        }
        else if (lexeme == ">>")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::BITWISE);
        }
        else if (lexeme == "%")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::REMAINDER);
        }
        else // Fallback em caso de esquecermos alguma operacao
        {
            throw SemanticError("Invalid operator: " + lexeme);
        }
        // TODO: Tem mais operacoes aqui?

        break;
    case 18: // BOOLEAN OP (for booleans)
        validateOneOfTypes({
            SemanticTable::Types::BOOL,
        });

        if (lexeme == "&&" || lexeme == "||")
        {
            this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::LOGICAL);
        }
        else if (lexeme == "!")
        {
            this->symbolTable.pushUnaryOp(SemanticTable::OperationsUnary::NOT);
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

        this->symbolTable.pushBinaryOp(SemanticTable::OperationsBinary::RELATION_LOW);

        break;

    // * 21-30: Functions, blocks, I/O *
    case 21: // FUNCTION_DEF_PARAMETER
    {
        this->currentSymbol->symbolClassification = SymbolTable::PARAM;
        this->currentSymbol->scope = this->symbolTable.getCurrentScope() + 1;
        this->currentSymbol->isInitialized = true;

        break;
    }
    case 22: // FUNCTION_CALL_PARAMETER
    {
        SymbolTable::SymbolInfo *functionSymbol = this->symbolTable.getSymbol(this->currentSymbol->id);

        if (functionSymbol == nullptr)
            throw SemanticError("Function '" + this->currentSymbol->id + "' not found");

        if (functionSymbol->symbolClassification != SymbolTable::FUNCTION)
            throw SemanticError("Symbol '" + this->currentSymbol->id + "' is not a function");

        SemanticTable::Types expectedType = functionSymbol->dataType;

        validateExpressionType(expectedType);

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
        break;
    }
    case 25: // RETURN
    {
        SymbolTable::SymbolInfo *currentScopeSymbol = this->symbolTable.getFunctionScope();

        validateReturnStatementScope(currentScopeSymbol);

        SemanticTable::Types expectedReturnType = currentScopeSymbol->dataType;

        validateExpressionType(expectedReturnType);

        break;
    }
    case 26: // INPUT
    {
        SymbolTable::SymbolInfo *matchedSymbol = this->symbolTable.getSymbol(this->currentSymbol->id);

        validateIfVariableIsDeclared(matchedSymbol, lexeme);

        validateIsVariable(matchedSymbol);

        validateSymbolClassification(matchedSymbol, SymbolTable::VARIABLE);

        this->currentSymbol->isInitialized = true;

        break;
    }
    case 27: // OUTPUT
    {
        validateOneOfTypes({SemanticTable::STRING, SemanticTable::CHAR});

        break;
    }
    case 28: // FUNCTION DEF
    {
        this->currentSymbol->symbolClassification = SymbolTable::FUNCTION;
        this->isDeclarating = false;

        break;
    }
    // * 31-40: Primitive values *
    case 31: // INT VALUE
    {
        this->symbolTable.pushType(SemanticTable::INT, lexeme);
        break;
    }
    case 32: // DECIMAL VALUE
    {
        if (lexeme[lexeme.length() - 1] == 'f')
        {
            this->symbolTable.pushType(SemanticTable::FLOAT, lexeme);
        }
        else
        {
            this->symbolTable.pushType(SemanticTable::DOUBLE, lexeme);
        }
        break;
    }
    case 33: // CHAR VALUE
    {
        this->symbolTable.pushType(SemanticTable::CHAR, lexeme);
        break;
    }
    case 34: // STRING VALUE
    {
        this->symbolTable.pushType(SemanticTable::STRING, lexeme);
        break;
    }
    case 35: // BOOLEAN VALUE
    {
        this->symbolTable.pushType(SemanticTable::BOOL, lexeme);
        break;
    }

    // * 41-50: Conditionals and loops *
    case 41: // IF CONDITION
        break;
    case 42: // SWITCH EXPRESSION
        break;
    case 43: // CASE VALUE
        break;
    case 44: // WHILE CONDITION
        break;
    case 45: // DO WHILE CONDITION
        break;
    case 46: // FOR ASSIGNMENT OR DECLARATION
        break;
    case 47: // FOR CONDITION
        break;
    case 48: // FOR INCREMENT
        break;
    case 49: // BREAK
        break;
    case 50: // CONTINUE
        break;
    case 51: // ARRAY VALUE
    {
        // TODO validate exp type
        this->arrayLengthsStack.top()++;
        break;
    }
    case 52: // ARRAY DEPTH IN
    {
        this->arrayDepth++;

        if (this->arrayDepth >= this->currentSymbol->arraySize.size())
        {
            throw SemanticError("Array length exceeded");
        }

        this->arrayLengthsStack.push(0);
        break;
    }
    case 53: // ARRAY DEPTH OUT
    {
        int arraySize = this->currentSymbol->arraySize[this->arrayDepth];
        if (this->arrayLengthsStack.top() > arraySize && arraySize != -1)
            throw SemanticError("Array length exceeded");

        this->valueArraySizes[this->arrayDepth] = this->arrayLengthsStack.top();

        this->arrayDepth--;
        this->arrayLengthsStack.pop();
        break;
    }
    case 54: // ARRAY ACCESS
    {
        // Push array size dimension
        // this->currentArrayDimension++;
        // SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(this->pendingId);
        // if (isNumber(lexeme, false))
        // {
        //     int value = stoi(lexeme);

        //     this->validateExistingSymbol(symbol);
        //     this->validateSymbolClassification(symbol, SymbolTable::ARRAY);

        //     int dimensions = symbol->arraySize.size();
        //     if (this->currentArrayDimension >= dimensions || value >= symbol->arraySize[this->currentArrayDimension])
        //     {
        //         throw SemanticError("Array index out of bounds");
        //     }
        // }
        // else if (!this->validateExpressionType(SemanticTable::INT))
        // {
        //     throw SemanticError("Invalid array index");
        // }

        // symbol->isUsed = true;
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