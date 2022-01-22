#include "terminal.h"

#include "sys/ioctl.h"

#define IOC_PTY_MAGIC 'p'

#define IOC_SLAVE _IOW(IOC_PTY_MAGIC, 1, int)
#define IOC_READ_OFFSET _IOW(IOC_PTY_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_PTY_MAGIC, 4, int)
#define IOC_MASTER_READ_NOBLOCK _IOW(IOC_PTY_MAGIC, 5, int)
typedef struct {
  char* buf;
  u32 buf_len;
  u32 len;
} str_t;

const char* prompt = "yiyiya$ ";
bool running = true;
bool cursor = true;
bool focused = true;
static str_t* g_text_buf = NULL;
static str_t* g_input_buf = NULL;
u32 last_time = 0;

u32 fd_ptm;
u32 fd_pts;
u32 fd_out;
FILE* file_out;

const u32 twidth = 550;
const u32 theight = 342;
const u32 char_width = 8;
const u32 char_height = 16;
u32 max_col;
u32 max_line;

const u32 margin = 1;
const u32 text_color = WHITE;  // 0xE0E0E0;
u32 bg_color = 0x00353535;
const float cursor_blink_time = 1;

const u32 ret_buf_size = 128;
char ret_buf[128];

void str_free(str_t* str) {
  free(str->buf);
  free(str);
}

str_t* str_new(const char* str) {
  str_t* s = malloc(sizeof(str_t));
  u32 size = strlen(str) + 1;
  s->buf = malloc(size);
  s->buf_len = size;
  s->len = size - 1;
  strncpy(s->buf, str, strlen(str));
  return s;
}

void str_append(str_t* str, const char* text) {
  u32 prev_len = str->len;
  u32 needed = strlen(text) + prev_len + 1;
  if (needed > str->buf_len) {
    char* new_buf = malloc(2 * needed);

    strncpy(new_buf, str->buf, str->buf_len);
    new_buf[str->buf_len] = 0;
    // free(str->buf);

    str->buf = new_buf;
    str->buf_len = 2 * needed;
  }

  strncpy(&str->buf[prev_len], text, strlen(text) + 1);
  str->len = needed - 1;
}

void send_input(str_t* input_buf) {
  char* cmd = input_buf->buf;
  if (cmd == NULL) {
    printf("input buf cmd is null\n");
    return;
  }
  printf("send ptm command %s\n", cmd);
  int len = strlen(cmd);
  write(fd_ptm, cmd, len);
}

void interpret_cmd(char* cmd) {
  fprintf(file_out, "interpret cmd: %s\n", cmd);
  if(strlen(cmd)<=0){
    return;
  }

  if (!strcmp(cmd, "exit")) {
    running = false;
    return;
  }

  if(strcmp("help",cmd)==0){
    printf("welcome to yiyiya os,have fun! ^_^!!\n");
    return;
  }else if(strcmp("ls",cmd)==0){
    printf("ls not support ^_^!!\n");
    return;
  }else{
    printf("not support ^_^!!\n");
    return;
  }

  u32 n_args = 0;
  char** args = NULL;
  char* next = cmd;

  while (*next) {
    args = realloc(args, (++n_args + 1) * sizeof(char*));
    while (isspace(*next)) {
      next++;
    }
    u32 n = (u32)strchrnul(next, ' ') - (u32)next;
    args[n_args - 1] = strndup(next, n);
    args[n_args] = NULL;
    fprintf(file_out, "arg[%d]: %s\n", n_args - 1, args[n_args - 1]);
    next = strchrnul(next, ' ');
  }

  if (!args) {
    fprintf(file_out, "no args\n");
    return;
  }
  int32_t ret = 0;
  char buf[128];
  sprintf(buf, "/%s", args[0]);
  // ret = execl(buf, args);
  // fprintf(file_out, "exec ret=%d\n", ret);
  // if (ret < 0) {
  //   printf("exec error\n");
  // } else {
  //   printf("\n");
  // }

  while (args && *args) {
    // free(*args);
    args++;
    fprintf(file_out, "free args\n");
  }
  // free(args);
  fprintf(file_out, "cmd end\n");
}

