#ifndef SEMANTIC_TABLE_H
#define SEMANTIC_TABLE_H

class SemanticTable
{
private:
    // stack<int> scope;

public:
    enum Types
    {
        __NULL = -1,
        INT = 0,
        FLOAT = 1,
        DOUBLE = 2,
        CHAR = 3,
        STRING = 4,
        BOOL = 5,
    };
    enum OperationsBinary
    {
        SUM = 0,
        SUBTRACTION = 1,
        MULTIPLICATION = 2,
        DIVISION = 3,
        REMAINDER = 4,
        LOGICAL = 5,
        RELATION_LOW = 6,
        RELATION_HIGH = 7,
        BITWISE = 8
    };
    enum OperationsUnary
    {
        NEG = 0,
        INCREMENT = 1, // Increment and decrement
        NOT = 2,
        BITWISE_NOT = 3
    };
    enum Status
    {
        ERR = -1,
        WAR = 0,
        OK = 1
    };

    // Return type of binary expressions
    // 6 x 6 x 9 = Type X Type X Operation
    static int const binaryExpTable[6][6][9];

    // Return type of unary expressions
    // 6 x 5 = Type X Operation
    static int const unaryExpTable[6][4];

    // Compatible atribuitions
    // 6 x 6 = Type X Type
    static int const atribTable[6][6];

    static int unaryResultType(int TP1, int OP)
    {
        if (TP1 < 0 || OP < 0)
        {
            return ERR;
        }
        return unaryExpTable[TP1][OP];
    }

    static int resultBinaryType(int TP1, int TP2, int OP)
    {
        if (TP1 < 0 || TP2 < 0 || OP < 0)
        {
            return ERR;
        }
        return binaryExpTable[TP1][TP2][OP];
    }

    static int atribType(int TP1, int TP2)
    {
        if (TP1 < 0 || TP2 < 0)
        {
            return ERR;
        }
        return atribTable[TP1][TP2];
    }

    // SemanticTable();
};

#endif // SEMANTIC_TABLE_H
