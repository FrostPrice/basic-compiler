#include "semantic_table.h"

// Definindo as variáveis externas
int const SemanticTable::expTable[5][5][10] =
    {/*                INT                  */ /*                 FLOAT               */ /*                 CHAR                */ /*               STRING                */ /*                 BOOL                */
    /*     SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_ , SUM,SUB,MUL,DIV,REL,MOD,POT,SQT,AND,OR_     */
    /*INT   */  {{INT,INT,INT,FLO,BOO,INT,INT,FLO,ERR,ERR},{FLO,FLO,FLO,FLO,BOO,ERR,FLO,FLO,ERR,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR}},
    };

int const SemanticTable::atribTable[5][5] =
    {     /* INT FLO CHA STR BOO  */
    /*INT*/ {OK_, WAR, ERR, ERR, ERR},
    // Outros tipos podem ser preenchidos conforme necessário
    };

