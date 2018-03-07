#include "jit.h"
#include "debug.h"
#include "parser.h"

#include <assert.h>

int pia_initialize_jit(pia_jit *jit) {
	jit->parser = pia_create_parser();
	jit->ctx = gcc_jit_context_acquire();
	jit->func_map = NULL;
	return jit->parser && jit->ctx;
}

void pia_destroy_jit(pia_jit *jit) {
	pia_destroy_parser(jit->parser);
	gcc_jit_context_release(jit->ctx);
	Word_t w;
	JSLFA(w, jit->func_map);
}

static int create_functions(pia_jit *jit, pia_parsed_function **functions) {
	pia_parsed_function **funcp;
	pia_parsed_function *func;
	gcc_jit_function **ptr;
	for(funcp = functions; *funcp; funcp++) {
		func = *funcp;
		JSLI(ptr, jit->func_map, (uint8_t *) func->name);
		if(*ptr != NULL) {
			FERROR("create_functions", "Function \"%s\" is already defined", func->name);
			return 0;
		}
		gcc_jit_param *params[] = {
			gcc_jit_context_new_param(jit->ctx, NULL, jit->doublep_type, "v"),
			gcc_jit_context_new_param(jit->ctx, NULL, jit->intp_type, "s"),
		};
		*ptr = gcc_jit_context_new_function(jit->ctx,
				gcc_jit_context_new_location(jit->ctx, jit->filename, func->line_on_file, 0),
				GCC_JIT_FUNCTION_EXPORTED,
				jit->void_type,
				func->name,
				2, params,
				0);
	}
	return 1;
}

static int populate(pia_jit *jit, gcc_jit_function *jitf, pia_parsed_function *definition) {
	gcc_jit_block *entry = gcc_jit_function_new_block(jitf, "entry");
	int i;
	for(i = 0; i < definition->instr_count; i++) {
		if(!pia_instr_populate(jit, jitf, entry, definition->instructions[i])) return 0;
	}
	gcc_jit_block_end_with_void_return(entry, NULL);
	return 1;
}

static int populate_functions(pia_jit *jit, pia_parsed_function **functions) {
	pia_parsed_function **funcp;
	pia_parsed_function *func;
	gcc_jit_function **ptr;
	gcc_jit_function *jitf;
	for(funcp = functions; *funcp; funcp++) {
		func = *funcp;
		JSLG(ptr, jit->func_map, (uint8_t *) func->name);
		jitf = *ptr;
		assert(jitf && "[jit.c::populate_functions] Funções deveriam já ter sido todas criadas");
		if(!populate(jit, jitf, func)) return 0;
	}
	return 1;
}

static int setup_functions(pia_jit *jit, pia_parsed_function **functions) {
	gcc_jit_context *ctx = jit->ctx;

	jit->void_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_VOID);
	jit->int_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_INT);
	jit->intp_type = gcc_jit_type_get_pointer(jit->int_type);
	jit->double_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_DOUBLE);
	jit->doublep_type = gcc_jit_type_get_pointer(jit->double_type);
	jit->int_one = gcc_jit_context_one(ctx, jit->int_type);
	jit->printf = gcc_jit_context_get_builtin_function(ctx, "printf");

	if(!create_functions(jit, functions)) return 0;
	if(!populate_functions(jit, functions)) return 0;
	return 1;
}

static void setup_jit_options(pia_jit *jit) {
	gcc_jit_context_set_bool_option(jit->ctx, GCC_JIT_BOOL_OPTION_DUMP_INITIAL_GIMPLE, 1);
	gcc_jit_context_set_int_option(jit->ctx,  GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 0);
}

int pia_run_file(pia_jit *jit, const char *filename) {
	jit->filename = filename;
	pia_parsed_function **functions = pia_parse_file(jit->parser, filename);
	if(!functions) return 1;

	if(!setup_functions(jit, functions)) return 2;
	setup_jit_options(jit);
	gcc_jit_result *compiled = gcc_jit_context_compile(jit->ctx);

	pia_function_type _main = gcc_jit_result_get_code(compiled, PIA_MAIN_FUNCTION);
	double v[STACK_MAX];
	int s = 0;
	_main(v, &s);
	
	pia_free_parsed_functions(functions);
	gcc_jit_result_release(compiled);
	return 0;
}
