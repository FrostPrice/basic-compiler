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
        this->pendingId = lexeme;

        break;
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

        break;
    }
    case 3: // ARRAY ACCESS
    {
        // Push array size dimension
        this->currentArrayDimension++;
        SymbolTable::SymbolInfo *symbol = this->symbolTable.getSymbol(this->pendingId);
        if (isNumber(lexeme, false))
        {
            int value = stoi(lexeme);

            this->validateExistingSymbol(symbol);
            this->validateSymbolClassification(symbol, SymbolTable::ARRAY);

            int dimensions = symbol->arraySize.size();
            if (this->currentArrayDimension >= dimensions || value >= symbol->arraySize[this->currentArrayDimension])
            {
                throw SemanticError("Array index out of bounds");
            }
        }
        else if (!this->validateExpressionType(SemanticTable::INT))
        {
            throw SemanticError("Invalid array index");
        }

        symbol->isUsed = true;
        break;
    }
    case 4:
    { // ASSIGNMENT VALUE
        // Assignment to a variable
        SymbolTable::SymbolInfo *matchedSymbol = this->symbolTable.getSymbol(this->pendingId);

        if (matchedSymbol == nullptr)
        {
            SymbolTable::SymbolInfo newSymbol;
            newSymbol.id = this->pendingId;
            newSymbol.dataType = this->pendingType;
            newSymbol.scope = this->symbolTable.currentScope;
            newSymbol.isInitialized = true; // Mark as initialized
            newSymbol.symbolClassification = pendingClassification;

            this->symbolTable.addSymbol(newSymbol);
        }
        else
        {
            // Check if the symbol is already declared in the current scope
            validateDuplicateSymbolInSameScope(matchedSymbol);

            // Check type compatibility
            validateVariableType(matchedSymbol);

            matchedSymbol->isInitialized = true; // Mark as initialized
        }

        break;
    }
    case 5: // ASSIGNMENT ARRAY VALUE
    {

        break;
    }
    case 7: // ARRAY VALUE
    {
        this->valueArrayLength++;
        break;
    }
    case 8: // ARRAY DIMENSIONS
        // Finalize array dimension info
        break;
    case 9: // ARRAY SIZE DECLARATION
        // Array size declaration
        break;
    case 10: // ARRAY DEPTH
    {
        this->valueArrayLength++;
        vector<int>::iterator it = this->valueArrayDimensions.begin();
        this->valueArrayDimensions.insert(it, this->valueArrayLength);
        this->valueArrayLength = 0;
        break;
    }
    // * 11-20: Operators *
    case 11: // ASSIGN OP

        break;
    case 12: // ARITHMETICAL ASSIGN OP
        // Handle +=, -=, *=, etc. for numbers
        break;
    case 13: // ADD ASSIGN OP
        // Handle += for strings or numbers
        break;
    case 14: // REMAINDER ASSIGN OP
        // Handle %= for integers
        break;
    case 15: // NUMBER OP
        // Handle +, -, *, /, etc.
        break;
    case 16: // ADD OP
        // Handle + for numbers or strings
        break;
    case 17: // INTEGER OP
        // Handle bitwise ops or % (only integers)
        break;
    case 18: // BOOLEAN OP
        // Handle &&, ||, !
        break;
    case 19: // EQ NE OP
        // == or != for numbers or strings
        break;

    // * 21-30: Functions, blocks, I/O *
    case 21:
    { // FUNCTION_DEF_PARAMETER
        // Save function parameter type/name
        SymbolTable::SymbolInfo *currentSymbol = this->symbolTable.getSymbol(this->pendingId);
        currentSymbol->symbolClassification = SymbolTable::PARAM;

        break;
    }
    case 22:
    { // FUNCTION_CALL_PARAMETER
        // Push function call parameter
        SymbolTable::SymbolInfo *functionSymbol = this->symbolTable.getSymbol(this->pendingId);

        if (functionSymbol == nullptr)
            throw SemanticError("Function '" + this->pendingId + "' not found");

        if (functionSymbol->symbolClassification != SymbolTable::FUNCTION)
            throw SemanticError("Symbol '" + this->pendingId + "' is not a function");

        SemanticTable::Types expectedType = functionSymbol->dataType;

        validateExpressionType(expectedType);

        break;
    }
    case 23:
        // BLOCK_INIT
        // Enter a new scope
        this->symbolTable.enterScope();
        cout << "Entered new scope" << this->symbolTable.currentScope << endl;
        break;

    case 24:
        // BLOCK_END
        // Exit current scope
        if (symbolTable.currentScope > 0)
        {
            this->symbolTable.exitScope();
            cout << "Exited scope" << this->symbolTable.currentScope << endl;
        }

        validateExitScope(this->symbolTable.exitScope());
        break;

    case 25: // RETURN
        // Handle return type checking
        // SymbolTable::SymbolInfo *currentFunction = this->symbolTable.getCurrentFunction();

        // Validar se estamos dentro de uma função
        // if (currentFunction == nullptr)
        // {
        //     throw SemanticError("Return statement outside of a function");
        // }

        // // Obter o tipo de retorno esperado da função
        // SemanticTable::Types expectedReturnType = currentFunction->dataType;

        // // Validar o tipo da expressão retornada
        // if (!this->validateExpressionType(expectedReturnType))
        // {
        //     throw SemanticError("Type mismatch in return statement: expected '" +
        //                         to_string(expectedReturnType) + "'");
        // }
        break;
    case 26: // INPUT
        // Read value from user
        break;
    case 27: // OUTPUT
        // Print value
        break;

    // * 31-40: Primitive values *
    case 31: // INT VALUE
        // Push int value to stack
        this->symbolTable.expressionStack.push(SemanticTable::INT);
        break;
    case 32:
    { // DECIMAL VALUE
        // Push float/double value to stack
        if (lexeme[lexeme.length() - 1] == 'f')
            this->symbolTable.expressionStack.push(SemanticTable::FLOAT);
        else
            this->symbolTable.expressionStack.push(SemanticTable::DOUBLE);

        break;
    }
    case 33: // CHAR VALUE
        // Push char value
        this->symbolTable.expressionStack.push(SemanticTable::CHAR);
        break;
    case 34: // STRING VALUE
        // Push string literal
        this->symbolTable.expressionStack.push(SemanticTable::STRING);
        break;
    case 35: // BOOLEAN VALUE
        // Push boolean literal
        this->symbolTable.expressionStack.push(SemanticTable::BOOL);
        break;

    // * 41-50: Conditionals and loops *
    case 41: // IF CONDITION
        // Validate type of condition
        break;
    case 42: // SWITCH EXPRESSION
        // Store switch expression type
        break;
    case 43: // CASE VALUE
        // Match case value type
        break;
    case 44: // WHILE CONDITION
        // Type check condition
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
        // Optional: validate context (inside loop/switch)
        break;
    case 50: // CONTINUE
        break;

    default:
        cout << "Ação não reconhecida: " << action << endl;
        break;
    }
}

