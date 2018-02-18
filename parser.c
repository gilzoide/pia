#include "parser.h"
#include "jit.h"

#include <ctype.h>

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
static pt_data conta_eol(const char *str, size_t begin, size_t end, int argc, pt_data *argv, void *data) {
    pia_jit *jit = data;
    jit->line_count++;
    jit->last_eol_pos = begin;
    return PT_NULL_DATA;
}

#include <pega-texto/macro-on.h>
pt_grammar *pia_create_parser() {
    pt_rule rules[] = {
        { "Programa", SEQ(V("HashBang"), V("S"), Q(SEQ(OR(V("DefFuncao"), V("Instrucao")), V("S")), 0), V("EOF")) },
        { "HashBang", SEQ(L("#!"), Q(BUT(L("\n")), 0)) },
        { "Instrucao", SEQ(OR(L("dup"),
                              L("rot"),
                              L("add"),
                              L("sub"),
                              L("mul"),
                              L("div"),
                              L("mod"),
                              SEQ(L("push"), V("EspacoArg"), V("Constante")),
                              SEQ(L("call"), V("EspacoArg"), V("NomeFuncao")),
                              SEQ(L("print"), Q(SEQ(V("EspacoArg"), V("String")), -1))),
                           V("EOI")) },
        { "DefFuncao", SEQ(L("function"), V("EspacoArg"), V("NomeFuncao"), V("EOI"),
                                          Q(SEQ(V("Instrucao"), V("S")), 0),
                           L("end")) },
        { "NomeFuncao", SEQ(OR(L("_"), F(isalpha)), Q(OR(L("_"), F(isalnum)), 0)) },

        { "Constante", Q_(todouble, F(isdigit), 1) },  // TODO: float
        { "String", SEQ_(tostring, L("\""), Q(SEQ(NOT(L("\"")), V("Char")), 0), L("\"")) },
        { "Char", OR_(tochar,
                      SEQ(L("\\"), S("abfnrtv\'\"\\")),
                      ANY) },

        { "EspacoArg", Q(S(" \t"), 0) },
        { "EOI", SEQ(Q(OR(S(" \t"), V("Comentario")), 0), OR(L(";"), V("EOL"), V("EOF"))) },
        { "EOL", L("\n") },
        { "EOF", NOT(ANY) },
        { "Comentario", SEQ(L("#"), Q(BUT(L("\n")), 0)) },
        { "S", Q(OR(F(isspace), V("Comentario")), 0) },
        { NULL, NULL },
    };

    pt_grammar *g = pt_create_grammar(rules, 0);
    pt_validate_grammar(g, PT_VALIDATE_PRINT_ERROR);
    return g;
}
#include <pega-texto/macro-off.h>

