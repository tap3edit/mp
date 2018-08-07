/****************************************************************************
|*
|* tap3edit Tools (http://www.tap3edit.com)
|*
|* $Id: mp.h 54 2014-10-11 19:51:18Z mrjones $
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
|* Module: mp_os.h
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20141013     JG              Initial version
|*
****************************************************************************/

#ifndef _MP_OS_H_ /* { */
#define _MP_OS_H_

/* Includes */

#ifdef MP_THREAD_SAFE /* { */
#   ifndef WIN32 /* { */
#       include <pthread.h>
#   else /* } WIN32 { */
#       include <windows.h>
#   endif /* } WIN32 */
#endif /* } */

/* Defines */

#ifdef MP_THREAD_SAFE /* { */
#   ifndef WIN32 /* { */
#       define MP_MUTEX_T           pthread_mutex_t                     /* Mutex datatype */
#       define MP_MUTEX_INIT(a)     pthread_mutex_init(a, NULL)         /* Mutex init */
#       define MP_MUTEX_INIT_VAL    PTHREAD_MUTEX_INITIALIZER           /* Mutex initial value */
#       define MP_MUTEX_DSTRY(a)    pthread_mutex_destroy(a)            /* Mutex destroy */
#       define MP_MUTEX_LOCK(a)     pthread_mutex_lock(a)               /* Mutex lock */
#       define MP_MUTEX_UNLOCK(a)   pthread_mutex_unlock(a)             /* Mutex unlock */
#       define MP_THREAD_T          pthread_t                           /* Thread datatype */
#       define MP_TLS_INT           __thread int                        /* TLS int */
#       define MP_CURR_THREAD       pthread_self()                      /* Returns thread Id */
#       define MP_THREAD_EQ(a,b)    pthread_equal(a, b)                 /* Returns zero if two threads are equal */
#   else /* } WIN32 { */
#       define MP_MUTEX_T           CRITICAL_SECTION
#       define MP_MUTEX_INIT(a)     (!InitializeCriticalSection(a)
#       define MP_MUTEX_INIT_VAL    {(void *)-1, -1, 0, 0, 0, 0}        /* Mutex initial value */
#       define MP_MUTEX_DSTRY(a)    DeleteCriticalSection(a)
#       define MP_MUTEX_LOCK(a)     EnterCriticalSection(a)             /* Mutex lock */
#       define MP_MUTEX_UNLOCK(a)   LeaveCriticalSection(a)             /* Mutex unlock */
#       define MP_THREAD_T          long
#       define MP_TLS_INT           __declspec(thread) int
#       define MP_CURR_THREAD       ((long)GetCurrentThreadId())
#       define MP_THREAD_EQ(a,b)    (a == b)
#   endif /* } WIN32 */
#else /* } MP_THREAD_SAFE { */
#       define MP_MUTEX_T           /* TODO */
#       define MP_MUTEX_INIT(a)     /* TODO */
#       define MP_MUTEX_DSTRY(a)    /* TODO */
#       define MP_THREAD_T          char
#       define MP_TLS_INT           int
#       define MP_CURR_THREAD       ((char)'\0')
#       define MP_THREAD_EQ(a,b)    (a == b)
#endif /* } MP_THREAD_SAFE */

/* Prototypes */

#if MP_THREAD_SAFE == 1 /* { */
int mp_mutex_init(MP_MUTEX_T **mutex_p);
#endif

#endif /* } _MP_OS_H_ */
/* EOF */
