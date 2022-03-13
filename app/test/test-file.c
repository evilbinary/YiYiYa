#include <dirent.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "cmocka.h"
#include "stdio.h"

char* buf = "hello,file\n";

#define PRINT_WIDTH 24
#define READ_BUFFER 24 * 20

void test_read(void** state) {
  char* name = "/duck.png";
  int succes = 1;
  char* buffer = malloc(READ_BUFFER);
  memset(buffer, 0, READ_BUFFER);
  FILE* fp;
  fp = fopen(name, "r+");
  assert_non_null(fp);
  printf("fd=%d\n", *fp);
  int offset = 0;
  for (;;) {
    int ret = fseek(fp, offset, SEEK_SET);
    assert_int_equal(ret, offset);
    ret = fread(buffer, READ_BUFFER, 1, fp);
    if (ret <= 0) {
      break;
    }
    for (int i = 0; i < ret; i++) {
      if (i % PRINT_WIDTH == 0) {
        printf("\n %07x   ", offset);
      }
      printf("%02x ", 0xff & buffer[i]);
      offset++;
      if (!(offset == 0x0011808)) {
        assert_int_equal(buffer[i], 0xbc);
      }
    }
    printf("ret=%d\n", ret);
  }
  if (fp != NULL) {
    fclose(fp);
  }
}

void test_write(void** state) {
  FILE* fp = fopen("/hello.txt", "w+");
  assert_non_null(fp);
  int ret = fseek(fp, 0, SEEK_SET);
  assert_true(ret == 0);
  ret = fwrite("ABCDEF", strlen("ABCDEF"), 1, fp);
  assert_int_equal(ret, 6);
  ret = fclose(fp);
  assert_true(ret == 0);
}

void test_write_read(void** state) {
  int ret;
  FILE* fp = fopen("/read-write.txt", "w+");
  assert_non_null(fp);
  ret = fseek(fp, 0, SEEK_SET);
  assert_true(ret == 0);
  for (int i = 0; i < 100; i++) {
    ret = fwrite("ABCDEF", strlen("ABCDEF"), 1, fp);
    assert_int_equal(ret, 6);
  }
  ret = fclose(fp);
  assert_true(ret == 0);

  char buf[64];
  fp = fopen("/read-write.txt", "r+");
  ret = fseek(fp, 0, SEEK_SET);
  assert_non_null(fp);
  for (int i = 0; i < 100; i++) {
    memset(buf, 0, 64);
    ret = fread(buf, strlen("ABCDEF"), 1, fp);
    assert_int_equal(ret, 6);
    assert_string_equal(buf, "ABCDEF");
  }
  ret = fclose(fp);
  assert_true(ret == 0);
}

void test_read_dir(void** state) {
  DIR* dir;
  struct dirent* ptr;
  int i;
  dir = opendir("/");
  assert_non_null(dir);
  while ((ptr = readdir(dir)) != NULL) {
    printf("name : %s type: %d\n", ptr->d_name, ptr->d_type);
  }
  int ret = closedir(dir);
  assert_true(ret == 0);
}

static char get_u8(int fd) {
  char buf[1] = {0xff};
  int ret = 0;
  if (ret = read(fd, &buf, 1) != 1) {
    printf("read faild ret=%d\n", ret);
    return -1;
  }
  printf("  ret=>%d  data=>%x\n", ret, buf[0]);
  return buf[0];
}

void test_read_byte(void** state) {
  char* path = "/scheme.boot";
  int fd = open(path, 0);
  assert_true(fd > 0);
  if (get_u8(fd) != 0 || get_u8(fd) != 0 || get_u8(fd) != 0 ||
      get_u8(fd) != 0 || get_u8(fd) != 'c' || get_u8(fd) != 'h' ||
      get_u8(fd) != 'e' || get_u8(fd) != 'z') {
    printf("malformed fasl-object header in %s\n", path);
    assert_true(0);
  }
}

void test_seek(void** state) {
  FILE* fp = fopen("/scheme.boot", "r+");
  assert_non_null(fp);

  int seek = fseek(fp, 1, SEEK_SET);
  assert_int_equal(seek, 1);

  seek = fseek(fp, 2, SEEK_SET);
  assert_int_equal(seek, 2);

  seek = fseek(fp, 2, SEEK_CUR);
  assert_int_equal(seek, 4);

  seek = fseek(fp, 2, SEEK_END);

  int ret = fclose(fp);
  assert_true(ret == 0);
}

int main(int argc, char* argv[]) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_read),       cmocka_unit_test(test_write),
      cmocka_unit_test(test_write_read), cmocka_unit_test(test_read_dir),
      cmocka_unit_test(test_seek),       cmocka_unit_test(test_read_byte),

  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}