#include "Semantic.h"
#include "Constants.h"

#include <iostream>

void Semantic::executeAction(int action, const Token *token)
{
    std::cout << "Ação: " << action << ", Token: " << token->getId()
              << ", Lexema: " << token->getLexeme() << std::endl;
}

// Criar pilha de escopo, usar contador numerico para controlar qual o escopo atual

// Ao encontrar um ID, adicionar esse simbolo na tabela de simbolos
// Ao encontrar um ID, verificar se ele já existe na tabela de simbolos

// Criar pilha de expressões
// Checar o operador da esquerda e da direita, e validar se eles são do mesmo tipo
// Ao encontrar um operando, coloca o tipo dele na pilha de expressões
// AO encontrar um operador, coloca ele na pilha de expressões

// Criar funcao auxiliar para verificar se o tipo é valido entre dois operadores/variaveis
// Permitir concatenacao entre strings e chars

// id = string
// type = enum
// scope = int
// was_used = bool
// was_initialized = bool
// array_dimensions = int
// array_size = int