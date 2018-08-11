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
|* Module: mp.c
|*
|* Description: Memory pool management
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20140801     JG              Initial version
|*
****************************************************************************/

/* Includes and defines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if _WIN32 /* { */
#   include <STDDEF.h>
#else /* } _WIN32 { */
#   include <stdint.h>
#endif /* } _WIN32 */
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <valgrind/valgrind.h>
#ifndef MP_VALGRIND_NOT_AVAILABLE /* { */
#   pragma message("Compiled with the Valgrind extension")
#   include <valgrind/memcheck.h>
#endif /* } MP_VALGRIND_NOT_AVAILABLE */
#include "mp.h"

/* Prototypes */
static void *mpget_chunk(size_t size, int mpid, size_t alignment);
static void *mpadd_block(size_t size, int mpid, size_t alignment);
static int mpadd_tot_phy_mem(size_t size2add, int sign);

/* Local variables */
#if MP_THREAD_SAFE == 1 /* { */
static MP_TLS_INT mp_cur_mpid = MP_DEF_MP_ID;               /* Current Memory Pool ID */
static MP_TLS_INT mp_prev_mpid = MP_NO_MP_ID;               /* Previous Memory Pool ID */
#    ifndef _WIN32
static MP_MUTEX_T mp_mutex = MP_MUTEX_INIT_VAL;             /* Mutex */
static MP_MUTEX_T *mp_mutex_p = &mp_mutex;                  /* Mutex Pointer */
#    else
static MP_MUTEX_T *mp_mutex_p = NULL;                       /* Mutex Pointer */
#    endif
#else /* } MP_THREAD_SAFE { */
static int mp_cur_mpid = MP_DEF_MP_ID;                      /* Current Memory Pool ID */
static int mp_prev_mpid = MP_NO_MP_ID;                      /* Previous Memory Pool ID */
#endif /* } */
static size_t mp_tot_phy_mem = 0;                           /* Total physical memory used */
static size_t volatile mp_mem_limit = 0;                    /* Memory limit */
static size_t volatile mp_blk_sz = MP_DEF_BLK_SZ;           /* Memory limit */

/* Global variables */
#if MP_THREAD_SAFE == 1
MP_TLS_INT mperrno = 0;
MP_TLS_CHAR *mperrstr = NULL;
#else
int  mperrno = 0;
char *mperrstr = NULL;
#endif

mp mp_arr[MP_MAX_MP_ID -1];                                 /* Array of memory pools */


/****************************************************************************
|*
|* Function: mpadd_block
|*
|* Description;
|*
|*     Adds a new block to our memory pool.
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
static void *mpadd_block(size_t size, int mpid, size_t alignment)
{
    void *chunk = NULL;
    size_t block_size = mpget_blksz() + alignment -1;
    mpblock *curr_block = NULL;
    mpblock *new_block = NULL;
    mperrno = MP_ERRNO_SUCCESS;

    /* Parameters check was done on mpget_chunk() */

    /* Check if the memory pool was initialized */
    if (mp_arr[mpid].init != 'Y')
    {
        if (mpid == MP_DEF_MP_ID)
        {
            mp_arr[mpid].init = 'Y';
            strncpy(mp_arr[mpid].descr, MP_DEF_MP_DESCR, sizeof(mp_arr[mpid].descr));
            mp_arr[mpid].descr[sizeof(mp_arr[mpid].descr) -1] = '\0';
#if MP_THREAD_SAFE == 1
            /* Threads should aquire a new Memory Pool, so we assume
             * The thread calling the default one is the main thread */
            mp_arr[mpid].thread_id = MP_CURR_THREAD;
#endif

#ifndef MP_VALGRIND_NOT_AVAILABLE
            VALGRIND_CREATE_MEMPOOL((void *)&mp_arr[mpid], 0, 1);
#endif
        }
        else
        {
            mperrno = MP_ERRNO_NOIN;
            return NULL;   
        }
    }

    /* Set memory limit */
    if (mp_mem_limit <= 0) /* Don't care about thread race */
    {
        if (sizeof(size_t) > 4)
        {
            mp_mem_limit = MP_DEF_MEM_LIMIT_64;
        }
        else
        {
            mp_mem_limit = MP_DEF_MEM_LIMIT_32;
        }
    }

    /* Creating new memory pool block */
    new_block = (mpblock *)malloc(sizeof(mpblock));
    if (new_block == NULL)
    {
        mperrno = MP_ERRNO_ALLO;
        return NULL;
    }
    memset(new_block, 0x00, sizeof(mpblock));
    new_block->block = NULL;
    new_block->next = NULL;

    /* Override default size if needed */
    if (size > mpget_blksz())
    {
        block_size = size + alignment -1;
    }

    /* Check memory limit */
    if ((mperrno = mpadd_tot_phy_mem(block_size, +1)) != MP_ERRNO_SUCCESS)
    {
        free(new_block);
        return NULL;
    }

    /* Setting new memory pool block info */
    new_block->block = (uchar *)malloc(block_size);
    if (new_block->block == NULL)
    {
        mperrno = MP_ERRNO_ALLO;
        return NULL;
    }
    new_block->size = block_size;
    new_block->used = alignment - ((uintptr_t)new_block->block % alignment);
    new_block->used = new_block->used == alignment ? 0 : new_block->used;
    chunk = new_block->block + new_block->used;
    new_block->used += size;
    new_block->next = NULL;
