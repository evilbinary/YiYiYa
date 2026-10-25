# coding:utf-8
"""桌面图标生成器：dock 应用图标 + 顶栏状态图标，输出 24bpp BMP。

风格仿 macOS：
  · dock 图标是圆角方形（squircle 近似）+ 垂直渐变底 + 白色符号；
  · 圆角以外的角落填胶囊/顶栏底色（0x2e3440），所以在没有 alpha 的
    X11 桌面上看过去就像"悬浮的圆角图标"；
  · 顶栏状态图标是细线条单色（同底色，只有白色线条）。

产物：
  dock-term/dock-clock/dock-eyes/dock-win.bmp   36x36
  bar-wifi/bar-battery/bar-volume.bmp           18x18

用法：python3 tools/gen_dock_icons.py [输出目录]
默认输出到 app/resource/res/。
"""
import os
import sys

from PIL import Image, ImageDraw

N = 36          # dock 图标边长（像素）
BAR = 18        # 顶栏状态图标边长（像素，顶栏高 26）
S = 4           # 超采样倍数（画完再缩小，得到抗锯齿边缘）
BG = (0x2e, 0x34, 0x40)   # = DOCK_BG2 / BAR_BG，胶囊与顶栏底色
FG = (0xec, 0xef, 0xf4)   # 顶栏图标白


def new_canvas(size=N):
    im = Image.new('RGB', (size * S, size * S), BG)
    return im, ImageDraw.Draw(im)


def save(im, path, size=N):
    im.resize((size, size), Image.LANCZOS).convert('RGB').save(path)
    print('->', path)


def rounded_grad(im, box, radius, top, bottom):
    """在 im 上贴一个垂直渐变的圆角矩形（box 为超采样坐标）。"""
    x0, y0, x1, y1 = box
    w, h = int(x1 - x0), int(y1 - y0)
    grad = Image.new('RGB', (w, h))
    gd = ImageDraw.Draw(grad)
    for y in range(h):
        t = y / float(max(1, h - 1))
        gd.line([(0, y), (w, y)],
                fill=tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)))
    mask = Image.new('L', (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, w - 1, h - 1],
                                           radius=radius, fill=255)
    im.paste(grad, (int(x0), int(y0)), mask)


def body(top, bottom, inset=2, radius=7.5):
    """36 画布上的图标本体（32x32 圆角方块 + 渐变）。"""
    im, d = new_canvas()
    rounded_grad(im, (inset * S, inset * S, (N - inset) * S, (N - inset) * S),
                 radius * S, top, bottom)
    return im, d


# ---------------- dock 应用图标 ----------------

def icon_term():
    """终端：深灰渐变 + 白色 >_ """
    im, d = body((0x46, 0x4e, 0x5c), (0x22, 0x26, 0x2c))
    w = int(2.8 * S)
    d.line([(11 * S, 11.5 * S), (18 * S, 18 * S), (11 * S, 24.5 * S)],
           fill=FG, width=w, joint='curve')
    d.rounded_rectangle([20 * S, 22.6 * S, 26 * S, 24.6 * S],
                        radius=1 * S, fill=FG)
    return im


def icon_clock():
    """时钟：白色渐变表盘 + 深色圆环与指针"""
    im, d = body((0xfb, 0xfc, 0xff), (0xd6, 0xdd, 0xe8))
    ink = (0x2b, 0x30, 0x38)
    r = 10.4 * S
    d.ellipse([18 * S - r, 18 * S - r, 18 * S + r, 18 * S + r],
              outline=ink, width=int(1.5 * S))
    w = int(2.1 * S)
    d.line([(18 * S, 18 * S), (18 * S, 11.6 * S)], fill=ink, width=w)
    d.line([(18 * S, 18 * S), (23.2 * S, 20.4 * S)], fill=ink, width=w)
    r = int(1.4 * S)
    d.ellipse([18 * S - r, 18 * S - r, 18 * S + r, 18 * S + r], fill=ink)
    return im


def icon_eyes():
    """眼睛：蓝色渐变 + 两只白眼 + 深蓝瞳孔"""
    im, d = body((0x6c, 0xb4, 0xf2), (0x2c, 0x66, 0xc8))
    for cx in (12.5, 23.5):
        d.ellipse([(cx - 5.4) * S, 13.6 * S, (cx + 5.4) * S, 22.4 * S], fill=FG)
        r = 2.5
        d.ellipse([(cx - r) * S, (18 - r) * S, (cx + r) * S, (18 + r) * S],
                  fill=(0x1a, 0x3c, 0x74))
    return im


def icon_win():
    """窗口按钮：浅灰渐变 + 深色窗口轮廓、标题分隔线与两个控制点"""
    im, d = body((0xe9, 0xee, 0xf6), (0xc4, 0xcf, 0xe0))
    ink = (0x38, 0x40, 0x4e)
    d.rounded_rectangle([11 * S, 11.5 * S, 25 * S, 24.5 * S], radius=2.2 * S,
                        outline=ink, width=int(1.7 * S))
    d.line([(11 * S, 16.2 * S), (25 * S, 16.2 * S)], fill=ink,
           width=int(1.4 * S))
    for cx in (13.8, 16.4):
        r = 0.85 * S
        d.ellipse([cx * S - r, 13.4 * S - r, cx * S + r, 13.4 * S + r], fill=ink)
    return im


# ---------------- 顶栏状态图标（18x18，细线条） ----------------

def icon_wifi():
    """Wi-Fi：三段同心弧 + 底部圆点"""
    im, d = new_canvas(BAR)
    cx, cy = 9 * S, 15 * S
    w = int(1.45 * S)
    for r in (7, 4.8, 2.7):
        rr = r * S
        d.arc([cx - rr, cy - rr, cx + rr, cy + rr], 225, 315, fill=FG, width=w)
    r = int(1.15 * S)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=FG)
    return im


def icon_battery():
    """电池：圆角外壳 + 右侧触片 + 三格电量"""
    im, d = new_canvas(BAR)
    d.rounded_rectangle([2 * S, 5.6 * S, 13.6 * S, 12.4 * S], radius=1.8 * S,
                        outline=FG, width=int(1.15 * S))
    d.rounded_rectangle([14.2 * S, 8 * S, 15.8 * S, 10 * S], radius=0.5 * S,
                        fill=FG)
    for x in (3.9, 6.3, 8.7):
        d.rounded_rectangle([x * S, 7.2 * S, (x + 1.7) * S, 10.8 * S],
                            radius=0.4 * S, fill=FG)
    return im


def icon_volume():
    """音量：喇叭 + 两道声波"""
    im, d = new_canvas(BAR)
    d.polygon([(2 * S, 7.2 * S), (4.7 * S, 7.2 * S), (8 * S, 4.4 * S),
               (8 * S, 13.6 * S), (4.7 * S, 10.8 * S), (2 * S, 10.8 * S)], fill=FG)
    w = int(1.2 * S)
    for r in (3.1, 5.6):
        d.arc([(9.5 - r) * S, (9 - r) * S, (9.5 + r) * S, (9 + r) * S],
              -52, 52, fill=FG, width=w)
    return im


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        'app/resource/res')
    os.makedirs(out, exist_ok=True)
    for name, fn in (('term', icon_term), ('clock', icon_clock),
                     ('eyes', icon_eyes), ('win', icon_win)):
        save(fn(), os.path.join(out, 'dock-%s.bmp' % name))
    for name, fn in (('wifi', icon_wifi), ('battery', icon_battery),
                     ('volume', icon_volume)):
        save(fn(), os.path.join(out, 'bar-%s.bmp' % name), size=BAR)


if __name__ == '__main__':
    main()
