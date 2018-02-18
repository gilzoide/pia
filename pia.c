#include "jit.h"

int main(int argc, char **argv) {
	pia_jit jit;
	if(!pia_initialize_jit(&jit)) return -1;

	pia_destroy_jit(&jit);
	return 0;
}
