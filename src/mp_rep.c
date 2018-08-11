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
|* Module: mp_rep.c
|*
|* Description: 
|*
|*  This module contains the functions for statistics reporting and
|*  dumping the content of the whole memory pools into a file
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
#ifndef MP_VALGRIND_NOT_AVAILABLE
#   include <valgrind/valgrind.h>
#endif

#include "mp.h"
#include "mp_trc.h"

/* Prototypes */
static char *mpsz2rnd(char *sizestr, size_t size);
static size_t mppow(int x, int y);
static char *mpbin2hex(char *hex, size_t hex_sz, size_t alignment, void* buff, size_t sz);

/* Structs */
typedef struct _mpstat_t
{
    char mpid[5];                                           /* Memory Pool ID */
    char descr[16];                                         /* Description */
    char blocks[9];                                         /* Number of blocks */
    char size[11];                                          /* Memory Pool size in bytes */
    char used[11];                                          /* Number of bytes used */
    char used_prc[9];                                       /* Percentage of number of bytes used */
    char free[11];                                          /* Number of bytes free */
    char free_prc[7];                                       /* Percentage of number of bytes free */
    char eol;                                               /* End of line (0x00)*/
} mpstat_t;

/* Global variables */
extern mp mp_arr[MP_MAX_MP_ID -1];                          /* Array of memory pools */
#if MP_THREAD_SAFE == 1 /* { */
extern MP_TLS_INT mperrno;
#else /* } MP_THREAD_SAFE { */
extern int mperrno;
#endif /* } MP_THREAD_SAFE */


/****************************************************************************
|*
|* Function: mpprn
|*
|* Description;
|*
|*     Displays the statistics of all the memory pools on screen or device 
|*     specified by the fucntion (*mptrc_fn)
|*
|* Return:
|*     n/a
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void mpprn()
{
    int block_no = 0;
    int i = 0;
    mpblock *curr_block = NULL;
    size_t size = 0;
    size_t used = 0;
    int tot_block_no = 0;
    size_t tot_size = 0;
    size_t tot_used = 0;
    char sizestr[16] = "";

    mpstat_t stat_rec;

    memset(&stat_rec, 0x00, sizeof(stat_rec));

    /* Header  */
    mptrc(NULL, "%s", "==============================================================================");
    mptrc(NULL, "%s", "MPID Descr           Blocks   Size       Used       %        Free       %");
    mptrc(NULL, "%s", "------------------------------------------------------------------------------");

    /* Loop all memory pools */
    for (i = 0; i < MP_MAX_MP_ID; i ++)
    {

        /* Ingore memory pool not initiliazed */
        if (mp_arr[i].init != 'Y')
        {
            continue;
        }

        block_no = 0;
        size = 0;
        used = 0;

        /* Gather info from all the memory blocks */
        curr_block = mp_arr[i].head_block;
        while(curr_block != NULL)
        {
            block_no++;
            size += curr_block->size;
            used += curr_block->used;

            curr_block = curr_block->next;
        }

        tot_block_no += block_no;
        tot_size += size;
        tot_used += used;

        /* Print out memory pool info details */
        memset(&stat_rec, 0x00, sizeof(stat_rec));

        sprintf(stat_rec.mpid       , "%*d ",     (int) sizeof(stat_rec.mpid    ) -1,    i);
        sprintf(stat_rec.descr      , "%-*.*s",   (int) sizeof(stat_rec.descr   )   , (int) sizeof(stat_rec.descr),  mp_arr[i].descr);
        sprintf(stat_rec.blocks     , "%-*d",     (int) sizeof(stat_rec.blocks  )   ,    block_no);
        sprintf(stat_rec.size       , "%-*s",     (int) sizeof(stat_rec.size    )   ,    mpsz2rnd(sizestr, size));
        sprintf(stat_rec.used       , "%-*s",     (int) sizeof(stat_rec.used    )   ,    mpsz2rnd(sizestr, used));
        sprintf(stat_rec.used_prc   , "%%%-*.*f", (int) sizeof(stat_rec.used_prc)   , 2, !size ? 0.0 : (double)(used/(long double)size) * 100);
        sprintf(stat_rec.free       , "%-*s",     (int) sizeof(stat_rec.free    )   ,    mpsz2rnd(sizestr, size - used));
        sprintf(stat_rec.free_prc   , "%%%-*.*f", (int) sizeof(stat_rec.free_prc) -1, 2, !size ? 0.0 : (double)((size - used)/(long double)size) * 100);

        mptrc(NULL, "%s", (char *)&stat_rec);
    }

    mptrc(NULL, "%s", "------------------------------------------------------------------------------");

    /* Print out totals */
    memset(&stat_rec, 0x00, sizeof(stat_rec));

    sprintf(stat_rec.mpid       , "%-*s",     (int) sizeof(stat_rec.mpid    )   ,    "Total");
    sprintf(stat_rec.descr      , "%-*s",     (int) sizeof(stat_rec.descr   )   ,    "");
    sprintf(stat_rec.blocks     , "%-*d",     (int) sizeof(stat_rec.blocks  )   ,    tot_block_no);
    sprintf(stat_rec.size       , "%-*s",     (int) sizeof(stat_rec.size    )   ,    mpsz2rnd(sizestr, tot_size));
    sprintf(stat_rec.used       , "%-*s",     (int) sizeof(stat_rec.used    )   ,    mpsz2rnd(sizestr, tot_used));
    sprintf(stat_rec.used_prc   , "%%%-*.*f", (int) sizeof(stat_rec.used_prc)   , 2, !tot_size ? 0.0 : (double)(tot_used/(long double)tot_size) * 100);
    sprintf(stat_rec.free       , "%-*s",     (int) sizeof(stat_rec.free    )   ,    mpsz2rnd(sizestr, tot_size - tot_used));
    sprintf(stat_rec.free_prc   , "%%%-*.*f", (int) sizeof(stat_rec.free_prc) -1, 2, !tot_size ? 0.0 : (double)((tot_size - tot_used)/(long double)tot_size) * 100);

    mptrc(NULL, "%s", (char *)&stat_rec);

    mptrc(NULL, "%s", "==============================================================================");

    return;
}

