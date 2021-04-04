#include "terminal.h"
#include "ioctl.h"

#define IOC_PTY_MAGIC 'p'

#define IOC_SLAVE _IOW(IOC_PTY_MAGIC, 1, int)
#define IOC_READ_OFFSET _IOW(IOC_PTY_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_PTY_MAGIC, 4, int)

typedef struct {
  char* buf;
  uint32_t buf_len;
  uint32_t len;
} str_t;

const char* prompt = "yiyiya$ ";
bool running = true;
bool cursor = true;
bool focused = true;
str_t* text_buf;
str_t* input_buf;
uint32_t last_time = 0;

u32 fd_ptm;
u32 fd_pts;

const uint32_t twidth = 550;
const uint32_t theight = 342;
const uint32_t char_width = 8;
const uint32_t char_height = 16;
const uint32_t max_col = twidth / char_width - 1;
const uint32_t max_line = theight / char_height - 1;

const uint32_t margin = 1;
const uint32_t text_color = 0xE0E0E0;
const float cursor_blink_time = 1;

void str_free(str_t* str) {
  free(str->buf);
  free(str);
}

str_t* str_new(const char* str) {
  str_t* s = malloc(sizeof(str_t));
  uint32_t size = strlen(str) + 1;
  s->buf = malloc(size);
  s->buf_len = size;
  s->len = size - 1;
  strcpy(s->buf, str);
  return s;
}

void str_append(str_t* str, const char* text) {
  uint32_t prev_len = str->len;
  uint32_t needed = strlen(text) + prev_len + 1;

  if (needed > str->buf_len) {
    char* new_buf = malloc(2 * needed);

    strcpy(new_buf, str->buf);
    free(str->buf);

    str->buf = new_buf;
    str->buf_len = 2 * needed;
  }

  strcpy(&str->buf[prev_len], text);
  str->len = needed - 1;
}

void interpret_cmd(str_t* text_buf, str_t* input_buf) {
  if (!strcmp(input_buf->buf, "exit")) {
    running = false;
    return;
  }

  char* cmd = input_buf->buf;
  uint32_t n_args = 0;
  char** args = NULL;
  char* next = cmd;

  while (*next) {
    args = realloc(args, (++n_args + 1) * sizeof(char*));

    while (isspace(*next)) {
      next++;
    }

    uint32_t n = strchrnul(next, ' ') - next;
    args[n_args - 1] = strndup(next, n);
    args[n_args] = NULL;

    next = strchrnul(next, ' ');
  }

  if (!args) {
    return;
  }
  int32_t ret = 0;
  //   int32_t ret = syscall2(SYS_EXEC, (uintptr_t)args[0], (uintptr_t)args);

  while (args && *args) {
    free(*args);
    args++;
  }

  free(args);

  if (ret != 0) {
    str_append(text_buf, "invalid command: ");
    str_append(text_buf, cmd);
    str_append(text_buf, "\n");
  }
}

char* scroll_view(char* str) {
  char* lf = strchrnul(str, '\n');
  uint32_t line_len = (uint32_t)(lf - str);

  if (line_len <= max_col) {
    return lf + 1;
  }

  return str + max_col;
}

uint32_t count_lines(str_t* str) {
  char* text_view = str->buf;
  uint32_t n_lines = 0;

  while (text_view < &str->buf[str->len]) {
    char* lf = strchrnul(text_view, '\n');
    uint32_t line_len = (uint32_t)(lf - text_view);

    if (line_len <= max_col) {
      text_view += line_len + 1;  // Discard the line feed
    } else {
      text_view += max_col;
    }

    n_lines += 1;
  }

  return n_lines;
}