#ifndef MP_VALGRIND_NOT_AVAILABLE
    VALGRIND_MAKE_MEM_NOACCESS(new_block->block, new_block->size);
#endif

    /* Attach new memory block to our memory pool */
    if (mp_arr[mpid].head_block != NULL)
    {
        curr_block = mp_arr[mpid].tail_block;
        curr_block->next = new_block;
    }
    else
    {
        mp_arr[mpid].head_block = new_block;
    }
    mp_arr[mpid].tail_block = new_block;

    /* Deliver required chunk of memory */
    return chunk;

}

/****************************************************************************
|*
|* Function: mpget_chunk
|*
|* Description;
|*
|*     Gets a chunk of memory of the specified size from the memmory pool 
|*     with id mpid and aligned to alignment.
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
static void *mpget_chunk(size_t size, int mpid, size_t alignment)
{
    void *chunk = NULL;
    mp *curr_mp = NULL;
    mpblock *curr_block = NULL;
    mperrno = MP_ERRNO_SUCCESS;

    /* We decided not to support negative sizes */
    if (size < 0) /* Works only when size_t is signed */
    {
        mperrno = MP_ERRNO_SZNG;
        return NULL;   
    }

    /* Memory pool ID out of limit */
    if (mpid > MP_MAX_MP_ID -1 || mpid < 0)
    {
        mperrno = MP_ERRNO_MPID;
        return NULL;
    }

    /* Memory pool ID not initiliazed */
    if (mp_arr[mpid].init != 'Y' && mpid != MP_DEF_MP_ID)
    {
        mperrno = MP_ERRNO_NOIN;
        return NULL;
    }

    /* Alignment out of limits: Alignment is not power of 2 or is less than size of void * */
    if ((alignment & (alignment -1)) != 0 ||
            alignment < sizeof(void *))
    {
        mperrno = MP_ERRNO_EXAL;
        return NULL;
    }

    /* Setting right alignment */
    if (alignment < MP_DEF_ALIGN)
    {
        alignment = MP_DEF_ALIGN;
    }

    /* Not to fail if size is 0 we deliver a pointer anyway */
    if (size == 0)
    {
        /* TODO: In this case we should return some lost memory chunk */
        size = 1;
    }

    curr_mp = &mp_arr[mpid];
#if MP_THREAD_SAFE == 1
    /* Check thread ID */
    if (!(mpid == MP_DEF_MP_ID && curr_mp->init != 'Y'))
    {
        if (MP_THREAD_EQ(curr_mp->thread_id, MP_CURR_THREAD) == 0)
        {
            mperrno = MP_ERRNO_THRD;
            return NULL;
        }
    }
