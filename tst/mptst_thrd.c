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
|* Module: test_thread.c
|*
|* Description: Memory pool management system test with threads. Run as:
|*              valgrind --tool=helgrind ./test_thread
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20141018     JG              Initial version
|*
****************************************************************************/

#include <stdio.h>
#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "mp.h"

#ifndef _WIN32
pthread_mutex_t lock;
#endif

#ifndef _WIN32
void *my_routine(void *par)
#else
DWORD WINAPI my_routine(LPVOID par)
#endif
{
    int thr_id = (*((int *)par));
    int mpid = -1;
    char *ptr = NULL;
    char str[256] = "";

//    pthread_mutex_lock(&lock); /* Lock is done inside MP, so not needed here */

#ifndef _WIN32
    printf("%d: thread self: %lu, pointer %p\n", thr_id, (unsigned long)pthread_self(), par);
#else
    printf("%d: thread self: %d, pointer %p\n", GetCurrentThreadId, par);
#endif
    printf("%d: Starting process\n", thr_id);
    sprintf(str, "Thread %d: 1", thr_id);
    mpid = mpnew(str);
    mppush(mpid);
    if ((ptr = mpmalloc_mpid(sizeof(char)*100, mpid)) == NULL)
        printf("Error %s\n", mpstrerror());
    mpfree(ptr);
    mpdel(mpid);
    mppop();
    sprintf(str, "Thread %d: 2", thr_id);
    mpid = mpnew(str);
    mppush(mpid);
    if ((ptr = mpmalloc_mpid(sizeof(char)*200, mpid)) == NULL)
        printf("Error %s\n", mpstrerror());
    mpclr(mpid);
    mppop();
    sprintf(str, "Thread %d: 3", thr_id);
    mpid = mpnew(str);
    mppush(mpid);
    if ((ptr = mpmalloc_mpid(sizeof(char)*300, mpid)) == NULL)
        printf("Error %s\n", mpstrerror());
    //sleep(2);
    printf("%d: End of process\n", thr_id);

//    pthread_mutex_unlock(&lock);
#ifndef _WIN32
	return NULL;
#else
	return 0;
#endif
}

#define MAX_THREADS     10
int main(int argc, char *argv[])
{
    int rc = 0;
    int i = 0;
#ifndef _WIN32
	pthread_t t[MAX_THREADS];
#else
	DWORD   ret_t[MAX_THREADS];
    HANDLE  t[MAX_THREADS]; 
#endif
    int tid[MAX_THREADS] = {0}; /* Parameter for my_routine() */

#ifndef _WIN32
    printf("main thread ID: %lu\n", (unsigned long)pthread_self());
#else
    printf("main thread ID: %d\n", GetCurrentThreadId());
#endif
#ifndef _WIN32
    if (pthread_mutex_init(&lock, NULL) != 0) /* Not really used here */
    {
        printf("Error initialing mutex\n");
        return 1;
    }
#endif
    /* pthread_create() defines the parameter of callback with the "restrict" keyword 
     * so if we need to send the thread ID we need to store them first */
    for(i = 0; i < MAX_THREADS; i++)
    {
        tid[i] = i + 1;
    }

    /* Create each of the threads */
    for(i = 0; i < MAX_THREADS; i++)
    {
        printf("Starting thread %d\n", i);
#ifndef _WIN32
        if ((rc = pthread_create(&t[i], NULL, &my_routine, &tid[i])) != 0)
#else
		if ((t[i] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            my_routine,            // thread function name
            (LPVOID)&(tid[i]),      // argument to thread function 
            0,                      // use default creation flags 
            &ret_t[i]               // returns the thread identifier 
		)) == NULL)
#endif
        {
            printf("Error launching thread\n");
            return 1;
        }

    }

    /* Join each of the threads */
#ifndef _WIN32
    for(i = 0; i < MAX_THREADS; i++)
    {
        pthread_join(t[i], NULL);
    }
#else
	WaitForMultipleObjects(MAX_THREADS, t, TRUE, INFINITE);
#endif

	/* Close threads */
#ifndef _WIN32
	/* Nothing for pthreads */
#else
	for(i = 0; i < MAX_THREADS; i++)
    {
        CloseHandle(t[i]);
	}
#endif

    mpprn();

    /* End */
#ifndef _WIN32
    pthread_mutex_destroy(&lock);
#endif
    mpdel_all();

    return 0;
}

/* EOF */
