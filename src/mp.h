/****************************************************************************
|*
|* tap3edit Tools (http://www.tap3edit.com)
|*
|* Copyright (c) 2014-2018, Javier Gutierrez <https://github.com/tap3edit/mp>
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
|* Module: mp.h
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20140801     JG              Initial version
|*
****************************************************************************/

#ifndef _MP_H_ /* { */
#define _MP_H_

/* Includes */
#include <stdarg.h>

/* Defines */
#define MP_THREAD_SAFE 1                                    /* 1 = Thread safe. Deactivate if threads are not needed */

#if MP_THREAD_SAFE == 1
#   include "mp_os.h"
#endif

#ifndef TRUE
    #define FALSE 0
    #define TRUE (!FALSE)
#endif

#define MP_NO_MP_ID              -2                         /* No Memory Pool ID assigned */
#define MP_MAX_MP_ID             100                        /* Maximum number of memory pools */
#define MP_DEF_MP_ID             0                          /* Default memory pool ID */
#define MP_DEF_MP_DESCR          "Default"                  /* Description for the default memory pool */
#define MP_DEF_ALIGN             8                          /* Default memory alignment in our pool */
#define MP_DEF_BLK_SZ            (250 * 1024)               /* Default size of each block inside the each  memory pool (250 Kb)*/
#define MP_DEF_MEM_LIMIT_64      ((size_t)5 * 1024 * 1024 * 1024)    /* Default memory usage limit (5 Gb for 64 bits) */
#define MP_DEF_MEM_LIMIT_32      ((size_t)3 * 1024 * 1024 * 1024)    /* Default memory usage limit (3 Gb for 32 bits) */

#define MP_MAX_DESCR_LEN         128

#define MP_ERRNO_SUCCESS         0                          /* Success */
#define MP_ERRNO_MPID            -10                        /* Memory pool ID out of range */
#define MP_ERRNO_SZNG            -20                        /* Negative size */
#define MP_ERRNO_NOMM            -30                        /* Out of memory */
#define MP_ERRNO_EXMM            -40                        /* Memory limit exceeded */
#define MP_ERRNO_ALLO            -50                        /* Error allocating memory */
#define MP_ERRNO_EXAL            -60                        /* Alignment limit exceeded */
#define MP_ERRNO_NOIN            -70                        /* Memory pool not initialized */
#define MP_ERRNO_EXMP            -80                        /* Number of pools exceeded */
#define MP_ERRNO_NOPP            -90                        /* Nothing to pop */
#define MP_ERRNO_DISP            -100                       /* Display error */
#define MP_ERRNO_PARM            -110                       /* Error on Function parameter */
#define MP_ERRNO_THRD            -130                       /* Expected different thread ID */
#define MP_ERRNO_SYSE            -990                       /* System error. In this case errno message is delivered */

#define MP_ERRSTR_MPID           "Memory pool ID out of range"
#define MP_ERRSTR_SZNG           "Negative size"
#define MP_ERRSTR_NOMM           "Out of memory"
#define MP_ERRSTR_EXMM           "Memory limit exceeded"
#define MP_ERRSTR_ALLO           "Error allocating memory"
#define MP_ERRSTR_EXAL           "Alignment is not bigger than void* or not multiple of 2"
#define MP_ERRSTR_NOIN           "Memory pool is not the default (0) and it is not initialized: use mpnew() first"
#define MP_ERRSTR_EXMP           "Limit of number of Memory Pools exceeded"
#define MP_ERRSTR_NOPP           "Nothing to pop, use first mppush()"
#define MP_ERRSTR_DISP           "Error displaying a message"
#define MP_ERRSTR_PARM           "Error on parameter passed to the function"
#define MP_ERRSTR_THRD           "Expected different thread ID"
#define MP_ERRSTR_SYSE           "System error"             /* This message is not delivered but strerror(errno) */

/* Typedefs and structures */
typedef unsigned char uchar;

typedef struct _mpblock
{
    size_t            size;                                  /* Size of the memory block (*block) */
    size_t            used;                                  /* Amount of memory used in the memory block */
    uchar             *block;                                /* Pointer to the actual memory block */
    struct _mpblock   *next;                                 /* Pointer to the next memory pool block */
} mpblock;

typedef struct _mp
{
    char              init;                                  /* Y/N whether the memory pool was initialized or not */
    char              descr[MP_MAX_DESCR_LEN];               /* Memory pool description */
    mpblock           *head_block;                           /* Pointer to the first memory pool block */
    mpblock           *tail_block;                           /* Pointer to the last memory pool block */
#if MP_THREAD_SAFE == 1
    MP_THREAD_T       thread_id;                             /* Thread ID */
#endif
} mp;

/* Prototypes */

void *mpmalloc(size_t size);
void *mpmalloc_mpid(size_t size, int mpid);
void *mpmemalign(size_t alignment, size_t size);
void *mpmemalign_mpid(size_t alignment, size_t size, int mpid);
void *mpcalloc(size_t nelem, size_t size);
void *mpcalloc_mpid(size_t nelem, size_t size, int mpid);
void mpfree(void *ptr);
void mpfree_mpid(void *ptr, int mpid);
void *mprealloc(void *ptr, size_t size);
void *mprealloc_mpid(void *ptr, size_t size, int mpid);
char *mpstrdup(const char *s1);
char *mpstrdup_mpid(const char *s1, int mpid);
int mpasprintf(char **strp, const char *fmt, ...);
int mpasprintf_mpid(char **strp, int mpid, const char *fmt, ...);

int mpnew(char *descr);
int mppush(int mpid);
int mppop();
int mpget();
int mpset(int mpid);
int mpdel(int mpid);
int mpdel_all();
int mpclr(int mpid);
void mpprn();
int mpdmp(char *filename);
void mpset_memlim(size_t size);
size_t mpget_memlim();
void mpset_blksz(size_t size);
size_t mpget_blksz();

char *mpstrerror();
int mptrc_set_fn(int (*function)(FILE *fd, char *fmt, va_list ap));

#endif /* } _MP_H_ */

/* EOF */
