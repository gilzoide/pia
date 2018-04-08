#include "debug.h"

void dump_func(jit_function_t func, const char *name) {
	jit_dump_function(stderr, func, name);
	puts("");
}
