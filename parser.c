#include "parser.h"
#include "debug.h"
#include "jit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

// Utils
static void find_current_line(pia_parser_state *state, size_t cur_pos) {
	int i;
	for(i = state->last_eol_pos + 1; i <= cur_pos; i++) {
		if(state->input[i] == '\n') {
			state->line_count++;
			state->last_eol_pos = i;
		}
	}
}

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
// Instructions
static pia_instr *new_instruction(int opcode, int line) {
	pia_instr *new_instr = malloc(sizeof(pia_instr));
	if(new_instr) {
		new_instr->opcode = opcode;
		new_instr->line_on_file = line;
	}
	return new_instr;
}
static pt_data instr_NOP(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(NOP, state->line_count) };
}
static pt_data instr_DUP(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(DUP, state->line_count) };
}
static pt_data instr_ROT(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(ROT, state->line_count) };
}
static pt_data instr_ADD(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(ADD, state->line_count) };
}
static pt_data instr_SUB(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(SUB, state->line_count) };
}
static pt_data instr_MUL(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(MUL, state->line_count) };
}
static pt_data instr_DIV(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(DIV, state->line_count) };
}
static pt_data instr_MOD(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(MOD, state->line_count) };
}
static pt_data instr_PUSH(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	pia_instr *new_instr = new_instruction(PUSH, state->line_count);
	if(new_instr) {
		new_instr->r1.d = argv[0].d;
	}
	return (pt_data){ .p = new_instr };
}
static pt_data instr_POP(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .p = new_instruction(POP, state->line_count) };
}
static pt_data instr_CALL(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	pia_instr *new_instr = new_instruction(CALL, state->line_count);
	if(new_instr) {
		new_instr->r1.s = argv[0].p;
	}
	return (pt_data){ .p = new_instr };
}
static pt_data instr_PRINT(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	pia_instr *new_instr = new_instruction(PRINT, state->line_count);
	if(new_instr) {
		new_instr->r1.s = argc > 0 ? argv[0].p : NULL;
	}
	return (pt_data){ .p = new_instr };
}
// Functions
static pt_data func_name(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	return (pt_data){ .p = strndup(str + begin, end - begin) };
}
static pt_data func_line(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	find_current_line(state, begin);
	return (pt_data){ .i = state->line_count };
}
static pia_parsed_function *new_function(char *name, int instr_count, pia_instr **instructions, int line) {
	pia_parsed_function *new_func = malloc(sizeof(pia_parsed_function));
	if(new_func) {
		new_func->instructions = malloc(instr_count * sizeof(pia_instr *));
		if(!new_func->instructions) {
			free(new_func);
			new_func = NULL;
		}
		else {
			new_func->instr_count = instr_count;
			new_func->name = name;
			new_func->line_on_file = line;
			memcpy(new_func->instructions, instructions, instr_count * sizeof(pia_instr *));
		}
	}
	return new_func;
}
static pt_data defun(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	state->func_count++;
	return (pt_data){ .p = new_function(argv[1].p, argc - 2, (pia_instr **) (argv + 2), argv[0].i) };
}
static pt_data add_to_main(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	pia_parsed_function *_main = &state->main;
	pia_instr **instructions = realloc(_main->instructions, (++_main->instr_count) * sizeof(pia_instr *));
	if(instructions) {
		instructions[_main->instr_count - 1] = argv[0].p;
		_main->instructions = instructions;
	}
	return PT_NULL_DATA;
}
static pt_data all_funcs(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
	pia_parser_state *state = data;
	pia_parsed_function *_main = malloc(sizeof(pia_parsed_function));
	if(!_main) return PT_NULL_DATA;
	*_main = state->main;
	pia_parsed_function **ret = malloc((state->func_count + 1) * sizeof(pia_parsed_function *));
	if(ret) {
		ret[0] = _main;
		int i, current_index = 1;
		for(i = 0; i < argc; i++) {
			if(argv[i].p != NULL) {
				ret[current_index++] = argv[i].p;
			}
		}
		assert(current_index == state->func_count && "[parser.c::all_funcs]");
		ret[state->func_count] = NULL;
	}
	return (pt_data){ .p = ret };
}

