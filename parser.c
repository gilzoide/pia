#include "parser.h"

#include <ctype.h>

#include <pega-texto/macro-on.h>
pt_grammar *cria_parser() {
    pt_rule rules[] = {
        { "Programa", SEQ(V("HashBang"), V("S"), Q(SEQ(OR(V("Instrucao"), V("DefFuncao")), V("S")), 0), V("EOF")) },
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
                              L("print")),
                           V("EOI")) },
        { "DefFuncao", SEQ(L("function"), V("EspacoArg"), V("NomeFuncao"), V("EOI"),
                                          Q(SEQ(V("Instrucao"), V("S")), 0),
                           L("end")) },
        { "NomeFuncao", SEQ(OR(L("_"), F(isalpha)), Q(OR(L("_"), F(isalnum)), 0)) },
        { "Constante", Q(F(isdigit), 1) },  // TODO: float
        { "EspacoArg", Q(S(" \t"), 0) },
        { "EOI", SEQ(Q(OR(S(" \t"), V("Comentario")), 0), OR(L(";"), V("EOL"), V("EOF"))) },
        { "EOL", L("\n") },
        { "EOF", NOT(ANY) },
        { "Comentario", SEQ(L("#"), Q(BUT(L("\n")), 0)) },
        { "S", Q(OR(F(isspace), V("Comentario")), 0) },
        { NULL, NULL },
    };

    pt_grammar *g = pt_create_grammar(rules, 0);
    pt_validate_grammar(g, PT_VALIDATE_ABORT);
    return g;
}
#include <pega-texto/macro-off.h>

