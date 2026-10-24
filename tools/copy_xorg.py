#!/usr/bin/env python3
"""copy_xorg.py — 从 raspios/debian arm64 镜像抽取 Xorg 服务器及依赖闭包，
写回 YiYiYa 的 image/disk.img（裸 FAT32）。

image/disk.img 被格式化后跑这个脚本即可恢复 Xorg 测试环境：
  python3 tools/copy_xorg.py --src ~/Downloads/2026-10-06-raspios-trixie-arm64.img

流程：
  1. 内置最小 ext4 只读解析（extent 映射 + symlink 跟随），不需要挂载/sudo
  2. 从根分区取固定清单 + ELF DT_NEEDED 传递闭包（FAT 无 symlink，解引用落盘）
  3. hdiutil 把 disk.img 挂到临时目录写入（macOS 原生 FAT），拷完清掉
     .Trashes/.fseventsd/._* 等 macOS 垃圾再卸载

--dry-run 只打印将拷贝的清单，不写盘。
"""
import argparse
import os
import posixpath
import struct
import subprocess
import sys
import tempfile
import time

SEC = 512

# ext4 特性
INCOMPAT_64BIT = 0x80
EXTENTS_FL = 0x80000

# dest 根下的目录树（整树拷贝，含子目录）
TREE_SRCS = ["/usr/lib/xorg", "/usr/share/X11"]
# 额外的单文件（缺了就跳过）
FILE_SRCS = ["/usr/bin/xsetroot", "/usr/bin/xkbcomp", "/usr/bin/xauth",
             "/etc/passwd", "/etc/group", "/lib/ld-linux-aarch64.so.1"]
# 库闭包的搜索路径（顺序即 musl/glibc 惯例）
LIB_DIRS = ["/lib/aarch64-linux-gnu", "/usr/lib/aarch64-linux-gnu",
            "/lib", "/usr/lib"]

# ---------------- 最小 X 桌面（--desktop）----------------
# 窗口管理器 openbox + 终端 lxterminal（GTK3）。两者都不是 Xorg 的依赖，
# 所以单独列一份：不开 --desktop 时镜像体积与原行为完全一致。
DESKTOP_FILES = ["/usr/bin/openbox", "/usr/bin/lxterminal"]
# GTK3 应用跑起来需要的数据文件，缺任意一类就会"能启动但看不见/没字"：
#   字体 + fontconfig 配置   → 有字（否则窗口只有空白）
#   gdk-pixbuf loaders       → PNG/XPM 解码（图标、光标、窗口装饰）
#   glib schemas             → GSettings 默认值（主题/字体/光标名从这里取）
#   themes/icons             → GTK 主题与图标
#   openbox/lxterminal 配置  → 窗口边框、终端默认字号/颜色
#   X11 字体                 → Xorg 光标字体（缺了会 (WW) 且默认光标缺失）
DESKTOP_TREES = [
    "/etc/fonts",
    "/usr/share/fonts/truetype/dejavu",
    "/usr/share/fonts/X11",
    "/usr/lib/aarch64-linux-gnu/gdk-pixbuf-2.0",
    "/usr/share/glib-2.0/schemas",
    "/usr/share/themes",
    "/usr/share/icons/hicolor",
    "/etc/xdg/openbox",
    "/etc/xdg/lxterminal",
]
# nsswitch.conf 用我们自己的最小内容（glibc 2.41 已内置 nss_files，
# 发行版默认的 "files systemd" 在缺 libnss_systemd 时会让 getpwnam 失败）
NSSWITCH = "passwd: files\ngroup: files\nshadow: files\ngai: files\n"