#endif

    /* Get memory chunk */
    if (curr_mp->head_block == NULL)
    {
        /* First time using this pool, creating block with right alignment */
        chunk = mpadd_block(size, mpid, alignment);
    }
    else
    {
        /* Block already in used */
        int margin = 0;

        /* Check if last memory block has enough space */
        curr_block = curr_mp->tail_block;
        margin = alignment - ((uintptr_t)(curr_block->block + curr_block->used) % alignment);
        margin = margin == alignment ? 0 : margin;
        if (curr_block->size > curr_block->used + margin + size)
        {
            curr_block->used += margin;
            chunk = curr_block->block + curr_block->used;
            curr_block->used += size;
        }

        if (chunk == NULL)
        {
            /* No space in last memory block, creating new memory block */
            chunk = mpadd_block(size, mpid, alignment);
        }

    }

#ifndef MP_VALGRIND_NOT_AVAILABLE
    VALGRIND_MEMPOOL_ALLOC((void *)&mp_arr[mpid], chunk, size);
#endif

    return chunk;
}

/****************************************************************************
|*
|* Function: mpmalloc
|*
|* Description;
|*
|*     Analog to malloc() but allocating a piece of memory from our private 
|*     pool on current pool id (selected with mpset()).
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mpmalloc(size_t size)
{
    return mpmalloc_mpid(size, mp_cur_mpid);
}

/****************************************************************************
|*
|* Function: mpmalloc_mpid
|*
|* Description;
|*
|*     Analog to malloc() but allocating a piece of memory from our private 
|*     pool on a certain pool id
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mpmalloc_mpid(size_t size, int mpid)
{
    return mpget_chunk(size, mpid, MP_DEF_ALIGN);
}

/****************************************************************************
|*
|* Function: mpmemalign
|*
|* Description;
|*
|*     Analog to memalign() but allocating a piece of memory from our private 
|*     pool on current pool id (selected with mpset()) and aligned to the 
|*     desired alignment.
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mpmemalign(size_t alignment, size_t size)
{
    return mpmemalign_mpid(alignment, size, mp_cur_mpid);
}

/****************************************************************************
|*
|* Function: mpmemalign_mpid
|*
|* Description;
|*
|*     Analog to malloc() but allocating a piece of memory from our private 
|*     pool on a certain pool id and aligned to the desired alignment.
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mpmemalign_mpid(size_t alignment, size_t size, int mpid)
{
    return mpget_chunk(size, mpid, alignment);
}

/****************************************************************************
|*
|* Function: mpcalloc
|*
|* Description;
|*
|*     Analog to calloc() but allocating a piece of memory from our private 
|*     pool on current pool id (selected with mpset()).
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mpcalloc(size_t nelem, size_t size)
{
    return mpcalloc_mpid(nelem, size, mp_cur_mpid);
}

/****************************************************************************
|*
|* Function: mpcalloc_mpid
|*
|* Description;
|*
|*     Analog to calloc() but allocating a piece of memory from our private 
|*     pool on a certain pool id
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mpcalloc_mpid(size_t nelem, size_t size, int mpid)
{
    /* Allocate */
    void *chunk =  mpget_chunk(size * nelem, mpid, MP_DEF_ALIGN);

    /* Initialize */
    if (chunk != NULL)
    {
        memset(chunk, 0x00, size * nelem);
    }

    return chunk;
}

