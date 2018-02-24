#include "jit.h"

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Uso: %s <nome do arquivo>\n", argv[0]);
		return 1;
	}

	pia_jit jit;
	if(!pia_initialize_jit(&jit)) return -1;

	int ret = pia_run_file(&jit, argv[1]);

	pia_destroy_jit(&jit);
	return ret;
}
