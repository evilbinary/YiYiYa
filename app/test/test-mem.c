#include <setjmp.h>
#include "stdio.h"
#include "cmocka.h"

void test_malloc_free(void** state){
    void* p=malloc(412);
    assert_non_null(p);
    free(p);
}

void test_malloc_large(void** state){
    void* p=malloc(1024*1024*1024);
    assert_non_null(p);
    free(p);
}

int main(int argc, char* argv[]) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_malloc_free), 
      cmocka_unit_test(test_malloc_large),

  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}