/****************************************************************************
|*
|* Function: mpfree
|*
|* Description;
|*
|*     Analog to free() but it's a dummy function that doesn't do anything.
|*     The reason is that this memory pool is a dynamic memory pool allocated
|*     for which the single memory chunks cannot be free-ed, but only the 
|*     whole memory pool. So the reason of this funcion is for 3rd party software 
|*     enabling a custom memory management and on which the malloc(), realloc(),
|*     free(), etc. can be overriden with own functions.
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
void mpfree(void *ptr)
{
    mpfree_mpid(ptr, mp_cur_mpid);
}

/****************************************************************************
|*
|* Function: mpfree_mpid
|*
|* Description;
|*
|*     Analog to free() but it's a dummy function that doesn't do anything.
|*     The reason is that this memory pool is a dynamic memory pool allocated
|*     for which the single memory chunks cannot be free-ed, but only the 
|*     whole memory pool. So the reason of this funcion is for 3rd party software 
|*     enabling a custom memory management and on which the malloc(), realloc(),
|*     free(), etc. can be overriden with own functions.
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
void mpfree_mpid(void *ptr, int mpid)
{
#ifndef MP_VALGRIND_NOT_AVAILABLE
    VALGRIND_MEMPOOL_FREE((void *)&mp_arr[mpid], ptr);
#endif
    return;
}

/****************************************************************************
|*
|* Function: mprealloc
|*
|* Description;
|*
|*     Analog to realloc() but allocating a piece of memory from our private 
|*     pool on current pool id (selected with mpset()).
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool and with the content of ptr if available.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mprealloc(void *ptr, size_t size)
{
    return mprealloc_mpid(ptr, size, mp_cur_mpid);
}

/****************************************************************************
|*
|* Function: mprealloc_mpid
|*
|* Description;
|*
|*     Analog to realloc() but allocating a piece of memory from our private 
|*     pool on a certain pool id
|*
|* Return:
|*     a pointer to a block of memory of the required size within our memory
|*     pool and with the content of ptr if available.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
void *mprealloc_mpid(void *ptr, size_t size, int mpid)
{
    void *chunk = NULL;
    uintptr_t ptrdiff = 0;

    /* Allocate new chunk of memory */
    chunk = mpget_chunk(size, mpid, MP_DEF_ALIGN);

    /* Copy content of all memory chunk */
    if (ptr != NULL && chunk != NULL)
    {
        /* Valgrind can yell here invalid or initiliazed read. 
         * At this point we don't know the size of previous pointer, so 
         * we need to copy as much as possible, therefore part of it
         * might not be initiliazed */

        /* Not to overlap */
        ptrdiff = (uintptr_t)chunk - (uintptr_t)ptr;

#ifndef MP_VALGRIND_NOT_AVAILABLE
        VALGRIND_DISABLE_ERROR_REPORTING;
#endif

        if (ptrdiff < size)
        {
            memcpy(chunk, ptr, ptrdiff);
        }
        else
        {
            memcpy(chunk, ptr, size);
        }

#ifndef MP_VALGRIND_NOT_AVAILABLE
        VALGRIND_ENABLE_ERROR_REPORTING;
#endif
    }

    return chunk;
}

/****************************************************************************
|*
|* Function: mpstrdup
|*
|* Description;
|*
|*     Returns a pointer to a new string that is a duplicate of s1.
|*
|* Return:
|*     a pointer to duplicated string.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
char *mpstrdup(const char *s1)
{
    return mpstrdup_mpid(s1, mp_cur_mpid);
}

/****************************************************************************
|*
|* Function: mpstrdup_mpid
|*
|* Description;
|*
|*     Returns a pointer to a new string that is a duplicate of s1.
|*
|* Return:
|*     a pointer to duplicated string or NULL if s1 is NULL.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
char *mpstrdup_mpid(const char *s1, int mpid)
{
    char *str = NULL;
    size_t str_len = 0;

    /* Check parameters */
    if (s1 == NULL)
    {
        return NULL;
    }

    str_len = strlen(s1) +1;
    /* Allocate memory for new string */
    str =  (char *)mpmalloc_mpid(str_len, mpid);

    /* Duplicate string in allocated memory */
    if (str != NULL)
    {
        strncpy(str, s1, str_len);
    }

    return str;
}

