#include "jit.h"
#include "debug.h"
#include "parser.h"

int pia_initialize_jit(pia_jit *jit) {
	jit->parser = pia_create_parser();
	jit->ctx = gcc_jit_context_acquire();
	jit->func_map = NULL;
	return jit->parser && jit->ctx;
}

void pia_destroy_jit(pia_jit *jit) {
	pia_destroy_parser(jit->parser);
	gcc_jit_context_release(jit->ctx);
}

static gcc_jit_struct *pia_setup_pilha_struct(gcc_jit_context *ctx) {
	gcc_jit_field *field_s = gcc_jit_context_new_field(ctx, NULL,
			gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_INT), "s");
	gcc_jit_field *field_v = gcc_jit_context_new_field(ctx, NULL,
			gcc_jit_type_get_pointer(gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_DOUBLE)), "v");
	gcc_jit_field *fields[] = {field_v, field_s};
	return gcc_jit_context_new_struct_type(ctx, NULL, "pilha_t", 2, fields);
}

static int pia_create_functions(pia_jit *jit, pia_parsed_function **functions, const char *filename) {
	pia_parsed_function **func;
	Word_t *ptr;
	for(func = functions; *func; func++) {
		JSLI(ptr, jit->func_map, (*func)->name);
		if(ptr != NULL) {
			FERROR("pia_create_functions", "Function \"%s\" is already defined", (*func)->name);
			return 0;
		}
		*ptr = (Word_t) gcc_jit_context_new_function(jit->ctx,
				gcc_jit_context_new_location(jit->ctx, filename, (*func)->instructions[0]->line_on_file, 0),
				GCC_JIT_FUNCTION_EXPORTED,
				jit->void_type,
				(*func)->name,
				1, &jit->pilha_param,
				0);
	}
	return 1;
}

static int pia_setup_functions(pia_jit *jit, pia_parsed_function **functions, const char *filename) {
	gcc_jit_context *ctx = jit->ctx;

	jit->void_type = gcc_jit_context_get_type(jit->ctx, GCC_JIT_TYPE_VOID);
	jit->pilha_struct = pia_setup_pilha_struct(jit->ctx);
	jit->pilha_type = gcc_jit_struct_as_type(jit->pilha_struct);
	jit->pilha_param = gcc_jit_context_new_param(ctx, NULL, jit->pilha_type, "pilha");

	if(!pia_create_functions(jit, functions, filename)) return 0;
	/* pia_populate_functions(jit, functions); */
	return 1;
}

int pia_run_file(pia_jit *jit, const char *filename) {
	pia_parsed_function **functions = pia_parse_file(jit->parser, filename);
	if(!functions) return 1;

	if(!pia_setup_functions(jit, functions, filename)) return 2;
	gcc_jit_result *compiled = gcc_jit_context_compile(jit->ctx);

	
	pia_free_parsed_functions(functions);
	gcc_jit_result_release(compiled);
	return 0;
}
