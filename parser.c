#include "parser.h"
#include "debug.h"
#include "jit.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

// Constants
static pt_data todouble(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
    return (pt_data){ .d = atof(str + begin) };
}
static pt_data tostring(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
    char *new_str;
    if(new_str = malloc((argc + 1) * sizeof(char))) {
        int i;
        for(i = 0; i < argc; i++) {
            new_str[i] = argv[i].c;
        }
        new_str[i] = '\0';
    }
    return (pt_data){ .p = new_str };
}
static pt_data tochar(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
    char c = str[begin];
    if(c == '\\') {
        c = str[begin + 1];
        switch(c) {
            case 'a': c = '\a'; break;
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case 'v': c = '\v'; break;
            default: break;
        }
    }
    return (pt_data){ .c = c };
}
static int eolcount(const char *str, void *data) {
	int iseol = *str == '\n';
	if(iseol) {
		pia_parser_state *state = data;
		size_t where = str - state->input;
		state->line_count++;
		state->last_eol_pos = where;
	}
    return iseol;
}
// Instructions
static pia_instr *new_instruction(int opcode, int line) {
	pia_instr *new_instr = malloc(sizeof(pia_instr));
	if(new_instr) {
		new_instr->opcode = opcode;
		new_instr->line_on_file = line;
	}
	return new_instr;
}
static pt_data instr_DUP(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	return (pt_data){ .p = new_instruction(DUP, state->line_count) };
}
static pt_data instr_ROT(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	return (pt_data){ .p = new_instruction(ROT, state->line_count) };
}
static pt_data instr_ADD(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	return (pt_data){ .p = new_instruction(ADD, state->line_count) };
}
static pt_data instr_SUB(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	return (pt_data){ .p = new_instruction(SUB, state->line_count) };
}
static pt_data instr_MUL(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	return (pt_data){ .p = new_instruction(MUL, state->line_count) };
}
static pt_data instr_DIV(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	return (pt_data){ .p = new_instruction(DIV, state->line_count) };
}
static pt_data instr_MOD(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	return (pt_data){ .p = new_instruction(MOD, state->line_count) };
}
static pt_data instr_PUSH(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	pia_instr *new_instr = new_instruction(PUSH, state->line_count);
	if(new_instr) {
		new_instr->r1.d = argv[0].d;
	}
	return (pt_data){ .p = new_instr };
}
static pt_data instr_CALL(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	pia_instr *new_instr = new_instruction(CALL, state->line_count);
	if(new_instr) {
		new_instr->r1.s = strdup(argv[0].p);
	}
	return (pt_data){ .p = new_instr };
}
static pt_data instr_PRINT(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	pia_instr *new_instr = new_instruction(PRINT, state->line_count);
	if(new_instr) {
		new_instr->r1.s = argc > 0 ? strdup(argv[0].p) : NULL;
	}
	return (pt_data){ .p = new_instr };
}

#include <pega-texto/macro-on.h>
pt_grammar *pia_create_parser() {
    pt_rule rules[] = {
        { "Programa", SEQ(Q(V("HashBang"), -1), V("S"), Q(SEQ(OR(V("DefFuncao"), V("Instrucao")), V("S")), 0), V("EOF")) },
        { "HashBang", SEQ(L("#!"), Q(BUT(L("\n")), 0)) },
        { "Instrucao", SEQ(OR(I_(instr_DUP, "DUP"),
                              I_(instr_ROT, "ROT"),
                              I_(instr_ADD, "ADD"),
                              I_(instr_SUB, "SUB"),
                              I_(instr_MUL, "MUL"),
                              I_(instr_DIV, "DIV"),
                              I_(instr_MOD, "MOD"),
                              SEQ_(instr_PUSH, I("PUSH"), V("EspacoArg"), V("Constante")),
                              SEQ_(instr_CALL, I("CALL"), V("EspacoArg"), V("NomeFuncao")),
                              SEQ_(instr_PRINT, I("PRINT"), Q(SEQ(V("EspacoArg"), V("String")), -1))),
                           V("EOI")) },
        { "DefFuncao", SEQ(I("FUNCTION"), V("EspacoArg"), V("NomeFuncao"), V("EOI"),
                                          Q(SEQ(V("Instrucao"), V("S")), 0),
                           I("END")) },
        { "NomeFuncao", SEQ(OR(L("_"), C(isalpha)), Q(OR(L("_"), C(isalnum)), 0)) },

        { "Constante", Q_(todouble, C(isdigit), 1) },  // TODO: float
        { "String", SEQ_(tostring, L("\""), Q(SEQ(NOT(L("\"")), V("Char")), 0), L("\"")) },
        { "Char", OR_(tochar,
                      SEQ(L("\\"), S("abfnrtv\'\"\\")),
                      ANY) },

        { "EspacoArg", Q(S(" \t"), 0) },
        { "EOI", SEQ(Q(OR(S(" \t"), V("Comentario")), 0), OR(L(";"), V("EOL"), V("EOF"))) },
        { "EOL", F(eolcount) },
        { "EOF", OR(NOT(ANY), E(ERR_UNEXPECTED_TOKEN, NULL)) },
        { "Comentario", SEQ(L("#"), Q(BUT(L("\n")), 0)) },
        { "S", Q(OR(V("EOL"), C(isspace), V("Comentario")), 0) },
        { NULL, NULL },
    };

    pt_grammar *g = pt_create_grammar(rules, 0);
    pt_validate_grammar(g, PT_VALIDATE_PRINT_ERROR);
    return g;
}
#include <pega-texto/macro-off.h>

static char *read_file(const char *filename) {
	char *buffer;
	FILE *f = fopen(filename, "r");
	if(f) {
		fseek(f, 0, SEEK_END);
		long buffer_size = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(buffer_size * sizeof(char));
		if(buffer) {
			fread(buffer, sizeof(char), buffer_size, f);
		}
		fclose(f);
	}
	return buffer;
}

static void on_error(const char *str, size_t where, int code, void *data) {
	pia_parser_state *state = data;
	const char *msg;
	switch(code) {
		case ERR_UNEXPECTED_TOKEN:
			msg = "token inesperado";
			break;
	}
	fprintf(stderr, "Erro de parse @ %s:%d:%d: %s\n",
			state->filename,
			state->line_count + 1,
			where - state->last_eol_pos,
			msg);
}

pia_parsed_function **pia_parse_file(pia_parser *parser, const char *filename) {
	char *input = read_file(filename);
	if(input == NULL) {
		FERROR("pia_parse_file", "%s", strerror(errno));
		return NULL;
	}

	pia_parser_state state = { filename, input };
	pt_match_options opts = { &state, .on_error = on_error };
	pt_match_result res = pt_match_grammar(parser, input, &opts);

	free(input);
	return res.data.p;
}

void pia_free_parsed_functions(pia_parsed_function **parsed_functions) {
	pia_parsed_function **it;
	for(it = parsed_functions; *it; it++) {
		free((*it)->name);
		free((*it)->instructions);
		free((*it));
	}
	free(parsed_functions);
}

