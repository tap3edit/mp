/****************************************************************************
|*
|* tap3edit Tools (http://www.tap3edit.com)
|*
|* $Id: $
|*
|* Copyright (c) 2015, Javier Gutierrez <jgutierrez@tap3edit.com>
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
|* Description: Shows the usage of different memory pools.
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20150104     JG              Initial version
|*
****************************************************************************/

#include <stdio.h>
#include "mp.h"

int main(int argc, char *argv[])
{
    int mpid1 = MP_NO_MP_ID;    /* Memory pool ID 1 */
    int mpid2 = MP_NO_MP_ID;    /* Memory pool ID 2 */
    char *strdef = NULL;
    char *str1 = NULL;
    char *str2 = NULL;

    /* At this moment no memory pool was created, 
     * so the default one is used */
    strdef = mpmalloc(sizeof(char) * 20);
    sprintf(strdef, "This is default mp");

    /* Here we create memory pool 1 */
    mpid1 = mpnew("MP 1");
    mpset(mpid1); /* In order to use the new pool 
                     we need to make it active */
    str1 = mpmalloc(sizeof(char) * 20);
    sprintf(str1, "This is mp 1");

    /* Here we create memory pool 2 */
    mpid2 = mpnew("MP 2");
    mpset(mpid2); /* In order to use the new pool 
                     we need to make it active */
    str2 = mpmalloc(sizeof(char) * 20);
    sprintf(str2, "This is mp 2");

    printf("strdef: <%s>\n", strdef);
    printf("str1: <%s>\n", str1);
    printf("str2: <%s>\n", str2);
    /* Print the usage statistics of all memory pools */
    mpprn();

    /* Delete all memory pools. This releases all memory allocated */
    mpdel_all();

    return 0;
}

/* EOF */
