#ifndef VID_H
#define VID_H

/* stuff implemented by the different sys/ backends */
/* For porting to new architectures / systems -> Implement these methods */

void vid_begin();
void vid_end();
void vid_init();
void vid_preinit();
void vid_close();
void vid_setpal(int i, int r, int g, int b);
void vid_settitle(char *title);

/* Stuff specific to sdl2-gnuboy video */
void vid_fullscreen_toggle();
void vid_screenshot();

void pcm_init();
int pcm_submit();
void pcm_close();
void pcm_pause();
void pcm_resume();

void ev_poll();
int confirm_exit();

void sys_checkdir(char *path, int wr);
void sys_sleep(int us);
void sys_sanitize(char *s);

void joy_init();
void joy_poll();
void joy_close();

void kb_init();
void kb_poll();
void kb_close();

int io_setup(const char* link);

int sys_elapsed(int *prev);
void sys_initpath();

#endif
