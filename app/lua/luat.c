#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

lua_State *L;
const char lua_src[] = {
    "print(\"hello lua!\")\n"
    };

void lua_test(void) {
  printf("luat test\n");
  L = luaL_newstate();
  printf("luat test 1\n");
  luaL_openlibs(L);
  printf("luat test 2\n");
  luaopen_base(L);
  printf("luat test 3\n");
  luaL_dostring(L, lua_src);
  printf("luat test 4\n");
  lua_close(L);
  printf("luat test end\n");
}

int main(int argc, char **argv) {
  lua_test();
  return 0;
}