/****************************************************************************
|*
|* Function: mpasprintf_mpid
|*
|* Description;
|*
|*     anolog to sprintf() except that allocates a string large enough to hold
|*     the output. Same functionality of asprintf().
|*
|* Return:
|*     The number of bytes allocated on success
|*     -1 on error and the content of strp is undefined on error.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140815    JG    Initial version
|*
****************************************************************************/
int mpasprintf(char **strp, const char *fmt, ...)
{
    int rc = 0;
    int len = 0;
    va_list ap;

    /* Check parameters */
    if (fmt == NULL || strp == NULL)
    {
        mperrno = MP_ERRNO_PARM;
        return -1;
    }

    /* Find out actual length of the built string */
    va_start (ap, fmt);
#ifdef _WIN32
    len = _vscprintf(fmt, ap) + 1;
#else
    len = vsnprintf(NULL, 0, fmt, ap) + 1;
#endif
    va_end(ap);

    /* Deliver string */
    if (len > 0)
    {
        *strp = (char *)mpmalloc(len);
        if (*strp != NULL)
        {
            va_start (ap, fmt);
            rc = vsprintf(*strp, fmt, ap);
            va_end(ap);
        }
    }

    return rc;
}

/****************************************************************************
|*
|* Function: mpasprintf_mpid
|*
|* Description;
|*
|*     anolog to sprintf() except that allocates a string large enough to hold
|*     the output.
|*
|* Return:
|*     The number of bytes allocated on success
|*     -1 on error and the content of strp is undefined on error.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140815    JG    Initial version
|*
****************************************************************************/
int mpasprintf_mpid(char **strp, int mpid, const char *fmt, ...)
{
    int rc = 0;
    int len = 0;
    va_list ap;

    /* Check parameters */
    if (fmt == NULL || strp == NULL)
    {
        mperrno = MP_ERRNO_PARM;
        return -1;
    }

    /* Find out actual length of the built string */
    va_start (ap, fmt);
#ifdef _WIN32
    len = _vscprintf(fmt, ap) + 1;
#else
    len = vsnprintf(NULL, 0, fmt, ap) + 1;
#endif
    va_end(ap);

    /* Deliver string */
    if (len > 0)
    {
        *strp = (char *)mpmalloc_mpid(len, mpid);
        if (*strp != NULL)
        {
            va_start (ap, fmt);
            rc = vsprintf(*strp, fmt, ap);
            va_end(ap);
        }
    }

    return rc;
}

/****************************************************************************
|*
|* Function: mpnew
|*
|* Description;
|*
|*     Initializes the next memory pool available in the array of memory pools 
|*     and assignes the corresponding description
|*
|* Return:
|*     the memory pool ID of the new allocated memory pool    
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mpnew(char *descr)
{
    int i = 0, mpid = MP_ERRNO_EXMP;
    char *curr_descr = NULL;
    mperrno = MP_ERRNO_SUCCESS;

    /* Check parameter */
    if (descr == NULL)
    {
        curr_descr = "-";
    }
    else
    {
        curr_descr = descr;
    }

#if MP_THREAD_SAFE == 1 /* { */

#   if _WIN32 /* { */
    if ((mperrno = mp_mutex_init(&mp_mutex_p)) != MP_ERRNO_SUCCESS)
    {
        return mperrno;
    }
#   endif /* } _WIN32 */

    MP_MUTEX_LOCK(mp_mutex_p); /* No UT but checked with helgrind */
#endif /* } MP_THREAD_SAFE */
    /* Find next available memory pool */
    for(i = 0; i < MP_MAX_MP_ID; i++)
    {
        if (mp_arr[i].init != 'Y')
        {
            if (i == MP_DEF_MP_ID)
            { 
                continue;
            }
            mp_arr[i].init = 'Y';
            strncpy(mp_arr[i].descr, curr_descr, sizeof(mp_arr[i].descr));
            mp_arr[i].descr[sizeof(mp_arr[i].descr) -1] = '\0';
#if MP_THREAD_SAFE == 1
            mp_arr[i].thread_id = MP_CURR_THREAD;
#endif
            mp_arr[i].head_block = NULL;
            mp_arr[i].tail_block = NULL;
            mpid = i;

#ifndef MP_VALGRIND_NOT_AVAILABLE
            VALGRIND_CREATE_MEMPOOL((void *)&mp_arr[mpid], 0, 1);
#endif
            break;
        }
    }