void redraw(EtkCanvas* can, str_t* text_buf, const str_t* input_buf) {
  /* Window decorations */

  // background
  //   snow_draw_rect(win->fb, 0, 0, win->width, win->height, 0x00353535);
  // title bar
  //   snow_draw_rect(win->fb, 0, 0, win->width, 20, 0x00222221);
  //   snow_draw_border(win->fb, 0, 0, win->width, 20, 0x00000000);
  //   snow_draw_string(win->fb, win->title, 4, 3, 0x00FFFFFF);
  // border of the whole window
  //   snow_draw_border(win->fb, 0, 0, win->width, win->height, 0x00555555);

  /* Text content */

  uint32_t y = 22;  // below title bar

  // Temporarily concatenate the input and a cursor
  str_append(text_buf, input_buf->buf);

  if (cursor) {
    str_append(text_buf, "_");
  }

  char line_buf[max_col + 1];
  char* text_view = text_buf->buf;
  uint32_t n_lines = count_lines(text_buf);

  // Scroll the buffer as needed
  if (n_lines > max_line) {
    for (uint32_t i = 0; i < n_lines - max_line; i++) {
      text_view = scroll_view(text_view);
    }
  }

  // Draw line by line, wrapping text
  while (text_view < &text_buf->buf[text_buf->len]) {
    char* lf = strchrnul(text_view, '\n');
    uint32_t line_len = (uint32_t)(lf - text_view);

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
    etk_canvas_draw_string_with_color(can, margin, y, line_buf, text_color, 0);

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

static Ret etk_terminal_event(EtkWidget* thiz, EtkEvent* event) {
  EtkWindowClass* priv = NULL;
  //printf("%d\n", (int)event->type);
 
  bool needs_redrawing = false;
  switch (event->type) {
    case ETK_EVENT_KEY_DOWN: {
      const uint32_t buf_size = 256;
      char buf[buf_size];
      uint32_t read;
      bool anything_read = false;

      while ((read = fread(buf, 1, buf_size - 1, fd_pts))) {
        buf[read] = '\0';
        str_append(text_buf, buf);
        needs_redrawing = true;
        anything_read = true;
      }

      if (anything_read) {
        str_append(text_buf, prompt);
      }

      u16 key = event->u.key.code;
      //printf("key down %x\n", key);

      switch (key) {
        case 0x1c:
        // case 0x02:
          str_append(text_buf, input_buf->buf);
          interpret_cmd(text_buf, input_buf);
          printf("\n");
          input_buf->buf[0] = '\0';
          input_buf->len = 0;
          break;
        // case 0x01:
        //   if (input_buf->len) {
        //     input_buf->buf[input_buf->len - 1] = '\0';
        //     input_buf->len -= 1;
        //   }
        //   break;
        default:
          if (key < 0x1c) {
            char str[2] = "\0\0";
            str[0] = key;
            str_append(input_buf, str);
          }
          break;
      }
    } break;

    case ETK_EVENT_WND_DESTROY:
    //   printf("destroy!\n");
    //   priv = (EtkWindowClass*)thiz->subclass;
    //   etk_sources_manager_remove(etk_default_sources_manager(),
    //                              (EtkSource*)priv->data[1]);
      break;
    default:
      break;
  }

  return etk_window_on_event(thiz, event);
}

Ret etk_terminal_paint(EtkWidget* thiz) {
  thiz->canvas->draw_rect(thiz->canvas, 0, 0, thiz->rect.width,
                          thiz->rect.height, 0x00353535);
  redraw(thiz->canvas, text_buf, input_buf);
}

EtkWidget* etk_terminal(e32 x, e32 y, e32 width, e32 height) {
  EtkWidget* thiz = NULL;
  EtkWindowClass* priv = NULL;
  thiz = etk_create_window(x, y, width, height, ETK_WIDGET_WINDOW);
  etk_widget_set_text(thiz, "terminal");
  priv = (EtkWindowClass*)thiz->subclass;
  // priv->has_head = 1;
  thiz->event = etk_terminal_event;
  thiz->paint = etk_terminal_paint;

  text_buf = str_new(prompt);
  input_buf = str_new("");
  cursor = true;

  fd_ptm=open("/dev/ptm","r");
  if(fd_ptm<0){
    printf("error get ptm \n");
  }
  u32 pts=ioctl(fd_ptm,IOC_SLAVE);
  char buf[20];
  sprintf(buf,"/dev/pts/%d",pts);
  fd_pts=open(buf,"r");
  if(fd_pts<0){
    printf("error get pts \n");
  }
  printf("ptm %d pts %d\n",fd_ptm,fd_pts);

  char* buf2="hello";
  int len=strlen(buf2);
  write(fd_ptm,buf2,len);
  memset(buf2,0,len);
  read(fd_pts,buf2,len);
  printf("read from ptm: %s\n",buf2);
  return thiz;
}