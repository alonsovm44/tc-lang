#ifndef TIGHTC_PARSER_H
#define TIGHTC_PARSER_H

#include "ast.h"
#include "lexer.h"

DeclVec parse_program(Token *tokens);

#endif
