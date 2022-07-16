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

void test_stdio(void **state) {
  FILE *fp;
  fp = fopen("/who.txt", "w+");
  assert_non_null(fp);
  int ret = fprintf(fp, "%s %s %s %d", "who", "are", "you", 2022);
  assert_true(ret);
  fclose(fp);

  fp = fopen("/who.txt", "r");
  assert_non_null(fp);
  fseek(fp, 0, SEEK_END);
  int len = ftell(fp);
  fclose(fp);
  assert_int_equal(len, 16);

  fp = fopen("who.txt", "r");
  assert_non_null(fp);

  char c = fgetc(fp);
  assert_true(c == 'w');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'h');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'o');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == ' ');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'a');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'r');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'e');
  assert_false(feof(fp));

  fclose(fp);

  fp = fopen("/fputs.txt", "w+");
  assert_non_null(fp);
  fputc('E', fp);
  fputc('V', fp);
  fputc('I', fp);
  fputc('L', fp);
  fclose(fp);

  fp = fopen("/fputs.txt", "r");
  assert_non_null(fp);

  c = fgetc(fp);
  assert_true(c == 'E');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'V');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'I');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(c == 'L');
  assert_false(feof(fp));

  c = fgetc(fp);
  assert_true(feof(fp));


}

int main(int argc, char *argv[]) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_stdio),

  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}