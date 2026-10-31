# coding:utf-8
"""dock 图标生成器：把 weston 自带的 png 缩成面板能用的 24x24。

背景：desktop-shell 的面板图标按**原始尺寸**居中绘制
（clients/desktop-shell.c 的 panel_launcher_redraw_handler），而本平台
`weston.ini` 里 `[shell] panel-size=32`。上游 data 目录里的图标尺寸差别很大：

    terminal.png / icon_flower.png / icon_editor.png / icon_window.png  24x24 左右 ✓
    fullscreen.png / home.png / random.png / sidebyside.png / tiling.png 96x96
    wayland.png 128x128、icon_ivi_*.png 256x256、border.png 116x81

直接把 96/128/256 的图标写进 `[launcher] icon=` 会在 32px 高的面板里上下溢出
被裁。所以这里把它们统一缩成 24x24（保持宽高比、透明底居中），文件名加
`dock_` 前缀，和上游图标并存。

产物（默认输出到镜像里 /usr/share/weston 的源目录）：
    app/resource/usr/share/weston/dock_*.png

用法：
    python3 tools/gen_panel_icons.py            # 就地补/刷新
    python3 tools/gen_panel_icons.py --check    # 只检查缺失，不写文件

依赖 Pillow（和 tools/gen_dock_icons.py 一样）。图标只是镜像里的运行期资源，
缺失时 desktop-shell 会打 `ERROR loading icon from file ...` 并画一个"方框叉"
的兜底图标 —— 界面不会崩，但看着像渲染坏了。
"""
import os
import sys

try:
    from PIL import Image
except ImportError:  # pragma: no cover - 提示比 traceback 友好
    sys.exit('需要 Pillow：python3 -m pip install pillow')

SIZE = 24  # 必须 <= weston.ini 的 [shell] panel-size

# 目标名 -> 源 png（都在同一个 /usr/share/weston 目录里）
# 只挑语义对得上的：图标名本身就说明它画的是什么，别拿 A 的图标配 B 的程序
# （原来 `icon_editor.png` 配 weston-clickdot 就是这么来的：editor 要
# pangocairo + gobject 编不出来，只能借别的图标占位 —— 看着像"点开是编辑器"）。
MAP = [
    ('dock_shm', 'wayland.png'),               # wl-simple-shm
    ('dock_clickdot', 'icon_ivi_clickdot.png'),  # weston-clickdot（它自己的图标）
    ('dock_fullscreen', 'fullscreen.png'),     # weston-fullscreen
    ('dock_smoke', 'icon_ivi_smoke.png'),      # weston-smoke
    ('dock_stacking', 'tiling.png'),           # weston-stacking（层叠/平铺）
    ('dock_transformed', 'sidebyside.png'),    # weston-transformed
    ('dock_resizor', 'border.png'),            # weston-resizor（窗口边框）
    ('dock_tablet', 'random.png'),             # weston-tablet（笔迹）
]

# 上游没有专属图标、又不想借别人图标的：画一块纯色圆角块 + 白色字母。
# 字母只求"能区分"，不假装是官方图标。
#   Z = zoom（weston-scaler，viewporter 缩放）
#   P = pointer（weston-constraints，指针锁在窗口内）
LETTER_TILES = [
    ('dock_scaler', 'Z', (0x3f, 0x7f, 0x8f)),
    ('dock_constraints', 'P', (0xa3, 0x4f, 0x7f)),
]

# 5x7 位图字形（1 = 白点）：只为字母占位图服务，够用就行
GLYPHS = {
    'A': ('01110', '10001', '10001', '11111', '10001', '10001', '10001'),
    'B': ('11110', '10001', '10001', '11110', '10001', '10001', '11110'),
    'C': ('01111', '10000', '10000', '10000', '10000', '10000', '01111'),
    'D': ('11110', '10001', '10001', '10001', '10001', '10001', '11110'),
    'E': ('11111', '10000', '10000', '11110', '10000', '10000', '11111'),
    'F': ('11111', '10000', '10000', '11110', '10000', '10000', '10000'),
    'I': ('11111', '00100', '00100', '00100', '00100', '00100', '11111'),
    'K': ('10001', '10010', '10100', '11000', '10100', '10010', '10001'),
    'P': ('11110', '10001', '10001', '11110', '10000', '10000', '10000'),
    'R': ('11110', '10001', '10001', '11110', '10100', '10010', '10001'),
    'S': ('01111', '10000', '10000', '01110', '00001', '00001', '11110'),
    'T': ('11111', '00100', '00100', '00100', '00100', '00100', '00100'),
    'W': ('10001', '10001', '10001', '10101', '10101', '11011', '10001'),
    'X': ('10001', '10001', '01010', '00100', '01010', '10001', '10001'),
    'Z': ('11111', '00001', '00010', '00100', '01000', '10000', '11111'),
}

