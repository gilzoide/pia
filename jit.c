#include "jit.h"
#include "debug.h"
#include "parser.h"

#include <assert.h>

int pia_initialize_jit(pia_jit *jit) {
	jit_init();
	jit->parser = pia_create_parser();
	jit->ctx = jit_context_create();
	jit->func_map = NULL;
	return jit->parser && jit->ctx;
}

void pia_destroy_jit(pia_jit *jit) {
	pia_destroy_parser(jit->parser);
	jit_context_destroy(jit->ctx);
	Word_t w;
	JSLFA(w, jit->func_map);
}

static int create_functions(pia_jit *jit, pia_parsed_function **functions) {
	pia_parsed_function **funcp;
	pia_parsed_function *func;
	jit_function_t *ptr;
	for(funcp = functions; *funcp; funcp++) {
		func = *funcp;
		JSLI(ptr, jit->func_map, (uint8_t *) func->name);
		if(*ptr != NULL) {
			FERROR("create_functions", "Function \"%s\" is already defined", func->name);
			return 0;
		}
		*ptr = jit_function_create(jit->ctx, jit->func_signature);
		if(*ptr == NULL) {
			FERROR("create_functions", "Couldn't create JIT function");
			return 0;
		}
	}
	return 1;
}

static int populate(pia_jit *jit, jit_function_t jitf, pia_parsed_function *definition) {
	int i;
	jit_context_build_start(jit->ctx);
	for(i = 0; i < definition->instr_count; i++) {
		if(!pia_instr_populate(jit, jitf, definition->instructions[i])) return 0;
	}
	jit_insn_return(jitf, NULL);
	jit_context_build_end(jit->ctx);
	return 1;
}

static int compile_functions(pia_jit *jit, pia_parsed_function **functions) {
	pia_parsed_function **funcp;
	pia_parsed_function *func;
	jit_function_t *ptr;
	jit_function_t jitf;
	for(funcp = functions; *funcp; funcp++) {
		func = *funcp;
		JSLG(ptr, jit->func_map, (uint8_t *) func->name);
		assert(ptr && "[jit.c::populate_functions] Funções deveriam já ter sido todas criadas");
		jitf = *ptr;
		if(!populate(jit, jitf, func)) return 0;
		
		// dump_func(jitf, func->name);
		
		if(!jit_function_compile(jitf)) {
			FERROR("populate_functions", "Erro ao compilar função \"%s\"", func->name);
			jit_function_abandon(jitf);
			return 0;
		}
	}
	return 1;
}

static int setup_functions(pia_jit *jit, pia_parsed_function **functions) {
	jit->_intp = jit_type_create_pointer(jit_type_int, 0);
	jit->_doublep = jit_type_create_pointer(jit_type_float64, 0);
	jit->func_signature = jit_type_create_signature(
			jit_abi_cdecl,
			jit_type_void,
			(jit_type_t[]){ jit->_doublep, jit->_intp },
			2, 0);

	jit->_charp = jit_type_create_pointer(jit_type_sbyte, 0);
	jit->printf_s_signature = jit_type_create_signature(
			jit_abi_cdecl,
			jit_type_sys_int,
			(jit_type_t[]){ jit->_charp, jit->_charp },
			2, 0);
	jit->printf_d_signature = jit_type_create_signature(
			jit_abi_cdecl,
			jit_type_sys_int,
			(jit_type_t[]){ jit->_charp, jit_type_float64 },
			2, 0);

	if(!create_functions(jit, functions)) return 0;
	if(!compile_functions(jit, functions)) return 0;
	return 1;
}

static void setup_jit_options(pia_jit *jit) {
}

jit_function_t pia_find_function(pia_jit *jit, const char *name) {
	jit_function_t *ptr;
	JSLG(ptr, jit->func_map, (uint8_t *) name);
	return ptr ? *ptr : NULL;
}

int pia_run_file(pia_jit *jit, const char *filename) {
	jit->filename = filename;
	pia_parsed_function **functions = pia_parse_file(jit->parser, filename);
	if(!functions) return 1;

	if(!setup_functions(jit, functions)) return 2;
	setup_jit_options(jit);

	// call 
	jit_function_t _main = pia_find_function(jit, PIA_MAIN_FUNCTION);
	jit_float64 v[STACK_MAX];
	jit_int s = -1;
	jit_float64 *arg1 = v;
	jit_int *arg2 = &s;
	void *args[] = {
		&arg1,
		&arg2,
	};
	jit_function_apply(_main, args, NULL);
	
	pia_free_parsed_functions(functions);
	return 0;
}