class Ext4:
    """只读 ext4：目录/普通文件/symlink，extent 映射。"""

    def __init__(self, path, part_off):
        self.f = open(path, "rb")
        self.off = part_off
        sb = self._read(1024, 1024)
        if struct.unpack_from("<H", sb, 56)[0] != 0xEF53:
            raise SystemExit("src root partition is not ext4")
        self.block = 1024 << struct.unpack_from("<I", sb, 24)[0]
        self.inode_size = struct.unpack_from("<H", sb, 88)[0] or 128
        self.incompat = struct.unpack_from("<I", sb, 96)[0]
        self.first_data_block = struct.unpack_from("<I", sb, 20)[0]
        self.ipg = struct.unpack_from("<I", sb, 40)[0]
        self.desc_size = struct.unpack_from("<H", sb, 254)[0]
        if not (self.incompat & INCOMPAT_64BIT) or self.desc_size < 32:
            self.desc_size = 32

    def _read(self, rel, size):
        self.f.seek(self.off + rel)
        d = self.f.read(size)
        if len(d) != size:
            raise IOError("short read")
        return d

    def _blk(self, no, size=None):
        return self._read(no * self.block, size or self.block)

    def _inode_raw(self, ino):
        g, idx = (ino - 1) // self.ipg, (ino - 1) % self.ipg
        d = self._blk(self.first_data_block + 1, self.desc_size * (g + 1))
        base = g * self.desc_size
        tbl = struct.unpack_from("<I", d, base + 8)[0]
        if self.incompat & INCOMPAT_64BIT:
            tbl |= struct.unpack_from("<I", d, base + 40)[0] << 32
        return self._blk(tbl, self.inode_size * (idx + 1))[
            idx * self.inode_size:(idx + 1) * self.inode_size]

    @staticmethod
    def mode(raw):
        return raw[0] | (raw[1] << 8)

    @staticmethod
    def size(raw):
        return struct.unpack_from("<I", raw, 4)[0]

    def _extent_map(self, buf, out):
        magic, entries, _mx, depth = struct.unpack_from("<HHHH", buf, 0)
        if magic != 0xF30A:
            raise IOError("bad extent magic")
        for i in range(entries):
            o = 12 + i * 12
            if depth == 0:
                ee_block, ee_len, hi, lo = struct.unpack_from("<IHHI", buf, o)
                start = (hi << 32) | lo
                for k in range(ee_len & 0x7FFF):
                    out[ee_block + k] = start + k
            else:
                _ei_block, leaf_lo, leaf_hi = struct.unpack_from("<IIH", buf, o)
                self._extent_map(self._blk((leaf_hi << 32) | leaf_lo), out)

    def _data(self, raw):
        size = self.size(raw)
        if size == 0:
            return b""
        if self.mode(raw) & 0xF000 == 0xA000:  # symlink
            if struct.unpack_from("<I", raw, 28)[0] == 0:  # fast symlink
                return bytes(raw[40:40 + size])
        m = {}
        if struct.unpack_from("<I", raw, 32)[0] & EXTENTS_FL:
            self._extent_map(raw[40:100], m)
        else:
            raise IOError("non-extent file not supported")
        out = bytearray()
        n = (size + self.block - 1) // self.block
        for b in range(n):
            blk = m.get(b)
            out += self._blk(blk) if blk else b"\0" * self.block
        return bytes(out[:size])

    def readlink(self, raw):
        return self._data(raw).decode(errors="replace")

    def readdir(self, raw):
        data = self._data(raw)
        ents = []
        o = 0
        while o + 8 <= len(data):
            ino, rec, nl, _ft = struct.unpack_from("<IHBB", data, o)
            if rec < 8:
                break
            if ino != 0 and nl > 0:
                ents.append((data[o + 8:o + 8 + nl].decode(errors="replace"),
                             ino))
            o += rec
        return ents

    def lookup(self, path, depth=0):
        """返回 (ino, inode_raw)；路径不存在返回 None；自动跟随 symlink"""
        if depth > 12:
            return None
        parts = [p for p in path.split("/") if p and p != "."]
        raw = self._inode_raw(2)
        i = 0
        while i < len(parts):
            if self.mode(raw) & 0xF000 != 0x4000:
                return None
            ents = dict(self.readdir(raw))
            nxt = ents.get(parts[i])
            if nxt is None:
                return None
            raw = self._inode_raw(nxt)
            if self.mode(raw) & 0xF000 == 0xA000:
                tgt = self.readlink(raw)
                if tgt.startswith("/"):
                    new = [q for q in tgt.split("/") if q]
                else:
                    new = parts[:i] + [q for q in tgt.split("/") if q]
                return self.lookup("/" + "/".join(new + parts[i + 1:]), depth + 1)
            i += 1
        return (2, raw) if not parts else (nxt, raw)

    def read_file(self, path):
        e = self.lookup(path)
        if e is None:
            return None
        return self._data(e[1])

    def walk(self, path):
        """递归产出 (相对路径, 内容)；目录以 ('d/', None) 顺序先出"""
        e = self.lookup(path)
        if e is None:
            raise SystemExit("src missing: " + path)
        yield "", e[1]
        stack = [(path, e[1], "")]
        while stack:
            p, raw, rel = stack.pop()
            for name, ino in self.readdir(raw):
                if name in (".", ".."):
                    continue
                child = self._inode_raw(ino)
                crel = posixpath.join(rel, name) if rel else name
                m = self.mode(child) & 0xF000
                if m == 0x4000:
                    yield "d" + crel, None
                    stack.append((posixpath.join(p, name), child, crel))
                elif m in (0x8000, 0xA000):
                    if m == 0xA000:
                        e = self.lookup(posixpath.join(p, name))
                        if e is None:
                            continue  # 悬空 symlink，跳过
                        child = self._inode_raw(e[0])
                    yield "f" + crel, self._data(child)


