#include "SemanticTable.hpp"

const int SemanticTable::binaryExpTable[6][6][9] = {
    // * Operation labels:
    // * SUM, SUBTRACTION, MULTIPLICATION, DIVISION, REMAINDER, LOGICAL, RELATION_LOW, RELATION_HIGH, BITWISE
    // INT
    {
        {INT, INT, INT, INT, INT, ERR, BOOL, BOOL, INT},             // INT
        {FLOAT, FLOAT, FLOAT, FLOAT, ERR, ERR, BOOL, BOOL, ERR},     // FLOAT
        {DOUBLE, DOUBLE, DOUBLE, DOUBLE, ERR, ERR, BOOL, BOOL, ERR}, // DOUBLE
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},               // CHAR
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},               // STRING
        {ERR, ERR, ERR, ERR, ERR, ERR, BOOL, BOOL, ERR}},            // BOOL

    // FLOAT
    {
        {FLOAT, FLOAT, FLOAT, FLOAT, ERR, ERR, BOOL, BOOL, ERR},
        {FLOAT, FLOAT, FLOAT, FLOAT, ERR, ERR, BOOL, BOOL, ERR},
        {DOUBLE, DOUBLE, DOUBLE, DOUBLE, ERR, ERR, BOOL, BOOL, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}},
    // DOUBLE
    {
        {DOUBLE, DOUBLE, DOUBLE, DOUBLE, ERR, ERR, BOOL, BOOL, ERR},
        {DOUBLE, DOUBLE, DOUBLE, DOUBLE, ERR, ERR, BOOL, BOOL, ERR},
        {DOUBLE, DOUBLE, DOUBLE, DOUBLE, ERR, ERR, BOOL, BOOL, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}},
    // CHAR
    {
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {STRING, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}, // char + char = string
        {STRING, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}, // char + string = string
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}},
    // STRING
    {
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {STRING, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {STRING, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR}},
    // BOOL
    {
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR},
        {ERR, ERR, ERR, ERR, ERR, BOOL, ERR, ERR, ERR}}};

const int SemanticTable::unaryExpTable[6][4] = {
    // * Operation labels:
    // * NEG, INC/DEC, NOT, BITWISE_NOT
    {INT, INT, ERR, INT},       // INT
    {FLOAT, FLOAT, ERR, ERR},   // FLOAT
    {DOUBLE, DOUBLE, ERR, ERR}, // DOUBLE
    {ERR, ERR, ERR, ERR},       // CHAR
    {ERR, ERR, ERR, ERR},       // STRING
    {ERR, ERR, BOOL, ERR}       // BOOL
};

const int SemanticTable::atribTable[6][6] = {
    // * Atribuition labels:
    // * INT, FLOAT, DOUBLE, CHAR, STRING, BOOL
    {OK, WAR, WAR, ERR, ERR, ERR}, // INT
    {OK, OK, WAR, ERR, ERR, ERR},  // FLOAT
    {OK, OK, OK, ERR, ERR, ERR},   // DOUBLE
    {ERR, ERR, ERR, OK, ERR, ERR}, // CHAR
    {ERR, ERR, ERR, OK, OK, ERR},  // STRING
    {ERR, ERR, ERR, ERR, ERR, OK}  // BOOL
};
