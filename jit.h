#ifndef __PIA_JIT_H__
#define __PIA_JIT_H__

#include <jit/jit.h>
#include <Judy.h>

#include "parser.h"

typedef struct pia_jit {
	pia_parser *parser;         // The parser
	jit_context_t ctx;          // JIT context

	const char *filename;       // The current filename

	jit_type_t _intp;
	jit_type_t _doublep;
	jit_type_t _charp;
	jit_type_t func_signature;  // Compiled functions signature: void(double *, int *)
	jit_type_t printf_s_signature;
	jit_type_t printf_d_signature;

	jit_value_t v;
	jit_value_t s;

	Pvoid_t func_map;           // 'Name -> Function' map
} pia_jit;

int pia_initialize_jit(pia_jit *jit);
void pia_destroy_jit(pia_jit *jit);
int pia_run_file(pia_jit *jit, const char *filename);
jit_function_t pia_find_function(pia_jit *jit, const char *name);

#define PIA_MAIN_FUNCTION "_main"
#define STACK_MAX 128

#endif

