#ifndef __PIA_DEBUG_H__
#define __PIA_DEBUG_H__

#include <jit/jit.h>
#include <jit/jit-dump.h>

#define FERROR(func, fmt, ...) \
	fprintf(stderr, "[ERROR " func " @ " __FILE__ ":%d] " fmt "\n", __LINE__, ##__VA_ARGS__)

void dump_func(jit_function_t func, const char *name);

#endif