def elf_needed(data):
    """ELF64 的 DT_NEEDED 列表"""
    if data[:4] != b"\x7fELF" or data[4] != 2:
        return []
    e = "<" if data[5] == 1 else ">"
    phoff = struct.unpack_from(e + "Q", data, 32)[0]
    phentsize, phnum = struct.unpack_from(e + "HH", data, 54)
    loads, dyn = [], None
    for i in range(phnum):
        o = phoff + i * phentsize
        p_type = struct.unpack_from(e + "I", data, o)[0]
        p_off, p_vaddr = struct.unpack_from(e + "QQ", data, o + 8)
        p_filesz = struct.unpack_from(e + "Q", data, o + 32)[0]
        if p_type == 1:
            loads.append((p_vaddr, p_vaddr + p_filesz, p_off))
        elif p_type == 2:
            dyn = (p_off, p_filesz)
    if dyn is None:
        return []

    def v2o(v):
        for lo, hi, off in loads:
            if lo <= v < hi:
                return off + (v - lo)
        return None

    buf = data[dyn[0]:dyn[0] + dyn[1]]
    strtab = None
    o = 0
    while o + 16 <= len(buf):
        tag, val = struct.unpack_from(e + "QQ", buf, o)
        if tag == 0:
            break
        if tag == 5:  # DT_STRTAB
            strtab = v2o(val)
        o += 16
    if strtab is None:
        return []
    needed = []
    o = 0
    while o + 16 <= len(buf):
        tag, val = struct.unpack_from(e + "QQ", buf, o)
        if tag == 0:
            break
        if tag == 1:  # DT_NEEDED
            s = strtab + val
            epos = data.index(b"\0", s)
            needed.append(data[s:epos].decode())
        o += 16
    return needed


def lib_closure(ext4, elfs):
    """从给定 ELF 出发的 DT_NEEDED 传递闭包：{lib名: 内容}"""
    seen, order = {}, []
    work = list(elfs)
    while work:
        data = work.pop()
        for name in elf_needed(data):
            if name in seen:
                continue
            content = None
            for d in LIB_DIRS:
                content = ext4.read_file(posixpath.join(d, name))
                if content is not None:
                    break
            if content is None:
                print("  warn: missing lib %s" % name)
                continue
            seen[name] = content
            order.append(name)
            work.append(content)
    return seen, order


def fat_mount(disk):
    mnt = tempfile.mkdtemp(prefix="yiyiya_sd_")
    r = subprocess.run(["hdiutil", "attach", "-mountpoint", mnt,
                        "-noautoopen", "-nobrowse", disk],
                       capture_output=True, text=True)
    if r.returncode != 0 or not os.path.ismount(mnt):
        raise SystemExit("hdiutil attach failed:\n%s%s" %
                         (r.stdout, r.stderr))
    return mnt


def fat_detach(mnt):
    for _ in range(5):
        r = subprocess.run(["hdiutil", "detach", mnt, "-force"],
                           capture_output=True, text=True)
        if r.returncode == 0:
            return
        time.sleep(1)
    print("warn: detach failed, unmount manually:", mnt)


def fat_cleanup(root):
    """删 macOS 自动生成的垃圾（fatfs 读到 ._. 开头文件会当普通文件）"""
    for dirpath, dirnames, filenames in os.walk(root, topdown=False):
        for n in filenames:
            if n.startswith("._") or n == ".DS_Store":
                try:
                    os.remove(os.path.join(dirpath, n))
                except OSError:
                    pass  # macOS FSEvents 边拷边建，删除竞态容错
        for n in list(dirnames):
            if n in (".Trashes", ".fseventsd", ".Spotlight-V100", ".TemporaryItems"):
                subprocess.run(["rm", "-rf", os.path.join(dirpath, n)])
                dirnames.remove(n)


