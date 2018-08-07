/****************************************************************************
|*
|* tap3edit Tools (http://www.tap3edit.com)
|*
|* $Id: mp.c 52 2014-09-20 15:08:24Z mrjones $
|*
|* Copyright (c) 2014, Javier Gutierrez <jgutierrez@tap3edit.com>
|* 
|* Permission to use, copy, modify, and/or distribute this software for any
|* purpose with or without fee is hereby granted, provided that the above
|* copyright notice and this permission notice appear in all copies.
|* 
|* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
|* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
|* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
|* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
|* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
|* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
|* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
|*
|*
|* Module: mp_trc.c
|*
|* Description: Memory pool management trace functions
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20140922     JG              Initial version
|*
****************************************************************************/

/* Includes and defines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if _WIN32
#   include <STDDEF.h>
#else
#   include <stdint.h>
#endif
#include <time.h>
#include <ctype.h>
#include <errno.h>

#include "mp.h"
#include "mp_trc.h"

/* Prototypes */
static int mptrc_internal(FILE* fd, char *fmt, va_list ap);

/* Global variables */
static int (*mptrc_fn)(FILE *fd, char *fmt, va_list ap) = &mptrc_internal; /* Pointer to the configurable trace() function */

#if MP_THREAD_SAFE == 1 /* { */
extern MP_TLS_INT mperrno;
#else /* } MP_THREAD_SAFE { */
extern int mperrno;
#endif /* } MP_THREAD_SAFE */


/****************************************************************************
|*
|* Function: mpset_trc_fn
|*
|* Description;
|*
|*     Sets the function to be used to display on screen
|*
|* Return:
|*     MP_ERRNO_SUCCESS on success
|*     MP_ERRNO_PARM on error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140812    JG    Initial version
|*
****************************************************************************/
int mptrc_set_fn(int (*function)(FILE *fd, char *fmt, va_list ap))
{
    mperrno = MP_ERRNO_SUCCESS;
    if (function == NULL)
    {
        mperrno = MP_ERRNO_PARM;
        return MP_ERRNO_PARM;
    }
    mptrc_fn = function;
    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mptrc
|*
|* Description;
|*
|*     Function used to display messages. it calls a configurable function.
|*
|* Return:
|*     MP_ERRNO_SUCCESS on success
|*     MP_ERRNO_PARM, MP_ERRNO_DISP on error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mptrc(FILE *fd, char *fmt, ...)
{
    int rc = MP_ERRNO_SUCCESS;
    va_list ap;
    FILE *fd_tmp = NULL;

    if (fmt == NULL)
    {
        mperrno = MP_ERRNO_PARM;
        return MP_ERRNO_PARM;
    }

    fd_tmp = fd == NULL ? stdout : fd;

    va_start(ap, fmt);

    if (mptrc_fn(fd_tmp, fmt, ap))
    {
        mperrno = MP_ERRNO_DISP;
        rc = MP_ERRNO_DISP;
    }

    va_end(ap);
    return rc;
}

/****************************************************************************
|*
|* Function: mptrc_internal
|*
|* Description;
|*
|*     Internal function to display messages on screen. This function can be
|*     replaced by a function defined on the client application set with the
|*     function mptrc_set_fn().
|*
|* Return:
|*     return code of the system function used to display.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140812    JG    Initial version
|*
****************************************************************************/
static int mptrc_internal(FILE *fd, char *fmt, va_list ap)
{
    int rc = 0;

    rc = vfprintf(fd, fmt, ap);
    fprintf(fd, "\n");

    return rc;
}


/* EOF */
