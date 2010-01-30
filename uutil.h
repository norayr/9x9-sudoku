#ifndef UUTIL_H
#define UUTIL_H

#include <stdarg.h>

void die(const char * format, ...);
void vwarn(const char * format, va_list ap);
int move_fd(int oldfd, int newfd);
void vfdprintf1k(int fd, const char * format, va_list ap);
void fdprintf1k(int fd, const char * format, ...);

#endif /* UUTIL_H */
