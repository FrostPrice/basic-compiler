/*
 * Template to help verify type compatibility in a Semantic Analyzer.
 * Available to Computer Science course at UNIVALI.
 * Professor Eduardo Alves da Silva.
 */

 #ifndef SEMANTIC_TABLE_H
 #define SEMANTIC_TABLE_H
 
 class SemanticTable {
 public:
     enum Types {INT = 0, FLO, CHA, STR, BOO};
     enum Operations {SUM = 0, SUB, MUL, DIV, REL, MOD, POT, ROO, AND, OR_};
     enum Status {ERR = -1, WAR, OK_};
 
     // Declaração das variáveis como externas
     static int const expTable[5][5][10];
     static int const atribTable[5][5];
 
     static int resultType(int TP1, int TP2, int OP) {
         if (TP1 < 0 || TP2 < 0 || OP < 0) {
             return ERR;
         }
         return (expTable[TP1][TP2][OP]);
     }
 
     static int atribType(int TP1, int TP2) {
         if (TP1 < 0 || TP2 < 0) {
             return ERR;
         }
         return (atribTable[TP1][TP2]);
     }
 };
 
 #endif // SEMANTIC_TABLE_H
 