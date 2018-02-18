#include "jit.h"
#include "parser.h"

int pia_initialize_jit(pia_jit *jit) {
	jit->grammar = pia_create_parser();
	jit->ctx = gcc_jit_context_acquire();
	jit->line_count = jit->last_eol_pos = 0;
	jit->func_map = NULL;
	return jit->grammar && jit->ctx;
}

void pia_destroy_jit(pia_jit *jit) {
	pt_destroy_grammar(jit->grammar);
	gcc_jit_context_release(jit->ctx);
}
