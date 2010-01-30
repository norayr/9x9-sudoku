#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -eu;
 WARN="-Wall -Wno-long-long -Wstrict-prototypes -pedantic"
 WARN="$WARN -Wcast-align -Wpointer-arith " # -Wfloat-equal #-Werror
 WARN="$WARN -W -Wwrite-strings -Wcast-qual -Wshadow" # -Wconversion
 case ${1:-} in '') set x -O2 ### set x -ggdb;
	shift ;; esac;
 set -x; ${CC:-gcc} $WARN "$@" -c "$0"
 exit $?
 */
#endif
/* ************************* lineread.c ************************* */
/*$Id: lineread.c,v 1.1 1996/04/18 15:33:56 too Exp $" */
/*
 *      lineread.c - functions to read lines from fd:s effectively
 *
 * Created: sometime year 1991  // too
 * Last Modified: Sat 30 Jan 2010 18:54:31 EET too
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "lineread.h"

typedef enum { false = 0, true = 1 } bool;

#define NLCHR '\n'

int lineread(LineRead * lr, char ** ptr)
{
  int i;

  if (lr->currp == lr->endp)

    if (lr->selected)	/* user called select() (or wants to block) */
    {
      if (lr->line_completed)
	lr->startp = lr->currp = lr->data;

      if ((i = read(lr->fd,
		    lr->currp,
		    lr->sizep - lr->currp)) <= 0) {
	/*
	 * here if end-of-file or on error. set endp == currp
	 * so if non-blocking I/O is in use next call will go to read()
	 */
	lr->endp = lr->currp;
	*ptr = (char *)i; /* user compares ptr (NULL, (char *)-1, ... */
	return -1;
      }
      else
	lr->endp = lr->currp + i;
    }
    else /* Inform user that next call may block (unless select()ed) */
    {
      lr->selected = true;
      return 0;
    }
  else /* currp has not reached endp yet. */
  {
    *lr->currp = lr->saved;
    lr->startp = lr->currp;
  }

  /*
   * Scan read string for next newline.
   */
  while (lr->currp < lr->endp)
    if (*lr->currp++ == NLCHR)	/* memchr ? */
    {
      lr->line_completed = true;
      lr->saved = *lr->currp;
      *lr->currp = '\0';
      lr->selected = false;
      *ptr = lr->startp;

      return lr->currp - lr->startp;
    }

  /*
   * Here if currp == endp, but no NLCHAR found.
   */
  lr->selected = true;

  if (lr->currp == lr->sizep) {
    /*
     * Here if currp reaches end-of-buffer (endp is there also).
     */
    if (lr->startp == lr->data) /* (data buffer too short for whole string) */
    {
      lr->line_completed = true;
      *ptr = lr->data;
      *lr->currp = '\0';
      return -1;
    }
    /*
     * Copy partial string to start-of-buffer and make control ready for
     * filling rest of buffer when next call to lineRead() is made
     * (perhaps after select()).
     */
    memmove(lr->data, lr->startp, lr->endp - lr->startp);
    lr->endp-=  (lr->startp - lr->data);
    lr->currp = lr->endp;
    lr->startp = lr->data;
  }

  lr->line_completed = false;
  return 0;
}

#if 0
int lineread_rest(LineRead * lr, char ** ptr)
{
  int rv = lr->endp - lr->currp;

  *lr->currp = lr->saved;

  *ptr = lr->currp;

  lr->currp = lr->endp = 0;
  lr->selected = lr->line_completed = true;

  return rv;
}
#endif

void lineread_init(LineRead * lr, int fd)
{
  lr->fd = fd;

  lr->currp = lr->endp = NULL; /* any value works */
  lr->selected = lr->line_completed = true;
}
