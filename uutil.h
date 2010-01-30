#ifndef UUTIL_H
#define UUTIL_H

#include <stdarg.h>

void die(const char * format, ...);
void vwarn(const char * format, va_list ap);

#endif /* UUTIL_H */
