#include "screen.h"

int input_fd;

extern int bThread;

static unsigned char keyPad = 0;

int InitJoypadInput(void) {
  input_fd = open("/dev/stdin", 0);
  keyPad = 0;
  return 1;
}

/**
 * FC手柄 bit 键位对应关系 真实手柄中有一个定时器，处理 连A  连B
 * 0  1   2       3       4    5      6     7
 * A  B   Select  Start  Up   Down   Left  Right
 */

int GetJoypadInput(void) {
  u32 key = 0;
  u32 press = event_read_key(&key);
  if (press > 0) {
    if (press == 2) {  // down
      switch (key) {
        case 'd':  //右
          keyPad |= (1 << 7);
          break;
        case 'a':  //左
          keyPad |= (1 << 6);
          break;
        case 's':  // 下
          keyPad |= (1 << 5);
          break;
        case 'w':  //上
          keyPad |= (1 << 4);
          break;
        case 'n':  //开始
          keyPad |= (1 << 3);
          break;
        case 'm':  //选择
          keyPad |= (1 << 2);
          break;
        case 'k':  // B
          keyPad |= (1 << 1);
          break;
        case 'j':  // A
          keyPad |= (1 << 0);
          break;
        case 'q':
          printf("quit\n");
          bThread = 0;
        default:
          break;
      }
    }else if (press == 1) {  // up
      switch (key) {
        case 'd':  //右
          keyPad &= ~(1 << 7);
          break;
        case 'a':  //左
          keyPad &= ~(1 << 6);
          break;
        case 's':  // 下
          keyPad &= ~(1 << 5);
          break;
        case 'w':  //上
          keyPad &= ~(1 << 4);
          break;
        case 'n':  //开始
          keyPad &= ~(1 << 3);
          break;
        case 'm':  //选择
          keyPad &= ~(1 << 2);
          break;
        case 'k':  // B
          keyPad &= ~(1 << 1);
          break;
        case 'j':  // A
          keyPad &= ~(1 << 0);
          break;
        default:
          break;
      }
    }
  } else {
    return -1;
  }
  return keyPad;
}