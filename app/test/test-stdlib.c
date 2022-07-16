#include <dirent.h>
#include <errno.h>
#include <float.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmocka.h"

void exit_function() { printf("exit function\n"); }

void test_stdlib(void **state) {
  // atexit(exit_function);

  assert_int_equal(abs(20), 20);
  assert_int_equal(abs(-20), 20);

  assert_float_equal(atof("123413412"), 123413412, FLT_EPSILON);
  assert_float_equal(atof("hello"), 0, FLT_EPSILON);

  assert_int_equal(atof("123413412"), 123413412);
  assert_int_equal(atof("hello"), 0);

  // assert_long_equal(atol("98993489"),98993489);
  // assert_long_equal(atol("hello"),0);

  div_t d = div(9, 4);
  assert_int_equal(d.quot, 2);
  assert_int_equal(d.rem, 1);

  char *ptr = NULL;
  char str[30] = "13.234 gaga haha";
  double ret = strtod(str, &ptr);
  assert_float_equal(ret, 13.234, DBL_EPSILON);
  assert_string_equal(ptr, " gaga haha");


  // assert_int_equal(system("ls"), 1);
  // assert_int_equal(rename("ls"),1);

  char str2[120] = "123414 gaga haha";
  long ret1 = strtol(str2, &ptr,10);
  assert_true(ret1==123414);
  assert_string_equal(ptr, " gaga haha");


}

int main(int argc, char *argv[]) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_stdlib),

  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}