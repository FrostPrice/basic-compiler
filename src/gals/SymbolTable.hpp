#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stack>

#include "SemanticTable.hpp"

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
        bool isInitialized = false; // true if the variable is initialized (has value)
        bool isUsed = false;        // true if the variable is used
        vector<int> arraySize;      // Array size of each dimension
        int functionParams = -1;    // Number of parameters in the function
        string functionId;          // ID of the function

        SymbolInfo() {}
        SymbolInfo(string id, SemanticTable::Types dataType, int scope) : id(id), dataType(dataType), scope(scope) {}
    };

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

    SymbolTable::SymbolInfo *addSymbol(SymbolInfo &newSymbol)
    {
        // Check if the symbol already exists in the current scope
        for (SymbolInfo &symbol : symbolTable)
        {
            if (symbol.id == newSymbol.id && symbol.scope == newSymbol.scope)
            {
                return nullptr; // Symbol already exists in this scope
            }
        }

        symbolTable.push_back(newSymbol);
        return &symbolTable.back(); // Symbol added successfully
    }

    SymbolInfo *getSymbol(string id)
    {
        for (auto it = symbolTable.rbegin(); it != symbolTable.rend(); ++it)
        {
            SymbolInfo &symbol = *it;
            if (it->id == id && isInValidScope(it->scope))
            {
                return &symbol; // Return the symbol if found in the current scope
            }
        }
        return nullptr; // Symbol not found
    }

    SymbolInfo *getSymbolInScope(const std::string &id, int currentScope)
    {
        for (SymbolInfo &symbol : symbolTable)
        {
            if (symbol.id == id && symbol.scope == currentScope)
            {
                return &symbol;
            }
        }
        return nullptr;
    }

    vector<SymbolInfo> getAllSymbols()
    {
        return symbolTable;
    }

    SymbolInfo *getEnclosingFunction(int scope)
    {
        for (SymbolInfo &symbol : symbolTable)
        {
            // Will get the function that is enclosing the current scope
            if (symbol.symbolClassification == FUNCTION && scope > symbol.scope)
            {
                return &symbol; // Return the symbol if found in the current scope
            }
        }
        return nullptr; // Symbol not found
    };

    SymbolInfo *getFunctionInScope()
    {
        for (auto it = symbolTable.rbegin(); it != symbolTable.rend(); ++it)
        {
            SymbolInfo &symbol = *it;
            if (symbol.symbolClassification == FUNCTION && isInValidScope(symbol.scope))
            {
                return &symbol; // Return the symbol if found in the current scope
            }
        }
        return nullptr; // Symbol not found
    };

    vector<SymbolInfo *> getFunctionParams(int scope)
    {
        vector<SymbolInfo *> params;

        for (auto it = symbolTable.rbegin(); it != symbolTable.rend(); ++it)
        {
            SymbolInfo &symbol = *it;
            if (symbol.symbolClassification == PARAM && symbol.scope == scope)
            {
                params.push_back(&symbol);
            }
        }
        return params;
    }

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