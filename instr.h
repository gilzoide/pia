#ifndef __PIA_INSTR_H__
#define __PIA_INSTR_H__

typedef enum {
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
} instruction_opcode;

typedef struct {
	int opcode;
	union {
		double d; // Argument for PUSH instruction
		char *s;  // Argument for CALL or PRINT instructions
	} r1;
} instr;

#endif

