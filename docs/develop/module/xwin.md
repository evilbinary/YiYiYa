# XWIN 窗口系统开发文档

XWIN 是 Duck Kernel 的轻量级窗口系统，提供窗口管理、图形绘制、事件处理和渲染合成功能。

## 模块结构

```
duck/modules/xwin/
├── ya.py            # 构建配置
├── xwin.h           # 核心头文件 (数据结构 + API 声明)
├── xwin.c           # 核心实现 (初始化、窗口管理)
├── xgraphics.c      # 图形绘制 (像素、线条、形状、文本)
├── xevent.c         # 事件系统 (事件队列、分发)
├── xrender.c        # 渲染引擎 (双缓冲、合成、光标)
├── xwm.c            # 窗口管理器 (装饰、标题栏)
├── xinput.c         # 输入集成 (键盘/鼠标驱动对接)
├── xwin_syscall.c   # 系统调用处理
├── xwin_user.h      # 用户空间 API 头文件
├── font.c           # 完整字体数据 (8x8, 8x16)
└── font.h           # 字体接口定义
```

## 核心数据结构

### 窗口 (xwindow_t)

```c
typedef struct xwindow {
    u32 id;                    // 窗口ID
    char title[64];            // 标题
    i32 x, y;                  // 位置 (相对于父窗口)
    i32 abs_x, abs_y;          // 绝对位置 (屏幕坐标)
    u32 width, height;         // 尺寸
    u32* framebuffer;          // 窗口缓冲区 (ARGB)
    u32 zorder;                // Z序
    u32 flags;                 // 标志
    u32 bg_color;              // 背景色
    
    // 窗口树结构
    xwindow_t* parent;
    xwindow_t* first_child;
    xwindow_t* next_sibling;
    
    // 回调
    void (*on_event)(xwindow_t* win, void* event);
    void* user_data;
} xwindow_t;
```

### 显示服务器 (xdisplay_t)

```c
typedef struct xdisplay {
    vga_device_t* vga;         // 底层显示设备
    u32* screen_buffer;        // 屏幕缓冲
    u32* back_buffer;          // 后备缓冲 (双缓冲)
    
    xwindow_t* root_window;    // 根窗口 (桌面)
    xwindow_t* focused_window; // 焦点窗口
    xwindow_t** windows;       // 窗口数组 (按zorder)
    u32 window_count;
    
    queue_pool_t* event_queue; // 事件队列
    
    // 鼠标状态
    i32 mouse_x, mouse_y;
    u32 mouse_buttons;
} xdisplay_t;
```

### 事件 (xevent_t)

```c
typedef struct xevent {
    u32 type;                  // 事件类型
    u32 window_id;             // 目标窗口ID
    u32 time;                  // 时间戳
    union {
        struct { i32 x, y; u32 button; } mouse;
        struct { u32 keycode; u32 pressed; char ch; } key;
        struct { u32 width, height; } resize;
    } data;
} xevent_t;
```

## 窗口标志

| 标志 | 值 | 说明 |
|------|-----|------|
| `XWIN_FLAG_VISIBLE` | 0x01 | 窗口可见 |
| `XWIN_FLAG_FOCUSABLE` | 0x02 | 可获取焦点 |
| `XWIN_FLAG_BORDERED` | 0x04 | 有边框 |
| `XWIN_FLAG_DRAGGABLE` | 0x08 | 可拖拽 |
| `XWIN_FLAG_RESIZABLE` | 0x10 | 可调整大小 |
| `XWIN_FLAG_ROOT` | 0x80 | 根窗口 |

## 事件类型