#include <pega-texto/macro-on.h>
pt_grammar *pia_create_parser() {
    pt_rule rules[] = {
        { "Programa", SEQ(Q(V("HashBang"), -1),
                          V("S"),
                          Q_(all_funcs, SEQ(OR(V_(defun, "DefFuncao"), V_(add_to_main, "Instrucao")), V("S")), 0),
                          V("EOF")) },
        { "HashBang", SEQ(L("#!"), Q(BUT(L("\n")), 0)) },
        { "Instrucao", SEQ(OR(I_(instr_NOP, "NOP"),
                              I_(instr_DUP, "DUP"),
                              I_(instr_ROT, "ROT"),
                              I_(instr_ADD, "ADD"),
                              I_(instr_SUB, "SUB"),
                              I_(instr_MUL, "MUL"),
                              I_(instr_DIV, "DIV"),
                              I_(instr_MOD, "MOD"),
                              SEQ_(instr_PUSH, I("PUSH"), V("EspacoArg"), V("Constante")),
                              I_(instr_POP, "POP"),
                              SEQ_(instr_CALL, I("CALL"), V("EspacoArg"), V("NomeFuncao")),
                              SEQ_(instr_PRINT, I("PRINT"), Q(SEQ(V("EspacoArg"), V("String")), -1))),
                           V("EOI")) },
        { "DefFuncao", SEQ(I_(func_line, "FUNCTION"), V("EspacoArg"), V("NomeFuncao"), V("EOI"), V("S"),
                                          Q(SEQ(V("Instrucao"), V("S")), 0),
                           OR(I("END"), E(ERR_END_EXPECTED, NULL))) },
        { "NomeFuncao", SEQ_(func_name, OR(S("-_"), C(isalpha)), Q(OR(S("-_"), C(isalnum)), 0)) },

        { "Constante", Q_(todouble, C(isdigit), 1) },  // TODO: float
        { "String", SEQ_(tostring, L("\""), Q(SEQ(NOT(L("\"")), V("Char")), 0), L("\"")) },
        { "Char", OR_(tochar,
                      SEQ(L("\\"), S("abfnrtv\'\"\\")), // escaped characters
                      ANY) },

        { "EspacoArg", Q(S(" \t"), 0) },
        { "EOI", SEQ(Q(OR(S(" \t"), V("Comentario")), 0), OR(L(";"), V("EOL"), V("EOF"))) },
        { "EOL", L("\n") },
        { "EOF", OR(NOT(ANY), E(ERR_UNEXPECTED_TOKEN, NULL)) },
        { "Comentario", SEQ(L("#"), Q(BUT(L("\n")), 0)) },
        { "S", Q(OR(C(isspace), V("Comentario")), 0) },
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
		buffer = malloc((buffer_size + 1) * sizeof(char));
		if(buffer) {
			fread(buffer, sizeof(char), buffer_size, f);
			buffer[buffer_size] = '\0';
		}
		fclose(f);
	}
	return buffer;
}

static void on_error(const char *str, size_t where, int code, void *data) {
	pia_parser_state *state = data;
	printf("!!%d", state->line_count);
	find_current_line(state, where);
	printf(" -> %d\n", state->line_count);
	fprintf(stderr, "Erro de parse @ %s:%d:%ld: %s\n",
			state->filename,
			state->line_count,
			where - state->last_eol_pos + 1,
			pia_error_messages[code]);
}

pia_parsed_function **pia_parse_file(pia_parser *parser, const char *filename) {
	char *input = read_file(filename);
	if(input == NULL) {
		FERROR("pia_parse_file", "%s", strerror(errno));
		return NULL;
	}

	pia_parser_state state = {
		.filename = filename,
		.input = input,
		.main = (pia_parsed_function){
			.instructions = NULL,
			.name = strdup(PIA_MAIN_FUNCTION),
			.instr_count = 0,
			.line_on_file = 0, // 'main' is implicitly declared
		},
		.func_count = 1, // 'main' function
		.line_count = 1, // start at line 1
		.last_eol_pos = -1,
	};
	pt_match_options opts = { &state, .on_error = on_error };
	pt_match_result res = pt_match_grammar(parser, input, &opts);

	free(input);
	return res.data.p;
}

void pia_free_parsed_function_instrs(pia_parsed_function *f) {
	int i;
	for(i = 0; i < f->instr_count; i++) {
		pia_instr *instr = f->instructions[i];
		if(instr->opcode == PRINT || instr->opcode == CALL) {
			free(instr->r1.s);
		}
		free(instr);
	}
	free(f->instructions);
}

void pia_free_parsed_functions(pia_parsed_function **parsed_functions) {
	pia_parsed_function **it;
	for(it = parsed_functions; *it; it++) {
		free((*it)->name);
		pia_free_parsed_function_instrs(*it);
		free((*it));
	}
	free(parsed_functions);
}