def copy_into(mnt, plan_trees, plan_files, libs, ext4):
    # 清掉旧版本脚本层级算错时误写到 FAT 根的残留（真目录在 usr/ 下）
    for stale in ("xorg", "X11"):
        p = os.path.join(mnt, stale)
        if os.path.isdir(p):
            subprocess.run(["rm", "-rf", p])

    def w(rel, content):
        dst = os.path.join(mnt, rel.lstrip("/"))
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        with open(dst, "wb") as f:
            f.write(content)

    n = 0
    for src in plan_trees:
        # 目标路径 = 源绝对路径去掉首'/'（保层级：/usr/lib/xorg -> usr/lib/xorg）
        base = src.lstrip("/")
        os.makedirs(os.path.join(mnt, base), exist_ok=True)
        for rel, content in ext4.walk(src):
            if rel == "":
                continue  # 树根本身
            r = rel[1:]  # 去掉 walk() 的 d/f 前缀（不能用 lstrip，会误伤）
            if content is None:
                os.makedirs(os.path.join(mnt, base, r), exist_ok=True)
            else:
                w(posixpath.join(base, r), content)
                n += 1
    for src in plan_files:
        content = ext4.read_file(src)
        if content is None:
            print("  skip missing", src)
            continue
        w(src, content)
        n += 1
    for name, content in libs.items():
        w(posixpath.join("/lib/aarch64-linux-gnu", name), content)
        n += 1
    w("/etc/nsswitch.conf", NSSWITCH.encode())
    os.makedirs(os.path.join(mnt, "tmp"), exist_ok=True)
    return n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default="/Users/evil/Downloads/"
                    "2026-10-06-raspios-trixie-arm64.img")
    ap.add_argument("--disk", default=os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "image/disk.img"))
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--desktop", action="store_true",
                    help="同时拷最小 X 桌面：openbox 窗口管理器 + lxterminal 终端"
                         "（含 GTK3 依赖闭包、字体、主题、图标、fontconfig）")
    args = ap.parse_args()

    if not os.path.exists(args.src):
        raise SystemExit("src image missing: " + args.src)
    with open(args.src, "rb") as f:
        f.seek(446)
        part_off = None
        for i in range(4):
            e = f.read(16)
            if len(e) < 16:
                break
            lba, cnt = struct.unpack("<II", e[8:16])
            ptype = e[4]
            if ptype == 0x83 and cnt:
                part_off = lba * SEC
                break
    if part_off is None:
        raise SystemExit("no Linux partition in src image")
    print("src root partition @ %d (%dMB)" % (part_off, part_off >> 20))

    ext4 = Ext4(args.src, part_off)

    # 1. 目录树清单
    trees = []
    want_trees = list(TREE_SRCS) + (DESKTOP_TREES if args.desktop else [])
    for src in want_trees:
        if ext4.lookup(src) is None:
            print("  warn: tree missing", src)
            continue
        trees.append(src)
    # 2. ELF：真实 server + 额外工具 → 依赖闭包
    seed = []
    server = ext4.read_file("/usr/lib/xorg/Xorg")
    if server is None:
        raise SystemExit("/usr/lib/xorg/Xorg not found in src")
    seed.append(server)
    want_elfs = ["/usr/bin/xsetroot", "/usr/bin/xkbcomp", "/usr/bin/xauth"]
    files = list(FILE_SRCS)
    if args.desktop:
        want_elfs += DESKTOP_FILES
        files += DESKTOP_FILES
    for src in want_elfs:
        c = ext4.read_file(src)
        if c is not None:
            seed.append(c)
        else:
            print("  warn: file missing", src)
    libs, order = lib_closure(ext4, seed)
    print("trees: %s" % ", ".join(trees))
    print("libs (%d): %s" % (len(order), " ".join(order)))
    total = sum(len(v) for v in libs.values())
    print("closure size: %.1fMB" % (total / 1048576.0))
    if args.desktop:
        # 数据文件（字体/主题/图标/gdk-pixbuf/配置）也要估一下，FAT 只有 512MB
        data = 0
        for src in trees:
            for rel, content in ext4.walk(src):
                if content:
                    data += len(content)
        print("data size: %.1fMB (total %.1fMB)"
              % (data / 1048576.0, (data + total) / 1048576.0))
    if args.dry_run:
        return

    # 3. 写回 FAT
    mnt = fat_mount(args.disk)
    try:
        n = copy_into(mnt, trees, files, libs, ext4)
        fat_cleanup(mnt)
        print("copied %d files into %s" % (n, args.disk))
    finally:
        fat_detach(mnt)


if __name__ == "__main__":
    main()
