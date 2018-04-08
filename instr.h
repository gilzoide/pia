#ifndef __PIA_INSTR_H__
#define __PIA_INSTR_H__

#include <jit/jit.h>

#include <stdint.h>

typedef enum {
	NOP = 0,
	DUP,
	ROT,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	PUSH,
	POP,
	CALL,
	PRINT,

	INSTRUCOES_END,
} pia_opcode;

static const char * const pia_instruction_names[] = {
	"NOP",
	"DUP",
	"ROT",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOD",
	"PUSH",
	"POP",
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

int pia_instr_populate(pia_jit *jit, jit_function_t func, pia_instr *instr);

#endif

