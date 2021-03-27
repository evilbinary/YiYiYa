#include "event.h"

#include "stdlib.h"
#include "syscall.h"

event_t null_event;
#define ARRAY_QUEUE_TYPE event_t
#define ARRAY_QUEUE_TYPE_NULL null_event

#include "algorithm/array_queue.h"

aqueue_t queue;
event_info_t event_info;
mouse_data_t current_mouse;
u32 button_press=0;

static const char key_map[0x3a][2] = {
    /*00*/ {0x0, 0x0},   {0x0, 0x0}, {'1', '!'},   {'2', '@'},
    /*04*/ {'3', '#'},   {'4', '$'}, {'5', '%'},   {'6', '^'},
    /*08*/ {'7', '&'},   {'8', '*'}, {'9', '('},   {'0', ')'},
    /*0c*/ {'-', '_'},   {'=', '+'}, {'\b', '\b'}, {'\t', '\t'},
    /*10*/ {'q', 'Q'},   {'w', 'W'}, {'e', 'E'},   {'r', 'R'},
    /*14*/ {'t', 'T'},   {'y', 'Y'}, {'u', 'U'},   {'i', 'I'},
    /*18*/ {'o', 'O'},   {'p', 'P'}, {'[', '{'},   {']', '}'},
    /*1c*/ {'\n', '\n'}, {0x0, 0x0}, {'a', 'A'},   {'s', 'S'},
    /*20*/ {'d', 'D'},   {'f', 'F'}, {'g', 'G'},   {'h', 'H'},
    /*24*/ {'j', 'J'},   {'k', 'K'}, {'l', 'L'},   {';', ':'},
    /*28*/ {'\'', '\"'}, {'`', '~'}, {0x0, 0x0},   {'\\', '|'},
    /*2c*/ {'z', 'Z'},   {'x', 'X'}, {'c', 'C'},   {'v', 'V'},
    /*30*/ {'b', 'B'},   {'n', 'N'}, {'m', 'M'},   {',', '<'},
    /*34*/ {'.', '>'},   {'/', '?'}, {0x0, 0x0},   {'*', '*'},
    /*38*/ {0x0, 0x0},   {' ', ' '}};

int event_init() {
  if (event_info.is_init > 0) {
    return 0;
  }
  event_info.is_init = 1;
  event_info.mouse_fd = open("/dev/mouse", 0);
  event_info.input_fd = open("/dev/stdin", 0);
  return 1;
}

u32 scan_code_to_key(u32 scan_code) { return key_map[scan_code]; }

int event_read_mouse(mouse_data_t* mouse_data) {
  read(event_info.mouse_fd, &event_info.mouse, sizeof(mouse_data_t));
  *mouse_data = event_info.mouse;
  return 1;
}

int event_read_key(u32* key) {
  int ret = read(event_info.input_fd, &event_info.scan_code, 1);
  *key = scan_code_to_key(event_info.scan_code);
  return ret;
}

int event_poll(event_t* event) {
  mouse_data_t mouse;
  event_t e;
  u32 count = 0;
  if (event_read_mouse(&mouse) > 0) {
    if (current_mouse.sate != mouse.sate) {
      u8 button = mouse.sate & BUTTON_MASK;
      if (button == BUTTON_LEFT || button == BUTTON_RIGHT ||
          button == BUTTON_MIDDLE) {
        if(button_press==0){
          e.type = MOUSE_BUTTON_DOWN;
          e.mouse = mouse;
          aqueue_push(&queue, e);
          button_press=1;
        }
      } else {
        if(button_press==1){
          e.type = MOUSE_BUTTON_UP;
          e.mouse = mouse;
          aqueue_push(&queue, e);
          button_press=0;
        }
      }
      count++;
      current_mouse = mouse;
    }
    if (current_mouse.x != mouse.x || current_mouse.y != mouse.y) {
      e.mouse = mouse;
      e.type = MOUSE_MOTION;
      aqueue_push(&queue, e);
      count++;
      current_mouse = mouse;
    }
    
  }

  u32 key;
  if (event_read_key(&key) > 0) {
    e.type = KEY_DOWN;
    e.key = key;
    aqueue_push(&queue, e);
  }

  e = aqueue_pop(&queue);
  if (e.type != 0) {
    *event = e;
    return 1;
  }
  return 0;
}