char* scroll_view(char* str) {
  char* lf = strchrnul(str, '\n');
  u32 line_len = (u32)(lf - str);

  if (line_len <= max_col) {
    return lf + 1;
  }

  return str + max_col;
}

u32 count_lines(str_t* str) {
  char* text_view = str->buf;
  u32 n_lines = 0;

  while (text_view < &str->buf[str->len]) {
    char* lf = strchrnul(text_view, '\n');
    u32 line_len = (u32)(lf - text_view);

    if (line_len <= max_col) {
      text_view += line_len + 1;  // Discard the line feed
    } else {
      text_view += max_col;
    }

    n_lines += 1;
  }

  return n_lines;
}

void redraw(EtkWidget* thiz, str_t* text_buf, const str_t* input_buf) {
  EtkWindowClass* priv = (EtkWindowClass*)thiz->subclass;
  EtkRect t = priv->head_rect;
  EtkCanvas* can = thiz->canvas;
  EtkRect r = thiz->rect;
  if (thiz->canvas != NULL) {
    etk_canvas_fill_rect(thiz->canvas, 0, t.height, r.width, r.height,
                         bg_color);
  }

  u32 y = 20;
  str_append(text_buf, input_buf->buf);
  if (cursor) {
    str_append(text_buf, "_");
  }

  char line_buf[max_col + 1];
  char* text_view = text_buf->buf;
  u32 n_lines = count_lines(text_buf);

  // Scroll the buffer as needed
  if (n_lines > max_line) {
    for (u32 i = 0; i < n_lines - max_line; i++) {
      text_view = scroll_view(text_view);
    }
  }

  // Draw line by line, wrapping text
  while (text_view < &text_buf->buf[text_buf->len]) {
    char* lf = strchrnul(text_view, '\n');
    u32 line_len = (u32)(lf - text_view);

    if (line_len <= max_col) {
      strncpy(line_buf, text_view, line_len);
      line_buf[line_len] = '\0';
      text_view += line_len + 1;  // Discard the line feed
    } else {
      strncpy(line_buf, text_view, max_col);
      line_buf[max_col] = '\0';
      text_view += max_col;
    }
    // snow_draw_string(win->fb, line_buf, margin, y, text_color);
    etk_canvas_draw_string_with_color(can, margin, y, line_buf, text_color,
                                      bg_color);

    y += char_height;
  }

  // De-concatenate
  text_buf->buf[text_buf->len - input_buf->len] = '\0';
  text_buf->len -= input_buf->len;

  if (cursor) {
    text_buf->buf[text_buf->len - 1] = '\0';
    text_buf->len -= 1;
  }

  // Update the window
  //   snow_render_window(win);
}

Ret etk_terminal_paint(EtkWidget* thiz) {
  if (thiz->canvas == NULL || thiz->canvas->draw_rect == NULL) {
    return;
  }
  // thiz->canvas->draw_rect(thiz->canvas, 0, 0, thiz->rect.width,
  //                         thiz->rect.height, 0x00353535);
  redraw(thiz, g_text_buf, g_input_buf);
}

int etk_terminal_get_cmd_result() {
  u32 nread;
  u8 anything_read = false;
  memset(ret_buf, 0, ret_buf_size);
  for (int i = 0; i < 1; i++) {
    // fprintf(file_out,"start read from ptm\n");
    printf("start read\n");
    nread = read(fd_ptm, ret_buf, ret_buf_size);
    if (nread >= 0) {
      ret_buf[nread] = '\0';
      printf("read from ptm\n");
      // fprintf(file_out,"read from ptm size:%d result:%s\n", nread, ret_buf);
      str_append(g_text_buf, "\n");
      str_append(g_text_buf, ret_buf);
      anything_read = true;
      break;
    } else {
      str_append(g_text_buf, "\n");
      break;
    }
  }
  if (anything_read) {
    str_append(g_text_buf, prompt);
  }
  // fprintf(file_out,"get cmd result end\n");
  return anything_read;
}

