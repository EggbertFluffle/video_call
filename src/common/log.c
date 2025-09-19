#include <stdio.h>

#include <stdio.h>
#include <stdarg.h>
#include "log.h"

void indent_print_log(int indent, const char *format, ...) {
    va_list args;

	#ifdef _CLIENT
    printf("[CLIENT] ");
	#endif

	#ifdef _SERVER
    printf("[SERVER] ");
	#endif

    for (int i = 0; i < indent; i++) {
        printf("\t");
    }
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void print_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

	#ifdef _CLIENT
    printf("[CLIENT] ");
	#endif

	#ifdef _SERVER
    printf("[SERVER] ");
	#endif

    vprintf(format, args);
    va_end(args);
    printf("\n");
}
