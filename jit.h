#ifndef __PIA_JIT_H__
#define __PIA_JIT_H__

#include <libgccjit.h>
#include <Judy.h>

#include "parser.h"

typedef struct pia_jit {
	pia_parser *parser;         // The parser
	gcc_jit_context *ctx;       // JIT context

	const char *filename;       // The current filename

	gcc_jit_type *int_type;     // 'int' type
	gcc_jit_type *intp_type;    // 'int *' type
	gcc_jit_type *double_type;  // 'double' type
	gcc_jit_type *doublep_type; // 'double *' type
	gcc_jit_type *void_type;    // 'void' return type
	gcc_jit_rvalue *int_one;    // '1'

	gcc_jit_lvalue *v;          // 'double *v' stack parameter
	gcc_jit_lvalue *s;          // 'int *s' stack parameter, `v`'s current size

	gcc_jit_function *printf;   // The 'printf' function
	Pvoid_t func_map;           // 'Name -> Function' map
} pia_jit;

int pia_initialize_jit(pia_jit *jit);
void pia_destroy_jit(pia_jit *jit);
int pia_run_file(pia_jit *jit, const char *filename);

#define PIA_MAIN_FUNCTION "_main"
typedef void(*pia_function_type)(double *, int *);
#define STACK_MAX 128

#endif

