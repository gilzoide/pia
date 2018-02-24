#ifndef __PIA_INSTR_H__
#define __PIA_INSTR_H__

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
	int opcode;
	int line_on_file;
} pia_instr;

#endif

