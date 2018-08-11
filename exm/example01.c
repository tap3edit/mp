/****************************************************************************
|*
|* tap3edit Tools (http://www.tap3edit.com)
|*
|* Copyright (c) 2015-2018, Javier Gutierrez <https://github.com/tap3edit/mp>
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
|* Description: for 10 given integer pointers, it allocates memory dynamically
|*      then it releases all allocated memory at once.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20150103     JG              Initial version
|*
****************************************************************************/

#include <stdio.h>
#include "mp.h"

int main(int argc, char *argv[])
{
    int i = 0;
    int *j[10] = {0}; /* Array of 10 integer pointers */

    /* Loop the array of pointers */
    for(i = 0; i < 10; i++)
    {
        /* Allocatest memory in our default memory pool */
        j[i] = mpmalloc(sizeof(int)); 
        /* Assignes to the allocated integer a certain value */
        *j[i] = i * 2;
    }

    /* Loop the array of pointers */
    for(i = 0; i < 10; i++)
    {
        /* Print out the values of each of the integer pointers */
        printf("j[%d] = %d\n", i, *j[i]);
    }

    /* Release allocated memory */
    mpdel_all();

    return 0;
}

/* EOF */
