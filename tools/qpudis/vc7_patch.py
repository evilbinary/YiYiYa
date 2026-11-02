#!/usr/bin/env python3
"""把 py-videocore7 的汇编器降级到 Python 3.11 可跑（去掉 PEP 695 语法）。

用法：把 upstream 的 src/_videocore7/*.py 拷到本文件的同级目录 vc7/ 下，
然后运行本脚本原地改写。

只改语法、不改语义——目的是拿它的 assembler 生成机器码，
不在本项目里跑它的 driver。
"""
import io
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'vc7')


def patch_file(path):
    s = io.open(path, encoding='utf-8').read()
    orig = s

    # 1) type X = ...   →  X = ...
    s = re.sub(r'^type\s+(\w+)\s*=\s*', lambda m: '%s = ' % m.group(1), s,
               flags=re.M)

    # 2) def f[**P, R](...)  →  def f(...)，并把 P.args/P.kwargs 展开
    s = re.sub(r'def\s+(\w+)\[?\*\*P,\s*R\]?\(\s*'
               r'(\w+):\s*Callable\[Concatenate\[Assembly,\s*P\],\s*R\]',
               lambda m: 'def %s(%s' % (m.group(1), m.group(2)), s)
    s = s.replace(', *args: P.args, **kwargs: P.kwargs)', ', *args, **kwargs)')
    s = s.replace(') -> Callable[Concatenate[Assembly, P], R]:', '):')
    s = s.replace(') -> list[int]:', '):')
    s = s.replace(') -> Assembly:', '):')

    if s != orig:
        io.open(path, 'w', encoding='utf-8').write(s)
        return True
    return False


def main():
    if not os.path.isdir(SRC):
        raise SystemExit('missing %s' % SRC)
    n = 0
    for f in sorted(os.listdir(SRC)):
        if f.endswith('.py'):
            if patch_file(os.path.join(SRC, f)):
                n += 1
                print('patched', f)
    print('total %d file(s)' % n)


if __name__ == '__main__':
    main()
