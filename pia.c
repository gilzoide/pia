#include <libgccjit.h>
#include "parser.h"

int main(int argc, char **argv) {
	pt_grammar *g = cria_parser();

	pt_destroy_grammar(g);
	return 0;
}
