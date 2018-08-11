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
|* Description: Shows the usage of different memory pools by using one pool
|*      for the creation of each of the structures and a different one for
|*      its elements.
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

int elem_mpid = MP_NO_MP_ID;    /* Memory pool ID for elements */
int struct_mpid = MP_NO_MP_ID;    /* Memory pool ID for the structures */

typedef struct myst_t
{
    int id;
    char *name;
    char *desc;
} myst_t;

int alloc_elem(myst_t *arr_st, int id);
int alloc_struct(myst_t **arr_st, int id);

int main(int argc, char *argv[])
{
    int i = 0;
    myst_t *arr_st[10] = {0};       /* Array of 10 pointers of myst_t */
    char *title = NULL;

    /* Create 2 memory pools, one for structures and one for their 
     * elements */
    struct_mpid = mpnew("Structures");
    elem_mpid = mpnew("Elements");

    /* To use those memory pools, mpset() should be used, otherwise
     * the default memory pool is used as in following case */
    title = mpmalloc(sizeof(char) * 20);
    sprintf(title, "Example Number 04: Same as sample03 but using mpush(), mpop()");

    /* To access the default memory pool again the following command 
     * should be used: mpset(MP_DEF_MP_ID); */

    /* Loop the pointers of structures and call allocation for 
     * each one */
    for(i = 0; i < 10; i++)
    {
        alloc_struct(&arr_st[i], i);
    } 
    
    /* Print each of the structures new information */
    printf("%s\n", title);
    for(i = 0; i < 10; i++)
    {
        printf("Array Info -> ID <%d>, Name: <%s>, Description: <%s>\n", 
                arr_st[i]->id, arr_st[i]->name, arr_st[i]->desc);
    }

    /* Print the statistics of all memory pools */
    mpprn();

    /* Delete all memory pools */
    mpdel_all();

    return 0;
}

int alloc_struct(myst_t **arr_st, int id)
{
    /* We set the memory pool for structures */
    mpset(struct_mpid);

    /* Allocate the memory needed for our structure */
    *arr_st = mpmalloc(sizeof(myst_t));

    /* Now call the allocation of its elements */
    alloc_elem(*arr_st, id);

    return 0;
}

int alloc_elem(myst_t *arr_st, int id)
{


    /* This time we make active our elements memory pool by using mppush() */
    mppush(elem_mpid);

    arr_st->id = id;
    /* Allocate the strings */
    arr_st->name = mpmalloc(sizeof(char) * 20);
    sprintf(arr_st->name, "Struct %d", id);
    arr_st->desc = mpmalloc(sizeof(char) * 40);
    sprintf(arr_st->desc, "This is the structure number %d", id);

    /* Now we go back to the previously set memory pool from the stack */
    mppop();

    return 0;
}

/* EOF */
