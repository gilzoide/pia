#ifndef __PIA_JIT_H__
#define __PIA_JIT_H__

#include <libgccjit.h>
#include <Judy.h>

#include "parser.h"

typedef struct {
	pia_parser *parser;           // The parser
	gcc_jit_context *ctx;         // JIT context
	gcc_jit_struct *pilha_struct; // The stack struct
	gcc_jit_type *pilha_type;     // The stack struct type
	gcc_jit_type *void_type;      // The void type, for returns
	gcc_jit_param *pilha_param;   // The stack parameter
	Pvoid_t func_map;             // 'Name -> Function' map
} pia_jit;

int pia_initialize_jit(pia_jit *jit);
void pia_destroy_jit(pia_jit *jit);
int pia_run_file(pia_jit *jit, const char *filename);

#endif

