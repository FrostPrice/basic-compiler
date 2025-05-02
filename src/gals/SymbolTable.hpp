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
    stack<int> expressionStack; // Expression to manage scopes

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
        SemanticTable::Types dataType; // 0: INT, 1: FLOAT, 2: DOUBLE, 3: CHAR, 4: STRING, 5: BOOL (From SemanticTable)
        int scope;                     // 0: global, n: local
        SymbolClassification symbolClassification;
        bool isInitialized = false; // true if the variable is initialized (has value)
        bool isUsed = false;        // true if the variable is used
        vector<int> arraySize;      // Array size of each dimension

        SymbolInfo() {}
        SymbolInfo(string id, SemanticTable::Types dataType, int scope) : id(id), dataType(dataType), scope(scope) {}
    };
    int currentScope = 0; // Current scope level

    // Private variables
private:
    vector<SymbolInfo> symbolTable; // Vector to store all symbols
    stack<int> scopeStack;          // Stack to manage scopes

    // Public methods

public:
    SymbolTable()
    {
        scopeStack.push(0);
    };

    void enterScope()
    {
        scopeStack.push(currentScope);
        currentScope++;
    };

    bool exitScope()
    {
        if (scopeStack.size() > 1)
        {
            scopeStack.pop();
            return true; // exited scope
        }
        return false;
    }

    bool addSymbol(SymbolInfo &newSymbol)
    {
        // Check if the symbol already exists in the current scope
        for (SymbolInfo &symbol : symbolTable)
        {
            if (symbol.id == newSymbol.id && symbol.scope == currentScope)
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
            if (symbol.id == id && isInValidScope(symbol.scope))
            {
                cout << "Found symbol: " << symbol.id << endl;
                return &symbol; // Return the symbol if found in the current scope
            }
        }
        return nullptr; // Symbol not found
    }

    vector<SymbolInfo> getAllSymbols()
    {
        return symbolTable;
    }

    // * Used only for debugging
    void printTable()
    {
        cout << "\n----- Symbol Table -----\n";
        for (SymbolInfo symbol : symbolTable)
        {
            cout << "ID: " << symbol.id
                 << ", Data Type: " << symbol.dataType
                 << ", Scope: " << symbol.scope
                 << ", Symbol Classification: " << symbol.symbolClassification
                 << ", Is Initialized: " << boolalpha << symbol.isInitialized
                 << ", Is Used: " << boolalpha << symbol.isUsed
                 << ", Array Size: ";

            for (int &size : symbol.arraySize)
            {
                cout << size << " ";
            }
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
};

#endif // SYMBOL_TABLE_H