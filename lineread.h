
#ifndef LINEREAD_H
#define LINEREAD_H

#define LR_BUFSIZE 4096
/* #define LR_BUFSIZE 59 */ /* test value */

typedef struct
{
  int    fd;              /* input file descriptor */
  char * currp;           /* current scan point in buffer */
  char * endp;            /* pointer of last read character in buffer */
  char * startp;          /* pointer to start of output */
  char * sizep;           /* pointer to the end of read buffer */
  char   selected;        /* has caller done select()/poll() or does he care */
  char   line_completed;  /* line completion in LineRead */
  unsigned char saved;    /* saved char in LineRead */
  char   data[LR_BUFSIZE];/* data allocated when structure created */
} LineRead;

int lineread(LineRead * lr, char ** ptr);
void lineread_init(LineRead * lr, int fd);
int lineread_count(LineRead * lr);

#endif /* LINEREAD_H */
