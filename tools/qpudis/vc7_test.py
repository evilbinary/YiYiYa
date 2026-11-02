#!/usr/bin/env python3
"""验证 py-videocore7 汇编器能产出机器码。"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from vc7.assembler import Assembly, qpu, assemble  # noqa: E402


@qpu
def tiny(asm: Assembly) -> None:
    mov(tmua, rf[10])           # noqa: F821
    nop()                       # noqa: F821
    nop()                       # noqa: F821
    nop(sig=ldtmu(rf[28]))      # noqa: F821
    mov(tmud, rf[28])           # noqa: F821
    mov(tmua, rf[11])           # noqa: F821
    nop(sig=thrsw)             # noqa: F821
    nop()                       # noqa: F821
    nop()                       # noqa: F821


code = assemble(tiny)
print('words:', len(code))
for i, w in enumerate(code):
    print('  [%2d] 0x%016x' % (i, w))
