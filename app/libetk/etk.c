#include "etk.h"
#include "etk_global.h"
#include "etk_display.h"
#include "etk_wnd_manager.h"
#include "etk_main_loop.h"

Ret etk_init(int argc, char* argv[]) {
  etk_platform_init(argc, argv);
  // main loop init
  etkglobal.sources_manager=etk_sources_manager_create();
  etkglobal.main_loop = etk_main_loop_create(etk_default_sources_manager());
  // win manager init
  etkglobal.wndManager = etk_default_wnd_manager_create(etkglobal.main_loop);

  // font init
  if (etkglobal.font == NULL) {
    etkglobal.font = etk_font_default_create("/dev/sda/zmrom.bin");
  }

  return RET_OK;
}

Ret etk_run(void) {
  Ret ret = etk_main_loop_run(etk_default_main_loop());
  return ret;
}
int etk_exit() {
  etk_platform_deinit();
  etk_default_main_loop()->running = 0;
}