void Semantic::reset()
{
    this->pendingType = SemanticTable::__NULL;
    this->pendingId.clear();
    this->symbolArrayDimensions.clear();

    while (!this->operatorStack.empty())
        this->operatorStack.pop();

    while (!this->idTypeStack.empty())
        this->operatorStack.pop();
}

bool Semantic::validateExpressionType(SemanticTable::Types expectedType)
{
    if (this->symbolTable.expressionStack.empty())
    {
        throw SemanticError("Expression stack is empty");
    }

    int actualType = this->symbolTable.expressionStack.top();
    this->symbolTable.expressionStack.pop();

    if (actualType != expectedType)
    {
        throw SemanticError("Type mismatch: expected '" + to_string(expectedType) + "'  got '" + to_string(actualType) + "'");
    }
}

void Semantic::validateExitScope(bool isValid)
{
    if (!isValid)
    {
        throw SemanticError("Cannot exit global scope");
    }
}

void Semantic::validateExistingSymbol(SymbolTable::SymbolInfo *symbol)
{
    if (symbol == nullptr)
    {
        throw SemanticError("Symbol '" + this->pendingId + "' not found");
    }
}

void Semantic::validateSymbolClassification(SymbolTable::SymbolInfo *symbol, SymbolTable::SymbolClassification classification)
{
    if (symbol->symbolClassification != classification)
    {
        throw SemanticError("Symbol '" + this->pendingId + "' is not of the expected classification");
    }
}

void Semantic::validateDuplicateSymbolInSameScope(SymbolTable::SymbolInfo *symbol)
{

    if (symbol->scope == this->symbolTable.currentScope)
    {
        throw SemanticError("Symbol '" + this->pendingId + "' already exists");
    }
}

void Semantic::validateVariableType(SymbolTable::SymbolInfo *matchedSymbol)
{
    if (matchedSymbol->dataType != this->pendingType)
    {
        throw SemanticError("Type mismatch: cannot assign '" + this->pendingId + "' to variable '" + this->pendingId + "' of type '" + to_string(matchedSymbol->dataType) + "'");
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