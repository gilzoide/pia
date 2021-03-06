#ifndef __PIA_PARSER_H__
#define __PIA_PARSER_H__

#include <pega-texto.h>

#include "instr.h"

typedef struct {
	pia_instr **instructions;
	char *name;
	int instr_count;
	int line_on_file;
} pia_parsed_function;

typedef struct {
	const char *filename;
	const char *input;
	pia_parsed_function main;
	int func_count;
	int line_count;
	int last_eol_pos;
} pia_parser_state;

enum {
	ERR_UNEXPECTED_TOKEN,
	ERR_END_EXPECTED,
};
static const char * const pia_error_messages[] = {
	[ERR_UNEXPECTED_TOKEN] = "token inesperado",
	[ERR_END_EXPECTED] = "'END' esperado fechando função",
};

typedef pt_grammar pia_parser;

/**
 * Create the parser.
 */
pia_parser *pia_create_parser();
#define pia_destroy_parser pt_destroy_grammar
/**
 * Parse a file, returning the parsed functions.
 *
 * This returns a NULL-terminated array of parsed functions, the first one
 * always being `main`, populated by top-level instructions.
 *
 * If there was any error, this prints it to stderr and return NULL.
 */
pia_parsed_function **pia_parse_file(pia_parser *parser, const char *filename);
/**
 * Free the memory associated with the parsed functions.
 */
void pia_free_parsed_functions(pia_parsed_function **parsed_functions);

#endif

