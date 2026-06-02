#ifndef TIGHTC_EMITTER_H
#define TIGHTC_EMITTER_H

#include "ast.h"

char *emit_program(DeclVec program);
char *emit_hot_split(DeclVec program, const char *hot_lib, char **hot_c_out);

#endif