| 类型 | 说明 |
|------|------|
| `XEVENT_MOUSE_MOVE` | 鼠标移动 |
| `XEVENT_MOUSE_DOWN` | 鼠标按下 |
| `XEVENT_MOUSE_UP` | 鼠标释放 |
| `XEVENT_MOUSE_WHEEL` | 滚轮 |
| `XEVENT_KEY_DOWN` | 键盘按下 |
| `XEVENT_KEY_UP` | 键盘释放 |
| `XEVENT_EXPOSE` | 重绘请求 |
| `XEVENT_RESIZE` | 窗口调整大小 |
| `XEVENT_MOVE` | 窗口移动 |
| `XEVENT_CLOSE` | 窗口关闭 |
| `XEVENT_FOCUS_IN` | 获得焦点 |
| `XEVENT_FOCUS_OUT` | 失去焦点 |

## 内核 API

### 窗口管理

```c
// 初始化显示服务器
int xwin_init(xdisplay_t* disp, vga_device_t* vga);

// 创建/销毁窗口
xwindow_t* xwin_create_window(xdisplay_t* disp, xwindow_t* parent,
                              i32 x, i32 y, u32 width, u32 height, u32 flags);
void xwin_destroy_window(xdisplay_t* disp, xwindow_t* win);

// 窗口操作
void xwin_move(xdisplay_t* disp, xwindow_t* win, i32 x, i32 y);
void xwin_resize(xdisplay_t* disp, xwindow_t* win, u32 w, u32 h);
void xwin_raise(xdisplay_t* disp, xwindow_t* win);
void xwin_lower(xdisplay_t* disp, xwindow_t* win);
void xwin_show(xdisplay_t* disp, xwindow_t* win, int show);
void xwin_set_focus(xdisplay_t* disp, xwindow_t* win);
```

### 图形绘制

```c
// 基本图形
void xwin_draw_pixel(xwindow_t* win, i32 x, i32 y, u32 color);
void xwin_draw_line(xwindow_t* win, i32 x1, i32 y1, i32 x2, i32 y2, u32 color);
void xwin_draw_rect(xwindow_t* win, i32 x, i32 y, u32 w, u32 h, u32 color);
void xwin_fill_rect(xwindow_t* win, i32 x, i32 y, u32 w, u32 h, u32 color);

// 圆形/椭圆
void xwin_draw_circle(xwindow_t* win, i32 cx, i32 cy, u32 r, u32 color);
void xwin_fill_circle(xwindow_t* win, i32 cx, i32 cy, u32 r, u32 color);
void xwin_draw_ellipse(xwindow_t* win, i32 x, i32 y, u32 w, u32 h, u32 color);

// 圆角矩形
void xwin_draw_round_rect(xwindow_t* win, i32 x, i32 y, u32 w, u32 h, u32 r, u32 color);
void xwin_fill_round_rect(xwindow_t* win, i32 x, i32 y, u32 w, u32 h, u32 r, u32 color);

// 文本
void xwin_draw_text(xwindow_t* win, i32 x, i32 y, const char* text, u32 color);
void xwin_draw_char(xwindow_t* win, i32 x, i32 y, char ch, u32 color, u32 size);

// 清除
void xwin_clear(xwindow_t* win);
void xwin_clear_color(xwindow_t* win, u32 color);
```

### 事件处理

```c
void xwin_process_events(xdisplay_t* disp);
int xwin_next_event(xdisplay_t* disp, xevent_t* event);
void xwin_send_event(xdisplay_t* disp, xwindow_t* win, xevent_t* event);

// 输入事件注入
void xwin_mouse_move(xdisplay_t* disp, i32 x, i32 y);
void xwin_mouse_button(xdisplay_t* disp, u32 button, u32 pressed);
void xwin_keyboard_event(xdisplay_t* disp, u32 keycode, u32 pressed, u32 mods);
```

### 渲染

```c
void xwin_render(xdisplay_t* disp);
void xwin_composite(xdisplay_t* disp);
void xwin_flip_buffer(xdisplay_t* disp);
void xwin_damage_all(xwindow_t* win);
```

## 用户空间 API

用户空间通过 syscall 调用窗口系统功能：

