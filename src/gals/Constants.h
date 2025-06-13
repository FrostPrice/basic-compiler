#ifndef CONSTANTS_H
#define CONSTANTS_H

enum TokenId 
{
    EPSILON  = 0,
    DOLLAR   = 1,
    t_SINGLE_LINE_COMMENT = 2,
    t_MULTI_LINE_COMMENT = 3,
    t_ID = 4,
    t_INT = 5,
    t_FLOAT = 6,
    t_DOUBLE = 7,
    t_CHAR = 8,
    t_STRING = 9,
    t_BOOLEAN = 10,
    t_VOID = 11,
    t_IF = 12,
    t_ELSE = 13,
    t_WHILE = 14,
    t_DO = 15,
    t_FOR = 16,
    t_SWITCH = 17,
    t_CASE = 18,
    t_RETURN = 19,
    t_CONTINUE = 20,
    t_BREAK = 21,
    t_DEFAULT = 22,
    t_INPUT = 23,
    t_OUTPUT = 24,
    t_ASSIGNMENT = 25,
    t_ASSIGNMENT_ADD = 26,
    t_ASSIGNMENT_SUB = 27,
    t_ASSIGNMENT_MULT = 28,
    t_ASSIGNMENT_DIVIDE = 29,
    t_ASSIGNMENT_REMAIN = 30,
    t_INCREMENT = 31,
    t_DECREMENT = 32,
    t_PLUS = 33,
    t_MINUS = 34,
    t_MULTIPLY = 35,
    t_DIVIDE = 36,
    t_REMAINDER = 37,
    t_AND = 38,
    t_OR = 39,
    t_NOT = 40,
    t_BIT_AND = 41,
    t_BIT_OR = 42,
    t_BIT_XOR = 43,
    t_BIT_NOT = 44,
    t_BIT_LSHIFT = 45,
    t_BIT_RSHIFT = 46,
    t_GT = 47,
    t_LT = 48,
    t_GE = 49,
    t_LE = 50,
    t_EQ = 51,
    t_NE = 52,
    t_VAL_BINARY = 53,
    t_VAL_HEXADECIMAL = 54,
    t_VAL_INTEGER = 55,
    t_VAL_FLOAT = 56,
    t_VAL_DOUBLE = 57,
    t_VAL_CHAR = 58,
    t_VAL_STRING = 59,
    t_VAL_TRUE = 60,
    t_VAL_FALSE = 61,
    t_TOKEN_62 = 62, //"("
    t_TOKEN_63 = 63, //")"
    t_TOKEN_64 = 64, //"["
    t_TOKEN_65 = 65, //"]"
    t_TOKEN_66 = 66, //"{"
    t_TOKEN_67 = 67, //"}"
    t_TOKEN_68 = 68, //","
    t_TOKEN_69 = 69, //";"
    t_TOKEN_70 = 70, //":"
    t_TOKEN_71 = 71, //".
};

const int STATES_COUNT = 60;

extern int SCANNER_TABLE[STATES_COUNT][256];

extern int TOKEN_STATE[STATES_COUNT];

extern int SPECIAL_CASES_INDEXES[73];

extern const char *SPECIAL_CASES_KEYS[22];

extern int SPECIAL_CASES_VALUES[22];

extern const char *SCANNER_ERROR[STATES_COUNT];

const int FIRST_SEMANTIC_ACTION = 136;

const int SHIFT  = 0;
const int REDUCE = 1;
const int ACTION = 2;
const int ACCEPT = 3;
const int GO_TO  = 4;
const int ERROR  = 5;

extern const int PARSER_TABLE[358][203][2];

extern const int PRODUCTIONS[151][2];

extern const char *PARSER_ERROR[358];

#endif