#if MP_THREAD_SAFE == 1
    MP_MUTEX_UNLOCK(mp_mutex_p);
#endif

    mperrno = mpid == MP_ERRNO_EXMP ? MP_ERRNO_EXMP : mperrno;
    return mpid;
}

/****************************************************************************
|*
|* Function: mppush
|*
|* Description;
|*
|*     Moves the current memory pool ID to the stack and makes mpid the 
|*     current one. The stack has only one element, for now.
|*
|* Return:
|*     MP_ERRNO_SUCCESS if success
|*     MP_ERRNO_MPID, MP_ERRNO_NOIN if error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mppush(int mpid)
{
    mperrno = MP_ERRNO_SUCCESS;

    /* Memory pool ID out of limit */
    if (mpid > MP_MAX_MP_ID -1 || mpid < 0)
    {
        mperrno = MP_ERRNO_MPID;
        return MP_ERRNO_MPID;
    }

    /* Memory pool ID not initiliazed */
    if (mp_arr[mpid].init != 'Y' && mpid != MP_DEF_MP_ID)
    {
        mperrno = MP_ERRNO_NOIN;
        return MP_ERRNO_NOIN;
    }

    /* Push */
    mp_prev_mpid = mp_cur_mpid;
    mp_cur_mpid = mpid;

    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mppop
