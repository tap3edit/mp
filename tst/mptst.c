/****************************************************************************
|*
|* tap3edit Tools (http://www.tap3edit.com)
|*
|* $Id: $
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
|* Module: test.c
|*
|* Description: Memory pool management system test
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20140801     JG              Initial version
|*
****************************************************************************/

#include <stdio.h>
#include <string.h>
#if _WIN32 /* { */
#   include <STDDEF.h>
#else /* } _WIN32 { */
#   include <stdint.h>
#endif /* } _WIN32 */

#include "mp.h"

#if MP_THREAD_SAFE /* { */
extern MP_TLS_INT mperrno;
#else /* } MP_THREAD_SAFE { */
extern int mperrno;
#endif /* } MP_THREAD_SAFE */

int mytrc(FILE *fd, char *fmt, va_list ap)
{
   char line[128] = "";

   if (fd == stdout)
   {
       vsnprintf(line, sizeof(line), fmt, ap);
       printf("         %s\n", line);
   }
   else
   {
       vfprintf(fd, fmt, ap);
       fprintf(fd, "\n");
   }


   return 0;
}

int main(int argc, char *argv[])
{
   int mpid_test1 = MP_NO_MP_ID;
   int mpid_test2 = MP_NO_MP_ID;
   int mpid_test3 = MP_NO_MP_ID;
   int mpid_test4 = MP_NO_MP_ID;
   int i = 0;
   size_t sz = 0;
   char * mystr = NULL;
   char * mystr2 = NULL;
   char * mystr3 = NULL;
   char * mystr4 = NULL;
   char * mystr_err = NULL;
   char debug = 'N';
   typedef struct _mystruct
   {
       char test[16];
       struct _mystruct *next;
   } mystruct;
   mystruct *myst = NULL;
   mystruct *myst_temp = NULL;
   size_t alignment = 0;
   char dmpfl[] = "memdmp.txt";
   int rc = 0;
   char descr[16] = "";

   if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'd')
   {
      debug = 'Y';
   }

   /* mptrc_set_fn() */
   mptrc_set_fn(&mytrc);
   printf("0. mptrc_set_fn(): We set our own trace() function\n");

   if ((rc = mptrc_set_fn(NULL)) != MP_ERRNO_SUCCESS)
   {
      printf("    0.1. mptrc_set_fn(): Correct error when giving NULL as a parameter: %s\n", mpstrerror());
   }

   /* mpmalloc() on default memory pool */
   sz = sizeof(char) * 1000;
   mystr = (char *)mpmalloc(sz);
   if (mystr == NULL)
   {
      printf("Error on malloc(): %s\n", mpstrerror());
   }
   else
   {

      sprintf(mystr, "This memory (%ld bytes) is allocated in the Default memory pool", (long)sz);
      printf("1. mpmalloc(): %s\n", mystr);
      if (debug == 'Y')
         mpprn();
   }
   mystr_err = (char *)mpmalloc(0);
   if (mystr_err == NULL)
   {
      printf("Error on malloc(): %s\n", mpstrerror());
   }
   else
   {
      printf("    1.1. mpmalloc(): Result is not NULL when given size 0\n");
   }
   if (((size_t)-1) < 0)
   {
      mystr_err = (char *)mpmalloc(-1);
      if (mystr_err != NULL)
      {
         printf("Result is not NULL. Check\n");
      }
      else
      {
         printf("    1.2. mpmalloc(): Result is NULL when given negative size\n");
      }
   }
   else
   {
      printf("    1.2. mpmalloc(): size_t is not signed, not testing with negative sizes\n");
   }

   /* mpnew() */
   mpid_test1 = mpnew("Test");
   printf("2. mpnew(): Initialized memory pool %d\n", mpid_test1);
   if (debug == 'Y')
      mpprn();
   for (i = 0; i < 201; i++)
   {
      sprintf(descr, "MP: %d", i);
      if (mpnew(descr) < 0)
      {
         printf("    2.1. mpnew(): Correct error when exceeding the memory pools limit of 100: %s\n", mpstrerror());
         break;
      }
   }
   mpdel_all();
   mpid_test1 = mpnew("Test");

   /* mpmalloc_mpid() */
   sz = sizeof(char) * 100000;
   mystr2 = (char *)mpmalloc_mpid(sz, mpid_test1);
   if (mystr2 == NULL)
   {
      printf("Error on malloc_mpid(): %s\n", mpstrerror());
   }
   else
   {
      sprintf(mystr2, "This memory (%ld bytes) is allocated in memory pool with ID <%d>", (long)sz, mpid_test1);
      printf("3. mpmalloc_mpid(): %s\n", mystr2);
      if (debug == 'Y')
         mpprn();
   }
   mystr_err = (char *)mpmalloc_mpid(sz, 98);
   if (mystr_err == NULL && mperrno != MP_ERRNO_SUCCESS)
   {
      printf("    3.1. mpmalloc_mpid(): Correct error when using a not yet initialized memory pool ID: %s\n", mpstrerror());
   }
   else
   {
      printf("Error mystr_err is not NULL or mperrno not set\n");
   }

   /* mpget_chunk(), mpadd_block() */
   mpid_test2 = mpnew("Test 2");
   myst_temp = NULL;
   for(i = 0; i < 100000; i++)
   {
      myst = (mystruct *)mpmalloc_mpid(sizeof(mystruct), mpid_test2);
      if (myst == NULL)
      {
         printf("Error on mpmalloc_mpid(): %s\n", mpstrerror());
         break;
      }
      memset(myst, 0x00, sizeof(mystruct));
      sprintf(myst->test, "test: %d", i);
      myst->next = myst_temp;
      myst_temp = myst;

   }
   if (mperrno == MP_ERRNO_SUCCESS)
   {
      printf("4. mpget_chunk(), mpadd_block(): Allocated 100000 structs (each %ld bytes) in the memory pool with ID <%d>\n", (long)sizeof(mystruct), mpid_test2);
      if (debug == 'Y')
         mpprn();
   }

   /* mpclr() */
   mpclr(mpid_test1);
   printf("5. mpclr(): Clear the memory of pool <%d>\n", mpid_test1);
   mystr2 = NULL;
   if (debug == 'Y')
      mpprn();

   /* mpdel() */
   mpdel(mpid_test1);
   printf("6. mpdel(): Delete (and free) the memory of pool <%d>\n", mpid_test1);
   if (debug == 'Y')
      mpprn();

   /* mpmemalign(), mpget() */
   sz = sizeof(char) * 10000;
   alignment = 128;
   mystr3 = (char *)mpmemalign(alignment, sz);
   if (mystr3 == NULL)
   {
      printf("Error on mpmemalign(): %s\n", mpstrerror());
   }
   else
   {
      printf("7. mpmemalign(), mpget(): Memory aligned to %d bytes in memory pool <%d> -> retrieved pointer <%p>, if this is 0, alignment was successful <%d>\n", (int)alignment, mpget(), mystr3, (int)((uintptr_t)mystr3 % alignment));
      if (debug == 'Y')
         mpprn();
   }
   mystr_err = (char *)mpmemalign(9, sz);
   if (mystr_err == NULL && mperrno != MP_ERRNO_SUCCESS)
   {
      printf("    7.1 mpmemalign(): Correct error when using wrong alignmet: %s\n", mpstrerror());
   }
   else
   {
      printf("Error mystr_err is not NULL or mperrno is not set\n");
   }

   /* mpmemalign_mpid() */
   mpid_test3 = mpnew("Test 3");
   sz = sizeof(char) * 10000;
   alignment = 256;
   mystr3 = (char *)mpmemalign_mpid(alignment, sz, mpid_test3);
   if (mystr3 == NULL)
   {
      printf("Error on mpmemalign_mpid(): %s\n", mpstrerror());
   }
   else
   {
      printf("8. mpmemalign_mpid(): Memory aligned to %d bytes in memory pool <%d> -> retrieved pointer <%p>, if this is 0, alignment was successful <%d>\n", (int)alignment, mpid_test3, mystr3, (int)((uintptr_t)mystr3 % alignment));
      if (debug == 'Y')
         mpprn();
   }

   /* mpcalloc(), mpset() */
   sz = 10000;
   mpset(mpid_test2);
   mystr2 = (char *)mpcalloc(1, sz);
   if (mystr2 == NULL)
   {
      printf("Error on mpcalloc(): %s\n", mpstrerror());
   }
   else
   {
      sprintf(mystr2, "This memory (%ld bytes) is allocated in memory pool <%d>, Firt 5 bytes: %02x%02x%02x%02x%02x", (long)sz, mpid_test2,
            mystr2[0],
            mystr2[1],
            mystr2[2],
            mystr2[3],
            mystr2[4]
            );
      printf("9. mpcalloc(): %s\n", mystr2);
      if (debug == 'Y')
         mpprn();
   }

   /* mpcalloc_mpid() */
   sz = 100000;
   mystr4 = (char *)mpcalloc_mpid(1, sz, mpid_test1);
   if (mystr4 == NULL)
   {
      printf("Error on mpcalloc_mpid(): %s\n", mpstrerror());
   }
   else
   {
      sprintf(mystr4, "This memory (%ld bytes) is allocated in memory pool with ID <%d>, Firt 5 bytes: %02x%02x%02x%02x%02x", (long)sz, mpid_test1,
            mystr4[0],
            mystr4[1],
            mystr4[2],
            mystr4[3],
            mystr4[4]
            );

      printf("10. mpcalloc_mpid(): %s\n", mystr4);
      if (debug == 'Y')
         mpprn();
   }

   /* mprealloc() */
   sz = 20000;
   mpset(mpid_test2);
   mystr2 = (char *)mprealloc(mystr2, sz);
   if (mystr2 == NULL)
   {
      printf("Error on mprealloc(): %s\n", mpstrerror());
   }
   else
   {
      printf("11. mprealloc(): This memory (%ld bytes) is allocated in memory pool <%d>, First 15 bytes <%.*s>\n", (long)sz, mpid_test2, 15, mystr2);
      if (debug == 'Y')
         mpprn();
   }

   /* mprealloc_mpid() */
   sz = 200000;
   mystr4 = (char *)mprealloc_mpid(mystr4, sz, mpid_test1);
   if (mystr4 == NULL)
   {
      printf("Error on mprealloc_mpid(): %s\n", mpstrerror());
   }
   else
   {
      printf("12. mprealloc_mpid(): This memory (%ld bytes) is allocated in memory pool with ID <%d>, First 15 bytes <%.*s>\n", (long)sz, mpid_test1, 15, mystr4);
      if (debug == 'Y')
         mpprn();
   }


   /* mpstrdup() */
   if (mystr2 != NULL)
   {
      sprintf(mystr2, "This str will be duplicated");
   }
   else
   {
      sz = 20000;
      mystr2 = (char *)mpmalloc(sz);
      if(mystr2 != NULL)
      {
         sprintf(mystr2, "This str will be duplicated");
      }
   }
   mpset(mpid_test1);
   mystr = mpstrdup(mystr2);
   if (mystr == NULL && mystr2 != NULL)
   {
      printf("Error on mpstrdup(): %s\n", mpstrerror());
   }
   else
   {
      printf("13. mpstrdup(): This memory (%d) is allocated in memory pool with ID <%d>. String duplicated <%s>\n", mystr2 == NULL ? 0 :(int)strlen(mystr2), mpget(), mystr == NULL ? "<NULL>" : mystr);
      if (debug == 'Y')
         mpprn();
   }

   /* mpstrdup_mpid() */
   if (mystr2 != NULL)
   {
      sprintf(mystr2, "This str will be duplicated");
   }
   else
   {
      sz = 20000;
      mystr2 = (char *)mpmalloc(sz);
      if(mystr2 != NULL)
      {
         sprintf(mystr2, "This str will be duplicated");
      }
   }
   mystr = mpstrdup_mpid(mystr2, mpid_test2);
   if (mystr == NULL && mystr2 != NULL)
   {
      printf("Error on mpstrdup(): %s\n", mpstrerror());
   }
   else
   {
      printf("14. mpstrdup_mpid(): This memory (%d) is allocated in memory pool with ID <%d>. String duplicated <%s>\n", mystr2 == NULL ? 0 :(int)strlen(mystr2), mpid_test2, mystr == NULL ? "<NULL>" : mystr);
      if (debug == 'Y')
         mpprn();
   }

   /* mpasprintf() */
   if (mpasprintf(&mystr, "This is a test of \"%s\"", "mpasprintf") > 0)
   {
      printf("15. mpasprintf(): This memory (%d) is allocated in memory pool with ID <%d>. Allocated string <%s>\n", mystr == NULL ? 0 :(int)strlen(mystr), mpget(), mystr == NULL ? "<NULL>" : mystr );
      if (debug == 'Y')
         mpprn();
   }
   else
   {
      printf("Error allocating memory for mystr: %s\n", mpstrerror());
   }
   if (mpasprintf(NULL, "This is a test of \"%s\"", "mpasprintf") < 0)
   {
      printf("    15.1 mpasprintf(): Correct error when passing NULL as parameter: %s\n", mpstrerror());
   }
   else
   {
      printf("Error returned bytes are not negative as expected\n");
   }

   /* mpasprintf_mpid() */
   mpid_test4 = mpnew("");
   if (mpasprintf_mpid(&mystr, mpid_test4, "This is a test of \"%s\"", "mpasprintf_mpid") > 0)
   {
      printf("16. mpasprintf_mpid(): This memory (%d) is allocated in memory pool with ID <%d>. Allocated string <%s>\n", mystr == NULL ? 0 :(int)strlen(mystr), mpid_test4, mystr == NULL ? "<NULL>" : mystr );
      if (debug == 'Y')
         mpprn();
   }
   else
   {
      printf("Error allocating memory for mystr: %s\n", mpstrerror());
   }
   if (mpasprintf_mpid(NULL, mpid_test4, "This is a test of \"%s\"", "mpasprintf_mpid") < 0)
   {
      printf("    16.1 mpasprintf_mpid(): Correct error when passing NULL as parameter: %s\n", mpstrerror());
   }
   else
   {
      printf("Error returned bytes are not negative as expected\n");
   }

   /* mppush(), mppop() */
   mpset(mpid_test1);
   printf("17. mppush(), mppop(): Now active pool ID is <%d>, ", mpget());   
   mppush(mpid_test2);
   printf("now the active pool ID is <%d>, ", mpget());
   mppop();
   printf("now back to pool ID is <%d>\n", mpget());

   /* mpdmp() */
   mpdmp(dmpfl);
   printf("18. mpdmp(): File <%s> should be created with the dump of all memory pools\n", dmpfl);
   if (debug != 'Y')
      remove(dmpfl);

   /* mpset_memlim(), mpget_memlim() */
   printf("19. mpset_memlim(), mpget_memlim(): current memory limit is <%lu>, ", (unsigned long) mpget_memlim());
   mpset_memlim(5 * 1024 * 1024);
   printf("now it is <%lu>\n", (unsigned long) mpget_memlim());

   /* mpdel_all() */
   mpdel_all();
   printf("20. mpdel_all(): now all memory pools allocated should be released (freeed)\n");
   mpprn();

   if (debug != 'Y')
       printf("Run in debug mode (with option -d) for more information and dumping the memory dump file <%s>\n", dmpfl);

   return 0;
}

/* EOF */