DEFAULT_DIR = os.path.join('app', 'resource', 'usr', 'share', 'weston')


def make_one(dst_dir, name, src_name):
    src = os.path.join(dst_dir, src_name)
    dst = os.path.join(dst_dir, name + '.png')
    if not os.path.exists(src):
        print('skip %s (no %s)' % (dst, src_name))
        return False
    im = Image.open(src)
    if im.mode != 'RGBA':
        im = im.convert('RGBA')
    im.thumbnail((SIZE, SIZE), Image.LANCZOS)
    canvas = Image.new('RGBA', (SIZE, SIZE), (0, 0, 0, 0))
    canvas.paste(im, ((SIZE - im.width) // 2, (SIZE - im.height) // 2))
    canvas.save(dst)
    print('-> %s (%dx%d)' % (dst, im.width, im.height))
    return True


def make_letter_tile(dst_dir, name, letter, color):
    """纯色圆角块 + 白色字母（超采样后缩小，边缘不发毛）。

    字形用自带 5x7 位图，不依赖系统字体文件：Pillow 的
    `ImageFont.load_default(size=...)` 只有 >=10.1 才支持，本机是 10.0.1，
    回退到默认位图字体时 11px 的字被缩到 1~2px ⇒ 画出来是空白的。
    """
    from PIL import ImageDraw

    glyph = GLYPHS[letter]
    ss = 8                      # 超采样倍数
    side = SIZE * ss
    im = Image.new('RGBA', (side, side), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if hasattr(d, 'rounded_rectangle'):
        d.rounded_rectangle([0, 0, side - 1, side - 1],
                            radius=side // 4, fill=color + (255,))
    else:  # 老 Pillow
        d.rectangle([0, 0, side - 1, side - 1], fill=color + (255,))

    cell = 2 * ss               # 字形每个像素的边长
    gw, gh = 5 * cell, 7 * cell
    ox, oy = (side - gw) // 2, (side - gh) // 2
    white = (255, 255, 255, 255)
    for gy, row in enumerate(glyph):
        for gx, bit in enumerate(row):
            if bit != '1':
                continue
            x0, y0 = ox + gx * cell, oy + gy * cell
            d.rectangle([x0, y0, x0 + cell - 1, y0 + cell - 1], fill=white)

    im.resize((SIZE, SIZE), Image.LANCZOS).save(
        os.path.join(dst_dir, name + '.png'))
    print('-> %s (letter %s)' % (os.path.join(dst_dir, name + '.png'), letter))
    return True


def main(argv):
    check = '--check' in argv
    dst_dir = DEFAULT_DIR
    for i, a in enumerate(argv):
        if a == '--dir' and i + 1 < len(argv):
            dst_dir = argv[i + 1]
    if check:
        names = [n for n, _ in MAP] + [n for n, _, _ in LETTER_TILES]
        missing = [n for n in names
                   if not os.path.exists(os.path.join(dst_dir, n + '.png'))]
        if missing:
            print('缺少 dock 图标：%s\n跑一遍 python3 tools/gen_panel_icons.py'
                  % ' '.join(missing))
            return 1
        print('dock 图标齐全')
        return 0
    ok = 0
    for name, src_name in MAP:
        if make_one(dst_dir, name, src_name):
            ok += 1
    for name, letter, color in LETTER_TILES:
        if make_letter_tile(dst_dir, name, letter, color):
            ok += 1
    print('done: %d/%d' % (ok, len(MAP) + len(LETTER_TILES)))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
