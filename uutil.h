#ifndef UUTIL_H
#define UUTIL_H

#include <stdarg.h>

void die(const char * format, ...);
void vwarn(const char * format, va_list ap);
int move_fd(int oldfd, int newfd);

#endif /* UUTIL_H */
