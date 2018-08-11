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
|* Module: mp_os.c
|*
|* Description: 
|*
|*  This module contains the functions OS specfic
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20141231     JG              Initial version
|*
****************************************************************************/

/* Includes and defines */
#include <stdio.h>
#if _WIN32
#   include <windows.h>
#   include <stdlib.h>
#endif
#include "mp.h"
#include "mp_os.h"

/* Prototypes */

/* Structs */

/* Global variables */
#if MP_THREAD_SAFE == 1 /* { */
extern MP_TLS_INT mperrno;
#endif /* } MP_THREAD_SAFE */


/****************************************************************************
|*
|* Function: mp_mutex_init
|*
|* Description;
|*
|*     Windows Critical Sections cannot be statically initialized, therefore 
|*     we need a explicit initialization with InitializeCriticalSection().
|*     PTHREAD_MUTEX_INITIALIZER is enouth on pthread, so this function will
|*     be used mainly for WIN32.
|*
|* Return:
|*     n/a
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|* 20141231    JG    Initial version
|*
****************************************************************************/
#if MP_THREAD_SAFE == 1 /* { */
int mp_mutex_init(MP_MUTEX_T **mutex_p)
{

	if (mutex_p == NULL)
	{
		mperrno = MP_ERRNO_PARM;
        return -1;
    }

#ifdef _WIN32 /* { */
	if (*mutex_p != NULL)
	{
        /* Already initialized */
		return MP_ERRNO_SUCCESS;
	}
	{
		CRITICAL_SECTION *mutex_new = malloc(sizeof(CRITICAL_SECTION));
		InitializeCriticalSection(mutex_new);
        if (InterlockedCompareExchangePointer(mutex_p, mutex_new, NULL) != NULL)
        {
			/* We lost the race, mutex already initialized by another thread. */
			/* So we destroy the new mutex and return. */
            DeleteCriticalSection(mutex_new);
            free(mutex_new);
			return MP_ERRNO_SUCCESS;
        }
		/* Our critical section is now initiliazed. */
	}
#endif /* } _WIN32 */

	/* Note: Mutex destruction can only happen on the caller, but for the moment no API is provided for that */

	return MP_ERRNO_SUCCESS;
}
#endif /* } MP_THREAD_SAFE */

/* EOF */
