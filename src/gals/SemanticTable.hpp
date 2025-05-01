#ifndef SEMANTIC_TABLE_H
#define SEMANTIC_TABLE_H

class SemanticTable
{
private:
public:
    enum Types
    {
        INT = 0,
        FLOAT = 1,
        DOUBLE = 2,
        CHAR = 3,
        STRING = 4,
        BOOL = 5
    };
    enum Operations
    {
        SUM = 0,
        SUBTRACTION = 1,
        MULTIPLICATION = 2,
        DIVISION = 3,
        REMAINDER = 4,
        LOGICAL = 5,
        NEG = 6,
        RELATION_LOW = 7,
        RELATION_HIGH = 8,
        NOT = 9,
        BITWISE = 10
    };
    enum Status
    {
        ERR = -1,
        WAR = 0,
        OK = 1
    };

    // TIPO DE RETORNO DAS EXPRESSOES ENTRE TIPOS
    // 5 x 5 X 10  = TIPO X TIPO X OPER
    static int const expTable[6][6][11];

    // atribuicoes compativeis
    // 5 x 5 = TIPO X TIPO
    static int const atribTable[6][6];

    static int resultType(int TP1, int TP2, int OP)
    {
        if (TP1 < 0 || TP2 < 0 || OP < 0)
        {
            return ERR;
        }
        return (expTable[TP1][TP2][OP]);
    }

    static int atribType(int TP1, int TP2)
    {
        if (TP1 < 0 || TP2 < 0)
        {
            return ERR;
        }
        return (atribTable[TP1][TP2]);
    }

    //    SemanticTable();
};

int const SemanticTable::expTable[6][6][11] =
    {/*                INT                  */ /*                 FLOAT               */ /*                 CHAR                */ /*               STRING                */ /*                 BOOL                */
                                                                                                                                                                             /*     SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_     */
     /*INT   */ {{INT, INT, INT, FLOAT, BOOL, INT, INT, FLOAT, ERR, ERR}, {FLOAT, FLOAT, FLOAT, FLOAT, BOOL, ERR, FLOAT, FLOAT, ERR, ERR}, {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}, {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}, {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}},
     /*FLOAT */ {{
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 }},
     /*CHAR  */ {{
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 }},
     /*STRING*/ {{
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 }},
     /*BOOL  */ {{
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 },
                 {
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                     ,
                 }}};

int const SemanticTable::atribTable[5][5] =
    {/* INT FLO CHA STR BOO  */
     /*INT*/ {OK, WAR, ERR, ERR, ERR},
     /*FLO*/ {
         ,
         ,
         ,
         ,
     },
     /*CHA*/ {
         ,
         ,
         ,
         ,
     },
     /*STR*/ {
         ,
         ,
         ,
         ,
     },
     /*BOO*/ {
         ,
         ,
         ,
         ,
     }};
}
;

#endif // SEMANTIC_TABLE_H

SemanticTable::SemanticTable(/* args */)
{
}

SemanticTable::~SemanticTable()
{
}