void etk_terminal_do_cmd() {
  printf("fork child\n");
  u32 pts = ioctl(fd_ptm, IOC_SLAVE);
  char buf[256];
  u32 len = 256;
  memset(buf, 0, 256);
  sprintf(buf, "/dev/pts/%d", pts);
  fd_pts = open(buf, "r");
  if (fd_pts < 0) {
    printf("error get pts %d\n", pts);
  }
  printf("ptm %d pts %d\n", fd_ptm, fd_pts);
  fd_out = dup(STDOUT_FILENO);
  file_out = fdopen(fd_out, "w");
  printf("fd out=%d\n", fd_out);
  dup2(fd_pts, STDIN_FILENO);
  dup2(fd_pts, STDOUT_FILENO);
  dup2(fd_pts, STDERR_FILENO);

  for (;;) {
    memset(buf, 0, len);
    int ret = read(fd_pts, buf, len);
    if (ret >= 0) {
      interpret_cmd(buf);
    }
  }
}

static Ret etk_terminal_event(EtkWidget* thiz, EtkEvent* event) {
  EtkWindowClass* priv = NULL;
  switch (event->type) {
    case ETK_EVENT_KEY_DOWN: {
      u16 key = event->u.key.code;
      printf("key down key:%x\n", key);
      switch (key) {
        case 0x0a:  // enter
        case 0x0d:
          str_append(g_text_buf, g_input_buf->buf);
          send_input(g_input_buf);
          if (g_input_buf->len > 0) {
            u32 cmdret = etk_terminal_get_cmd_result();
            if (cmdret > 0) {
              printf("cmd ret:%d text:%s\n", cmdret, g_text_buf->buf);
            }
          }
          if (g_input_buf != NULL) {
            g_input_buf->buf[0] = '\0';
            g_input_buf->len = 0;
          }

          break;
        case 0x08:  // delete
          if (g_input_buf->len) {
            g_input_buf->buf[g_input_buf->len - 1] = '\0';
            g_input_buf->len -= 1;
          }
          break;
        default:
          if (key < 0xf0) {
            char str[2] = "\0\0";
            str[0] = key;
            str_append(g_input_buf, str);
          }
          break;
      }
    } break;
    default:
      break;
  }

  etk_terminal_paint(thiz);
  etk_widget_update_rect(thiz, &thiz->rect);
  u32 ret = etk_window_on_event(thiz, event);
  return ret;
}

EtkWidget* etk_terminal(e32 x, e32 y, e32 width, e32 height) {
  EtkWidget* thiz = NULL;
  EtkWindowClass* priv = NULL;
  thiz = etk_create_window(x, y, width, height, ETK_WIDGET_WINDOW);
  etk_widget_set_text(thiz, "terminal");
  priv = (EtkWindowClass*)thiz->subclass;
  priv->has_head = 1;
  thiz->event = etk_terminal_event;
  thiz->paint = etk_terminal_paint;

  g_text_buf = str_new(prompt);
  g_input_buf = str_new("");
  cursor = true;

  max_col = twidth / char_width - 1;
  max_line = theight / char_height - 1;

  fd_ptm = open("/dev/ptm", "r");
  if (fd_ptm < 0) {
    printf("error get ptm \n");
  }

  pid_t pid;
  pid = fork();
  if (pid != 0) {  // parent
    printf("fork parent\n");
    ioctl(fd_ptm, IOC_MASTER_READ_NOBLOCK);
  } else {  //  child
    etk_terminal_do_cmd();
  }
  return thiz;
}