/****************************************************************************
|*
|* Function: mpdmp
|*
|* Description;
|*
|*     dumps to a text file the content of the memeory of all pools. If 
|*     filenames doesn't have path, it is created in current folder.
|*
|* Return:
|*     MP_ERRNO_SUCCESS on success
|*     MP_ERRNO_PARM, MP_ERRNO_SYSE on error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140814    JG    Initial version
|*
****************************************************************************/
#define MP_LINE_HEX_MAX_LEN   128
#define MP_DMP_SKIP_ZERO_BYTES 1
int mpdmp(char *filename)
{
    FILE *fd = NULL;
    char date[24] = "";
    mpblock *curr_block = NULL;
    int i = 0;
    long long j = 0;
    int margin = 0;
    const size_t alignment = 16;
    int blkno = 0;
    char sizestr[16] = "";
    char hex[MP_LINE_HEX_MAX_LEN] = "";
    int skip = 0;
    int bl_zero_b = 0;
    int skip_zero_bytes = MP_DMP_SKIP_ZERO_BYTES;

    mperrno = MP_ERRNO_SUCCESS;
    
	if (filename == NULL)
    {
        mperrno = MP_ERRNO_PARM;
        return MP_ERRNO_PARM;
    }

    /* Open report file */
    if ((fd = fopen(filename, "wt")) == NULL)
    {
        mperrno = MP_ERRNO_SYSE;
        return MP_ERRNO_SYSE;
    }

    /* Current date */
    {
        time_t t = time(0);
        struct tm *tm = localtime(&t);
        strftime(date, sizeof(date), "%Y/%m/%d %H:%M:%S", tm);
    }

    /* Report header */
    mptrc(fd, "==================================================================================");
    mptrc(fd, "Memory pool dump (Report of memory used)                      %s", date);

    /* Loop all memory pools */
    for (i = 0; i < MP_MAX_MP_ID; i ++)
    {

        /* Ignore memory pool not initiliazed */
        if (mp_arr[i].init != 'Y')
        {
            continue;
        }

        /* Header for the memory pool */
        mptrc(fd, "----------------------------------------------------------------------------------");
        mptrc(fd, "Memory pool: %s (ID: %d)", mp_arr[i].descr, i);
        mptrc(fd, "----------------------------------------------------------------------------------");

        /* Loop all memory pool blocks */
        blkno = 1;
        curr_block = mp_arr[i].head_block;
        while(curr_block != NULL)
        {

            uchar *block = curr_block->block;
            size_t size = curr_block->used;
            margin = (int)((uintptr_t)block % alignment);

            /* Header for the memory pool block */
            mptrc(fd, "----------------------------------------------------------------------------------");
            mptrc(fd, "Block number: %d size: %s from: %p to %p", blkno, mpsz2rnd(sizestr, size), block, block + size -1); 
            mptrc(fd, "----------------------------------------------------------------------------------");

            /* Loop our memory block to print certain amount of bytes per line */
            for (j = 0 - margin; j < size; j += alignment)
            {
                memset(hex, 0x00, sizeof(hex)); /* Hex is 128, so this won't work for align bigger than that */
#ifndef MP_VALGRIND_NOT_AVAILABLE
                VALGRIND_DISABLE_ERROR_REPORTING;
#endif
                bl_zero_b = memcmp(hex, block + j, alignment) == 0; /* warning: align can't be bigger than hex */
#ifndef MP_VALGRIND_NOT_AVAILABLE
                VALGRIND_ENABLE_ERROR_REPORTING;
#endif
                if (
                        skip_zero_bytes &&
                        j >= alignment && 
                        bl_zero_b &&
                        j < size - alignment
                   )
                {
                    if (skip != 1)
                    {
                        mptrc(fd, "(skipped zero bytes...)");
                        skip = 1;
                    }
                }
                else
                {
                    mptrc(fd, "%0*p: %s", 
                            16, 
                            block + j, 
                            mpbin2hex(hex, sizeof(hex), alignment, block + (size_t)(j < 0 ? 0 : j), size - (size_t)(j < 0 ? 0 : j))
                            );
                    skip = 0;
                }
            }

            blkno++;
            curr_block = curr_block->next;
        }

    }

    /* End of report */
    mptrc(fd, "==================================================================================");

    /* Close report */
    fflush(fd);
    fclose(fd);

    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mpbin2hex
|*
|* Description;
|*
|*     Converts the memory content from a certain pointer to hexadecimal
|*     and human readable format.
|*
|* Return:
|*     the string containing the hexadecimal values
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140814    JG    Initial version
|*
****************************************************************************/
static char *mpbin2hex(char *hex, size_t hex_sz, size_t alignment, void* buff, size_t sz)
{
    char asc[MP_LINE_HEX_MAX_LEN] = "";
    int i = 0, idx = 0, i_align = (int)alignment;

    int margin = 0;

    if (alignment <= 0)
    {
        return "";
    }

    if (sz > alignment)
    {
        sz = alignment;
    }

    margin = (int)((uintptr_t)buff % alignment);

    for (i = 0; i < i_align; i++)
    {
        idx = i - margin;
        if (i < margin || (size_t)idx > sz -1)
        {
            sprintf(hex + (i * 3), "   ");
            sprintf(asc + i, " ");
        }
        else
        {
            /* Following lines might raise a complain from some memory analyzers such as valgrind.
             * The complain is about showing an unitiliazed value with printf(), but that is what
             * is all about with a memory dumpper, to show all the memory raw as it is, no matter 
             * if it is initialized or not. */
#ifndef MP_VALGRIND_NOT_AVAILABLE
            VALGRIND_DISABLE_ERROR_REPORTING;
#endif

            sprintf(hex + (i * 3), "%02x ", ((uchar *)buff)[idx]);
            sprintf(asc + i, "%c", isprint(((uchar *)buff)[idx]) ? ((uchar *)buff)[idx] : '.');

#ifndef MP_VALGRIND_NOT_AVAILABLE
            VALGRIND_ENABLE_ERROR_REPORTING;
#endif
        }
    }

    strncpy(hex + strlen(hex), asc, hex_sz - strlen(hex));
    hex[hex_sz -1] = '\0';
    //snprintf(hex + strlen(hex), hex_sz - strlen(hex), "%s", asc);

    return hex;
}

/****************************************************************************
|*
|* Function: mpsz2rnd
|*
|* Description;
|*
|*     Rounds sizes up in bytes to the corresponding values in Kb, Mb, Gb,
|*     Tb or Pb.
|*
|* Return:
|*     return a string with the rounded size.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140812    JG    Initial version
|*
****************************************************************************/
static char *mpsz2rnd(char *sizestr, size_t size)
{
    int i = 0;
    size_t b  = 1;
    size_t kb = 1024 * b;
    size_t mb = 1024 * kb;
    size_t gb = 1024 * mb;
    size_t tb = 1024 * gb;
    size_t pb = 1024 * tb;

    struct {
        int      power;
        char     fmt[3];
        size_t   div;   
    } powinfo[] = {
        {1, "b",   b},
        {2, "Kb", kb},
        {3, "Mb", mb},
        {4, "Gb", gb},
        {5, "Tb", tb},
        {6, "Pb", pb},
        {-1, "", 0}
    };

    if (sizestr == NULL)
    {
        return sizestr;
    }

    while(powinfo[i].power > 0)
    {
        if (size < mppow(1000, powinfo[i].power))
        {
            sprintf(sizestr, "%.*f%s", 1, (double)size/powinfo[i].div, powinfo[i].fmt);
            break;
        }
        i++;
    }

    return sizestr;
}

/****************************************************************************
|*
|* Function: mppow
|*
|* Description;
|*
|*     Computes the power of two numers, this is meant to replace the pow()
|*     function not to need the m library. Also our function accepts int
|*     numbers but it supports only postive exponent (y)
|*
|* Return:
|*     the power of x and y
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140812    JG    Initial version
|*
****************************************************************************/
static size_t mppow(int x, int y)
{
    size_t pow = 1;
    while(y && y > 0) {pow *= x; y--;};
    return pow;
}

/* EOF */