|*
|* Description;
|*
|*     Sets the previous Memory Pool ID as the current one
|*
|* Return:
|*     MP_ERRNO_SUCCESS if success
|*     MP_ERRNO_NOPP if error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mppop()
{
    mperrno = MP_ERRNO_SUCCESS;

    /* Nothing to pop */
    if (mp_prev_mpid < 0 || mp_prev_mpid > MP_MAX_MP_ID -1)
    {
        mperrno = MP_ERRNO_NOPP;
        return MP_ERRNO_NOPP;
    }

    /* Pop */
    mp_cur_mpid = mp_prev_mpid;
    mp_prev_mpid = MP_NO_MP_ID;

    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mpset
|*
|* Description;
|*
|*     Sets the current memory pool ID
|*
|* Return:
|*     MP_ERRNO_SUCCESS if success
|*     MP_ERRNO_MPID, MP_ERRNO_NOIN, MP_ERRNO_THRD if error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mpset(int mpid)
{
    mperrno = MP_ERRNO_SUCCESS;

    /* Memory pool ID out of limit */
    if (mpid > MP_MAX_MP_ID -1 || mpid < 0)
    {
        mperrno = MP_ERRNO_MPID;
        return MP_ERRNO_MPID;
    }

    /* Memory pool ID not initiliazed */
    if (mp_arr[mpid].init != 'Y' && mpid != MP_DEF_MP_ID)
    {
        mperrno = MP_ERRNO_NOIN;
        return MP_ERRNO_NOIN;
    }

#if MP_THREAD_SAFE == 1
    /* Check thread ID */
    if (MP_THREAD_EQ(mp_arr[mpid].thread_id, MP_CURR_THREAD) == 0)
    {
        mperrno = MP_ERRNO_THRD;
        return MP_ERRNO_THRD;
    }
#endif

    /* Set */
    mp_cur_mpid = mpid;

    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mpget
|*
|* Description;
|*
|*     Gets the current memory pool ID
|*
|* Return:
|*     the current memory pool ID
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mpget()
{
    return mp_cur_mpid;
}

/****************************************************************************
|*
|* Function: mpdel
|*
|* Description;
|*
|*     Deletes a given memory pool
|*
|* Return:
|*     MP_ERRNO_SUCCESS if success
|*     MP_ERRNO_MPID, MP_ERRNO_THRD if error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mpdel(int mpid)
{
    mpblock *curr_block = NULL;
    mpblock *temp_block = NULL;

    mperrno = MP_ERRNO_SUCCESS;

    /* Memory pool ID out of limit */
    if (mpid > MP_MAX_MP_ID -1 || mpid < 0)
    {
        mperrno = MP_ERRNO_MPID;
        return MP_ERRNO_MPID;
    }

#if MP_THREAD_SAFE == 1
    /* Check thread ID */
    if (MP_THREAD_EQ(mp_arr[mpid].thread_id, MP_CURR_THREAD) == 0)
    {
        mperrno = MP_ERRNO_THRD;
        return MP_ERRNO_THRD;
    }
#endif

    /* Deallocating all memory blocks in given memory pool */
    curr_block = mp_arr[mpid].head_block;
    while(curr_block != NULL)
    {
        if (curr_block->block != NULL)
        {
            free(curr_block->block);
        }
        mpadd_tot_phy_mem(curr_block->size, -1); /* No need to check for error */
        temp_block = curr_block;
        curr_block = curr_block->next;
        free(temp_block);
    }

#ifndef MP_VALGRIND_NOT_AVAILABLE
    // VVALGRIND_MEMPOOL_TRIM((void *)&mp_arr[mpid], curr_block->block, 0);
    VALGRIND_DESTROY_MEMPOOL((void *)&mp_arr[mpid]);
#endif
    memset(&mp_arr[mpid], 0x00, sizeof(mp));
    mp_arr[mpid].head_block = NULL;
    mp_arr[mpid].tail_block = NULL;

    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mpdel_all
|*
|* Description;
|*
|*     Deletes all memory pools. This should be called by the main thread.
|*
|* Return:
|*     MP_ERRNO_SUCCESS
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mpdel_all()
{
    mpblock *curr_block = NULL;
    mpblock *temp_block = NULL;
    int i = 0;

    for (i = 0; i < MP_MAX_MP_ID; i ++)
    {
        if (mp_arr[i].init != 'Y')
            continue;

        curr_block = mp_arr[i].head_block;
        while(curr_block != NULL)
        {
            if (curr_block->block != NULL)
            {
                free(curr_block->block);
            }
            temp_block = curr_block;
            curr_block = curr_block->next;
            free(temp_block);
        }

#ifndef MP_VALGRIND_NOT_AVAILABLE
        // VVALGRIND_MEMPOOL_TRIM((void *)&mp_arr[i], curr_block->block, 0);
        VALGRIND_DESTROY_MEMPOOL((void *)&mp_arr[i]);
#endif
        memset(&mp_arr[i], 0x00, sizeof(mp));
        mp_arr[i].head_block = NULL;
        mp_arr[i].tail_block = NULL;
    }

    mp_tot_phy_mem = 0;

    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mpclr
|*
|* Description;
|*
|*     Clears to 0 all memory used in the given memory pool.
|*
|* Return:
|*     MP_ERRNO_SUCCESS if success
|*     MP_ERRNO_MPID, MP_ERRNO_THRD if error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
int mpclr(int mpid)
{
    mpblock *curr_block = NULL;

    mperrno = MP_ERRNO_SUCCESS;

    /* Memory pool ID out of limit */
    if (mpid > MP_MAX_MP_ID -1 || mpid < 0)
    {
        mperrno = MP_ERRNO_MPID;
        return MP_ERRNO_MPID;
    }

#if MP_THREAD_SAFE == 1
    /* Check thread ID */
    if (MP_THREAD_EQ(mp_arr[mpid].thread_id, MP_CURR_THREAD) == 0)
    {
        mperrno = MP_ERRNO_THRD;
        return MP_ERRNO_THRD;
    }
#endif

    /* Set the used parameter of each memory block to zero */
    curr_block = mp_arr[mpid].head_block;
    while(curr_block != NULL)
    {
        curr_block->used = 0;
        curr_block = curr_block->next;
    }

#ifndef MP_VALGRIND_NOT_AVAILABLE
    // VALGRIND_MEMPOOL_TRIM((void *)&mp_arr[mpid], curr_block->block, 0);
#endif

    return MP_ERRNO_SUCCESS;
}

/****************************************************************************
|*
|* Function: mpset_memlim
|*
|* Description;
|*
|*     Sets the memory limit
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
void mpset_memlim(size_t size)
{
    mp_mem_limit = size;
    return;
}

/****************************************************************************
|*
|* Function: mpget_memlim
|*
|* Description;
|*
|*     Gets the current memory limit
|*
|* Return:
|*     memory limit
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140813    JG    Initial version
|*
****************************************************************************/
size_t mpget_memlim()
{
    return mp_mem_limit;
}

/****************************************************************************
|*
|* Function: mpset_blksz
|*
|* Description;
|*
|*     Sets the memory block size
|*
|* Return:
|*     n/a
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20150103    JG    Initial version
|*
****************************************************************************/
void mpset_blksz(size_t size)
{
    mp_blk_sz = size;
    return;
}

/****************************************************************************
|*
|* Function: mpget_blksz
|*
|* Description;
|*
|*     Gets the current memory limit
|*
|* Return:
|*     memory limit
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20150103    JG    Initial version
|*
****************************************************************************/
size_t mpget_blksz()
{
    return mp_blk_sz;
}

/****************************************************************************
|*
|* Function: mpstrerror
|*
|* Description;
|*
|*     Delivers the current error message string
|*
|* Return:
|*     error message string
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20140811    JG    Initial version
|*
****************************************************************************/
char *mpstrerror()
{
    switch (mperrno)
    {
        case MP_ERRNO_SUCCESS:
            return "";
        case MP_ERRNO_MPID:
            return MP_ERRSTR_MPID;
        case MP_ERRNO_SZNG:
            return MP_ERRSTR_SZNG;
        case MP_ERRNO_NOMM:
            return MP_ERRSTR_NOMM;
        case MP_ERRNO_EXMM:
            return MP_ERRSTR_EXMM;
        case MP_ERRNO_ALLO:
            return MP_ERRSTR_ALLO;
        case MP_ERRNO_EXAL:
            return MP_ERRSTR_EXAL;
        case MP_ERRNO_NOIN:
            return MP_ERRSTR_NOIN;
        case MP_ERRNO_EXMP:
            return MP_ERRSTR_EXMP;
        case MP_ERRNO_NOPP:
            return MP_ERRSTR_NOPP;
        case MP_ERRNO_DISP:
            return MP_ERRSTR_DISP;
        case MP_ERRNO_PARM:
            return MP_ERRSTR_PARM;
        case MP_ERRNO_THRD:
            return MP_ERRSTR_THRD;
        case MP_ERRNO_SYSE:
            return strerror(errno); // TODO replace with reentrant
    }

    return "Error number not recognized";
}

/****************************************************************************
|*
|* Function: mpadd_tot_phy_mem
|*
|* Description;
|*
|*     Atomic function to modify the current total physical memory used
|*
|* Return:
|*     MP_ERRNO_SUCCESS if success
|*     MP_ERRNO_EXMM if error
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20150101    JG    Initial version
|*
****************************************************************************/
static int mpadd_tot_phy_mem(size_t size2add, int sign)
{
#if MP_THREAD_SAFE == 1 /* { */

#   if _WIN32 /* { */
    if ((mperrno = mp_mutex_init(&mp_mutex_p)) != MP_ERRNO_SUCCESS)
    {
        return mperrno;
    }
#   endif /* } _WIN32 */

	MP_MUTEX_LOCK(mp_mutex_p);
#endif /* } MP_THREAD_SAFE */

    if (sign >= 0 && mp_tot_phy_mem + size2add > mp_mem_limit)
    {
        mperrno = MP_ERRNO_EXMM;
        return MP_ERRNO_EXMM;
    }

    mp_tot_phy_mem += (size2add * ((sign >= 0) - (sign < 0)));

#if MP_THREAD_SAFE == 1
    MP_MUTEX_UNLOCK(mp_mutex_p);
#endif

    return MP_ERRNO_SUCCESS;
}
/* EOF */
