#include <dirent.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cmocka.h"

void test_memcmp(void **state) {
  char str1[15];
  char str2[15];
  int ret;

  memcpy(str1, "abcdef", 6);
  memcpy(str2, "ABCDEF", 6);

  ret = memcmp(str1, str2, 5);
  assert_true(ret > 0);

  memcpy(str1, "ABCDEF", 6);
  memcpy(str2, "abcdef", 6);

  ret = memcmp(str1, str2, 5);
  assert_true(ret < 0);

  memcpy(str1, "ABCDEF", 6);
  memcpy(str2, "ABCDEF", 6);

  ret = memcmp(str1, str2, 5);
  assert_true(ret == 0);
}

void test_memcpy(void **state) {
  char str1[15];
  void *ret;

  ret = memcpy(str1, "abcdef", 6);
  assert_string_equal(str1, "abcdef");
  assert_ptr_equal(ret, str1);
}

void test_memmove(void **state) {
  const char dest[] = "oldstring";
  const char src[] = "newstring";
  memmove(dest, src, 9);

  assert_string_equal(dest, "newstring");
}

void test_memchr(void **state) {
  const char str[] = "http://evilbinary.org";
  const char ch = '.';
  char *ret;
  ret = (char *)memchr(str, ch, strlen(str));
  assert_string_equal(ret, ".org");
}

void test_memset(void **state) {
  char str1[80];
  strcpy(str1, "hello world,haha gaga");
  memset(str1, 'E', 6);

  assert_string_equal(str1, "EEEEEEworld,haha gaga");
}

void test_strcat(void **state) {
  char src[80], dest[80];

  strcpy(src, "from the havent");
  strcpy(dest, "to the hell");
  strcat(dest, src);

  assert_string_equal(dest, "to the hellfrom the havent");
}

void test_strchr(void **state) {
  const char str[] = "http://evilbinary.org";
  const char ch = '.';
  char *ret;
  ret = (char *)strchr(str, ch);
  assert_string_equal(ret, ".org");
}

void test_strcmp(void **state) {
  char str1[15];
  char str2[15];
  int ret;

  strcpy(str1, "abcdef");
  strcpy(str2, "ABCDEF");

  ret = strcmp(str1, str2);
  assert_true(ret > 0);

  strcpy(str1, "ABCDEF");
  strcpy(str2, "abcdef");

  ret = strcmp(str1, str2);
  assert_true(ret < 0);

  strcpy(str1, "ABCDEF");
  strcpy(str2, "ABCDEF");

  ret = strcmp(str1, str2);
  assert_true(ret == 0);
}

void test_strcoll(void **state) {
  char str1[15];
  char str2[15];
  int ret;

  strcpy(str1, "abcdef");
  strcpy(str2, "ABCDEF");

  ret = strcoll(str1, str2);
  assert_true(ret > 0);

  strcpy(str1, "ABCDEF");
  strcpy(str2, "abcdef");

  ret = strcoll(str1, str2);
  assert_true(ret < 0);

  strcpy(str1, "ABCDEF");
  strcpy(str2, "ABCDEF");

  ret = strcoll(str1, str2);
  assert_true(ret == 0);
}

void test_strcspan() {
  int len;
  const char str1[] = "ABCDEF4960910";
  const char str2[] = "013";

  len = strcspn(str1, str2);
  assert_true(len == 9);
}

void test_strerror() {
  FILE *fp;
  fp = fopen("file-aadd.txt", "r");
  if (fp == NULL) {
    printf("Error: %s\n", strerror(errno));
    // todo
    assert_string_equal(strerror(errno), "unknown error number.");
  }
}

void test_strncat(void **state) {
  char src[80], dest[80];

  strcpy(src, "from the havent");
  strcpy(dest, "to the hell");
  strncat(dest, src, strlen(src));

  assert_string_equal(dest, "to the hellfrom the havent");
}

void test_strspn() {
  int len;
  const char str1[] = "ABCDEFG019874";
  const char str2[] = "ABCD";
  len = strspn(str1, str2);
  assert_int_equal(len, 4);
}

void test_strstr() {
  const char haystack[20] = "HELLOEVIL";
  const char needle[10] = "EVIL";
  char *ret;

  ret = strstr(haystack, needle);
  assert_string_equal(ret, "EVIL");
}

void tst_strtok() {
  // char str[80] = "gaga-evil-binary";
  // const char s[2] = "-";
  // char *token;

  // token = strtok(str, s);
  // assert_string_equal(token, "gaga");

  // token = strtok(NULL, s);
  // assert_string_equal(token, "gaga");
  // token = strtok(NULL, s);
  // assert_string_equal(token, "binary");
}

int main(int argc, char *argv[]) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_memcmp),   cmocka_unit_test(test_memcpy),
      cmocka_unit_test(test_memmove),  cmocka_unit_test(test_memchr),
      cmocka_unit_test(test_memset),   cmocka_unit_test(test_strcat),
      cmocka_unit_test(test_strchr),   cmocka_unit_test(test_strcmp),
      cmocka_unit_test(test_strcoll),  cmocka_unit_test(test_strcspan),
      cmocka_unit_test(test_strerror), cmocka_unit_test(test_strncat),
      cmocka_unit_test(test_strspn),   cmocka_unit_test(test_strstr),
      // cmocka_unit_test(tst_strtok),

  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}