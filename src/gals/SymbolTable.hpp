#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stack>

using namespace std;

class SymbolTable
{
    // Public variables
public:
    enum SymbolClassification
    {
        NONE = -1,
        VARIABLE,
        ARRAY,
        FUNCTION,
        PARAM,
    };

    struct SymbolInfo
    {
        string id;
        SemanticTable::Types dataType; // -1: __NULL, 0: INT, 1: FLOAT, 2: DOUBLE, 3: CHAR, 4: STRING, 5: BOOL (From SemanticTable)
        int scope;                     // 0: global, n: local
        SymbolClassification symbolClassification;
        bool isDeclared = false;    // true if the variable is declared
        bool isInitialized = false; // true if the variable is initialized (has value)
        bool isUsed = false;        // true if the variable is used
        vector<int> arraySize;      // Array size of each dimension

        SymbolInfo() {}
        SymbolInfo(string id, SemanticTable::Types dataType, int scope) : id(id), dataType(dataType), scope(scope) {}
    };

    struct ExpressionsEntry
    {
        SemanticTable::Types entryType = SemanticTable::Types::__NULL; // Type of the entry
        SemanticTable::OperationsBinary binaryOperation;               // Binary operation
        SemanticTable::OperationsUnary unaryOperation;                 // Unary operation
        string value;
    };
    stack<ExpressionsEntry> expressionStack; // Stack to manage expressions

    // Private variables
private:
    int currentScope = 0;           // Current scope level for the symbol table
    vector<SymbolInfo> symbolTable; // Vector to store all symbols
    stack<int> scopeStack;          // Stack to manage scopes

    // Public methods
public:
    SymbolTable()
    {
        scopeStack.push(0);
    };

    // Getters and Setters
    int getCurrentScope()
    {
        return scopeStack.top();
    };

    // Expression stack
    // Pushes a type onto the expression stack
    void pushType(SemanticTable::Types type, const string &value)
    {
        ExpressionsEntry entry;
        entry.entryType = type;
        entry.value = value;
        expressionStack.push(entry);
    }

    // Pushes a pending binary operator onto the expression stack
    void pushBinaryOp(SemanticTable::OperationsBinary op)
    {
        ExpressionsEntry entry;
        entry.binaryOperation = op;
        expressionStack.push(entry);
    }

    // Pushes a pending unary operator
    void pushUnaryOp(SemanticTable::OperationsUnary op)
    {
        ExpressionsEntry entry;
        entry.unaryOperation = op;
        expressionStack.push(entry);
    }

    // Scope
    int enterScope()
    {
        currentScope++;
        scopeStack.push(currentScope);
        return currentScope; // Entered new scope
    };

    int exitScope()
    {
        if (scopeStack.size() > 1)
        {
            int tempScope = scopeStack.top();
            scopeStack.pop();
            return tempScope; // exited scope
        }
        return 0; // Global scope
    }

    bool addSymbol(SymbolInfo &newSymbol)
    {
        // Check if the symbol already exists in the current scope
        for (SymbolInfo &symbol : symbolTable)
        {
            if (symbol.id == newSymbol.id && symbol.scope == newSymbol.scope)
            {
                return false; // Symbol already exists in this scope
            }
        }

        symbolTable.push_back(newSymbol);
        return true; // Symbol added successfully
    }

    SymbolInfo *getSymbol(string id)
    {
        for (SymbolInfo &symbol : symbolTable)
        {
            // TODO: Talvez deixar essa verificacao para os casos que nao seja funcao
            if (symbol.id == id && isInValidScope(symbol.scope))
            {
                return &symbol; // Return the symbol if found in the current scope
            }
        }
        return nullptr; // Symbol not found
    }

    vector<SymbolInfo> getAllSymbols()
    {
        return symbolTable;
    }

    SymbolInfo *getFunctionScope()
    {
        for (SymbolInfo &symbol : symbolTable)
        {
            if (symbol.symbolClassification == FUNCTION && isInValidScope(symbol.scope))
            {
                cout << "Found function: " << symbol.id << " in scope: " << symbol.scope << endl;
                return &symbol; // Return the symbol if found in the current scope
            }
        }
        return nullptr; // Symbol not found
    };

    // * Used only for debugging
    void printTable()
    {
        cout << "\n----- Symbol Table -----\n";
        for (SymbolInfo symbol : symbolTable)
        {
            string dataType;
            if (symbol.dataType == SemanticTable::Types::__NULL)
            {
                dataType = "NULL";
            }
            else if (symbol.dataType == SemanticTable::INT)
            {
                dataType = "INT";
            }
            else if (symbol.dataType == SemanticTable::FLOAT)
            {
                dataType = "FLOAT";
            }
            else if (symbol.dataType == SemanticTable::DOUBLE)
            {
                dataType = "DOUBLE";
            }
            else if (symbol.dataType == SemanticTable::CHAR)
            {
                dataType = "CHAR";
            }
            else if (symbol.dataType == SemanticTable::STRING)
            {
                dataType = "STRING";
            }
            else if (symbol.dataType == SemanticTable::BOOL)
            {
                dataType = "BOOL";
            }

            string symbolClassification;
            if (symbol.symbolClassification == FUNCTION)
            {
                symbolClassification = "Function";
            }
            else if (symbol.symbolClassification == VARIABLE)
            {
                symbolClassification = "Variable";
            }
            else if (symbol.symbolClassification == ARRAY)
            {
                symbolClassification = "Array";
            }
            else if (symbol.symbolClassification == PARAM)
            {
                symbolClassification = "Parameter";
            }

            cout << "ID: " << symbol.id
                 << ", Data Type: " << dataType
                 << ", Scope: " << symbol.scope
                 << ", Symbol Classification: " << symbolClassification
                 << ", Is Initialized: " << boolalpha << symbol.isInitialized
                 << ", Is Used: " << boolalpha << symbol.isUsed
                 << ", Array Size: " << this->getArraySizeString(symbol.arraySize);

            cout << "\n-------------\n";
        }
    }

    // Private methods
private:
    bool isInValidScope(int scope)
    {
        stack<int> tempStack = scopeStack;

        while (!tempStack.empty())
        {

            if (tempStack.top() == scope)
            {
                return true; // Scope is valid
            }
            tempStack.pop();
        }

        return false; // Scope is not valid
    }

    string getArraySizeString(vector<int> arraySize)
    {
        string sizeString = "";
        for (int size : arraySize)
        {
            sizeString += "[" + (size == -1 ? "exp" : to_string(size)) + "]";
        }
        return sizeString;
    }
};

#endif // SYMBOL_TABLE_H