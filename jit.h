#ifndef __PIA_JIT_H__
#define __PIA_JIT_H__

#include <libgccjit.h>
#include <Judy.h>

#include "parser.h"

typedef struct {
	pt_grammar *grammar;  // Grammar, for parsing source code
	int line_count;       // Line count, for location
	int last_eol_pos;     // Position of last "\n", for location
	gcc_jit_context *ctx; // JIT context
	Pvoid_t func_map;     // 'Name -> Function' map
} pia_jit;

int pia_initialize_jit(pia_jit *jit);
void pia_destroy_jit(pia_jit *jit);

#endif

