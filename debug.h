#ifndef __PIA_DEBUG_H__
#define __PIA_DEBUG_H__

#define FERROR(func, fmt, ...) \
	fprintf(stderr, "[ERROR " func " @ " __FILE__ ":%d] " fmt "\n", __LINE__, ##__VA_ARGS__)

#endif

