/*
 * DO NOT EDIT THIS FILE. Generated by checkmk.
 * Edit the original source file "ut/ut_mp_trc.check" instead.
 */

#include <check.h>

#line 1 "ut/ut_mp_trc.check"
/****************************************************************************
|*
|* tap3edit Tools (http://www.tap3edit.com)
|*
|* $Id: ut.check 52 2014-09-20 15:08:24Z mrjones $
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
|* Module: ut_mp_trc.check
|*
|* Description: Trace unit test
|*
|* Author: Javier Gutierrez (JG)
|*
|* Modifications:
|*
|* When         Who     Pos     What
|* 20140926     JG              Initial version
|*
****************************************************************************/
#include "mp.c"
#include "mp_trc.c"

static char g_str[128] = "";

static int mptrc_ut(FILE *fd, char *fmt, va_list ap)
{
    int rc = 0;

    rc = vsprintf(g_str, fmt, ap);

    return rc;
}

static int mptrc_error_ut(FILE *fd, char *fmt, va_list ap)
{
    int rc = -1;

    /* freopen("fff", "r", stdout); */
    /* rc = printf(fmt, ap); */

    return rc;
}


START_TEST(mptrc_set_fn_parm_check)
{
#line 61
    int rc = 0;

    rc = mptrc_set_fn(NULL);

    ck_assert_msg(
        rc == MP_ERRNO_PARM
    , "Parameter is NULL and return value is not MP_ERRNO_PARM");

    ck_assert_msg(
        mperrno == MP_ERRNO_PARM
    , "Parameter is NULL and mperrno is not MP_ERRNO_PARM");


}
END_TEST

START_TEST(mptrc_set_fn_check)
{
#line 75
    mptrc_set_fn(&mptrc_ut);

    ck_assert_msg(
        mptrc_fn == &mptrc_ut
    , "The user trace function was not assigned correctly");


}
END_TEST

START_TEST(mptrc_parm_check)
{
#line 83
    FILE fd;
    int rc = 0;

    rc = mptrc(&fd, NULL);

    ck_assert_msg(
        rc == MP_ERRNO_PARM
    , "Parameter is NULL and return value is not MP_ERRNO_PARM");

    ck_assert_msg(
        mperrno == MP_ERRNO_PARM
    , "Parameter is NULL and mperrno is not MP_ERRNO_PARM");


}
END_TEST

START_TEST(mptrc_check_string)
{
#line 98
    char str[] = "This is a test";

    memset(g_str, 0x00, sizeof(g_str));

    mptrc_set_fn(&mptrc_ut);

    mptrc(NULL, "%s", str);

    ck_assert_msg(
        strcmp(str, g_str) == 0
    , "The printed string <%s> doesn't match with the original string <%s>", g_str, str);


}
END_TEST

START_TEST(mptrc_check_file)
{
#line 112
    FILE *fd = NULL;
    char filename[] = "ut_mptrc_check_file.txt";
    char str[] = "This is a test";

    memset(g_str, 0x00, sizeof(g_str));

    if ((fd = fopen(filename, "w+b")) == NULL)
    {
        ck_abort_msg("Error opening file <%s>: %s", filename, strerror(errno));
    }

    mptrc(fd, "%s", str); /* This checks also mtrc_internal() */

    if (fseek(fd, 0, SEEK_SET) != 0)
    {
        fclose(fd);
        ck_abort_msg("Error rewinding in file <%s>: %s", filename, strerror(errno));
    }

    if (fread(g_str, sizeof(str), 1, fd) == 0)
    {
        fclose(fd);
        ck_abort_msg("Error reading from file <%s>: %s", filename, strerror(errno));
    }

    fclose(fd);
    remove(filename);

    if (g_str[strlen(g_str) -1] == '\n')
    {
        g_str[strlen(g_str) -1] = '\0';
    }

    ck_assert_msg(
        strcmp(str, g_str) == 0
    , "The printed string <%s> doesn't match with the original string <%s>", g_str, str);

}
END_TEST

START_TEST(mptrc_error_check)
{
#line 150
    int rc = 0;
    char str[] = "This is a test";

    mptrc_set_fn(&mptrc_error_ut);
    
    rc = mptrc(NULL, "%s", str);

    ck_assert_msg(
        mperrno == MP_ERRNO_DISP
    , "Expected mperrno to be MP_ERRNO_DISP but it is <%d>", mperrno);

    ck_assert_msg(
        rc == MP_ERRNO_DISP
    , "Expected return code to be MP_ERRNO_DISP but it is <%d>", mperrno);


}
END_TEST

int main(void)
{
    Suite *s1 = suite_create("Trace Logic");
    TCase *tc1_1 = tcase_create("Trace Logic");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_test(tc1_1, mptrc_set_fn_parm_check);
    tcase_add_test(tc1_1, mptrc_set_fn_check);
    tcase_add_test(tc1_1, mptrc_parm_check);
    tcase_add_test(tc1_1, mptrc_check_string);
    tcase_add_test(tc1_1, mptrc_check_file);
    tcase_add_test(tc1_1, mptrc_error_check);

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}
