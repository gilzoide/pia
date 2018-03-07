#ifndef __PIA_INSTR_H__
#define __PIA_INSTR_H__

#include <libgccjit.h>

#include <stdint.h>

enum {
	DUP = 0,
	ROT,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	PUSH,
	CALL,
	PRINT,

	INSTRUCOES_END,
};

static const char * const pia_instruction_names[] = {
	"DUP",
	"ROT",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOD",
	"PUSH",
	"CALL",
	"PRINT",
};

typedef struct {
	union {
		double d; // Argument for PUSH instruction
		char *s;  // Argument for CALL or PRINT instructions
	} r1;
	int line_on_file;
	int8_t opcode;
} pia_instr;

// forward declaration
typedef struct pia_jit pia_jit;

int pia_instr_populate(pia_jit *jit, gcc_jit_function *func, gcc_jit_block *block, pia_instr *instr);

#endif

