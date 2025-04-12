#ifndef Syntactic_H
#define Syntactic_H

#include "Constants.h"
#include "Token.h"
#include "Lexical.h"
#include "Semantic.h"
#include "SyntacticError.h"

#include <stack>

class Syntactic
{
public:
    Syntactic() : previousToken(0), currentToken(0) { }

    ~Syntactic()
    {
        if (previousToken != 0 && previousToken != currentToken) delete previousToken;
        if (currentToken != 0)  delete currentToken;
    }

    void parse(Lexical *scanner, Semantic *semanticAnalyser);

private:
    std::stack<int> stack;
    Token *previousToken;
    Token *currentToken;
    Lexical *scanner;
    Semantic *semanticAnalyser;

    bool step();
};

#endif