```c
#include "xwin_user.h"

// 创建窗口
xwin_handle_t win = xwin_create(100, 100, 400, 300, "My Window");

// 绘制
xwin_fill_rect(win, 10, 10, 100, 50, 0xFF0000);  // 红色矩形
xwin_draw_text(win, 20, 20, "Hello", 0xFFFFFF);   // 白色文本

// 事件循环
xwin_process_events();
xwin_render();
```

### Syscall 编号

| 编号 | 功能 |
|------|------|
| 0x5000 | xwin_create |
| 0x5001 | xwin_destroy |
| 0x5002 | xwin_move |
| 0x5003 | xwin_resize |
| 0x5004 | xwin_show |
| 0x5005 | xwin_set_title |
| 0x5006 | xwin_set_bg_color |
| 0x500A | xwin_clear |
| 0x500B | xwin_fill_rect |
| 0x500C | xwin_draw_rect |
| 0x500D | xwin_draw_line |
| 0x500E | xwin_draw_text |
| 0x5014 | xwin_get_event |
| 0x5015 | xwin_process_events |
| 0x5016 | xwin_render |

## 颜色定义

```c
#define XCOLOR_BLACK       0x00000000
#define XCOLOR_WHITE       0x00FFFFFF
#define XCOLOR_RED         0x00FF0000
#define XCOLOR_GREEN       0x0000FF00
#define XCOLOR_BLUE        0x000000FF
#define XCOLOR_YELLOW      0x00FFFF00
#define XCOLOR_CYAN        0x0000FFFF
#define XCOLOR_MAGENTA     0x00FF00FF
#define XCOLOR_GRAY        0x00808080
```

## 使用示例

### 内核模块中使用

```c
#include "modules/xwin/xwin.h"

void demo_window(void) {
    xdisplay_t display;
    xwin_init(&display, vga_get_device());
    
    // 创建窗口
    xwindow_t* win = xwin_create_window(&display, display.root_window,
                                        50, 50, 200, 150,
                                        XWIN_FLAG_VISIBLE | XWIN_FLAG_BORDERED);
    xwin_set_title(win, "Demo");
    xwin_set_bg_color(win, XCOLOR_WHITE);
    
    // 绘制内容
    xwin_fill_rect(win, 10, 10, 50, 50, XCOLOR_RED);
    xwin_draw_text(win, 70, 30, "Hello XWIN!", XCOLOR_BLACK);
    
    // 渲染
    xwin_render(&display);
    xwin_flip_buffer(&display);
}
```

### 用户程序中使用

```c
#define XWIN_USER_IMPL
#include "xwin_user.h"

int main(void) {
    xwin_handle_t win = xwin_create(100, 100, 320, 240, "Test");
    if (win == XWIN_INVALID_HANDLE) return -1;
    
    xwin_fill_rect(win, 0, 0, 320, 240, 0x404040);
    xwin_draw_text(win, 10, 10, "Hello World!", 0xFFFFFF);
    xwin_render();
    
    while (1) {
        xwin_process_events();
        // 处理事件...
    }
    
    xwin_destroy(win);
    return 0;
}
```

## 构建配置

模块依赖关系：

```
mod-xwin
  ├── kernel
  └── mod-vga
```

`ya.py` 配置：

```python
target("mod-xwin")
set_kind("static")

add_deps('kernel', 'mod-vga')

common_source=[
    'module.c', 'xwin.c', 'xgraphics.c', 'xevent.c',
    'xrender.c', 'xwm.c', 'font.c', 'xwin_syscall.c', 'xinput.c',
]

add_files(source+common_source)
add_includedirs('../')
```

## 性能优化

- **双缓冲渲染**: 避免闪烁，`back_buffer` → `screen_buffer`
- **损坏区域**: 只重绘变化区域 (`xwin_damage`)
- **Z序管理**: 按可见性排序，跳过被遮挡窗口

## 待完善功能

- [ ] 贝塞尔曲线绘制
- [ ] 多边形填充
- [ ] 渐变填充
- [ ] 硬件加速 (GPU)
- [ ] 窗口透明度 (Alpha 混合)
- [ ] 剪贴板支持
