#ifndef _VIDEO_CALL_LOG
#define _VIDEO_CALL_LOG

#include <stdio.h>
#include <stdarg.h>

void indent_print_log(int indent, const char *format, ...);

void print_log(const char *format, ...);

#endif
