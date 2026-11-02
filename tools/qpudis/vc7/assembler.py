# Copyright (c) 2016 Broadcom
# Copyright (c) 2025- Idein Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
import ctypes
import functools
from abc import ABC, abstractmethod
from collections.abc import Callable, Iterable
from types import TracebackType
from typing import Annotated, Concatenate, Final, Literal, Self, cast, overload

from .util import pack_unpack


class AssembleError(Exception):
    pass


class Assembly(list["Instruction"]):
    _labels: dict[str, int]
    _label_name_spaces: list[str]

    def __init__(self: Self):
        super().__init__()
        self._labels = {}
        self._label_name_spaces = []

    def gen_unused_label(self: Self, label_format: str = "{}") -> str:
        n: int = 0
        label = label_format.format(n)
        while self.gen_ns_label_name(label) in self._labels:
            n += 1
            next_label = label_format.format(n)
            assert label != next_label, "Bug: Invalid label format"
            label = next_label

        return label_format.format(n)

    def gen_ns_label_name(self: Self, name: str) -> str:
        return ".".join(self._label_name_spaces + [name])

    @property
    def labels(self: Self) -> dict[str, int]:
        return self._labels

    @property
    def label_name_spaces(self: Self) -> list[str]:
        return self._label_name_spaces


class LabelNameSpace:
    """Label namespace controller."""

    _asm: Assembly
    _name: str

    def __init__(self: Self, asm: Assembly, name: str) -> None:
        self._asm = asm
        self._name = name

    def __enter__(self: Self) -> Self:
        self._asm.label_name_spaces.append(self._name)
        return self

    def __exit__(
        self: Self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: type[TracebackType] | None,
    ) -> None:
        self._asm.label_name_spaces.pop()


class Label:
    _asm: Assembly

    def __init__(self: Self, asm: Assembly) -> None:
        self._asm = asm

    def __getattr__(self: Self, name: str) -> None:
        ns_name = self._asm.gen_ns_label_name(name)
        if ns_name in self._asm.labels:
            raise AssembleError(f"Label is duplicated: {name}")
        self._asm.labels[ns_name] = len(self._asm)


class Reference:
    _asm: Assembly
    _name: str | None

    def __init__(self: Self, asm: Assembly, name: str | None = None) -> None:
        self._asm = asm
        self._name = self._asm.gen_ns_label_name(name) if name is not None else None

    def __int__(self: Self) -> int:
        if self._name is None:
            raise AssembleError("Reference name is not specified")
        return self._asm.labels[self._name]


class ReferenceHelper:
    _asm: Assembly

    def __init__(self: Self, asm: Assembly) -> None:
        self._asm = asm

    def __getattr__(self: Self, name: str) -> Reference:
        return Reference(self._asm, name)


class OutputPackModifier:
    _name: Final[str]
    _f32: Final[Annotated[int | None, "Packed code of pack modifier from 32-bit float"]]

    def __init__(
        self: Self,
        name: str,
        f32: int | None = None,
    ):
        self._name = name
        self._f32 = f32

    @property
    def name(self: Self) -> str:
        return self._name

    @property
    def f32(self: Self) -> int | None:
        return self._f32


class InputUnpackModifier:
    _name: Final[str]
    _f32: Final[Annotated[int | None, "Packed code of unpack modifier to 32-bit float"]]
    _f16: Final[Annotated[int | None, "Packed code of unpack modifier to 16-bit float"]]
    _i32: Final[Annotated[int | None, "Packed code of unpack modifier to 32-bit integer"]]

    def __init__(
        self: Self,
        name: str,
        f32: int | None = None,
        f16: int | None = None,
        i32: int | None = None,
    ):
        self._name = name
        self._i32 = i32
        self._f16 = f16
        self._f32 = f32

    @property
    def name(self: Self) -> str:
        return self._name

    @property
    def f32(self: Self) -> int | None:
        return self._f32

    @property
    def f16(self: Self) -> int | None:
        return self._f16

    @property
    def i32(self: Self) -> int | None:
        return self._i32


class Register:
    OUTPUT_MODIFIER: Final[dict[str, OutputPackModifier]] = {
        "none": OutputPackModifier("none", f32=0),
        "l": OutputPackModifier("l", f32=1),
        "h": OutputPackModifier("h", f32=2),
    }

    INPUT_MODIFIER: Final[dict[str, InputUnpackModifier]] = {
        "abs": InputUnpackModifier("abs", f32=0),  # Absolute value.  Only available for some operations.
        "none": InputUnpackModifier("none", f32=1, f16=0, i32=0),
        "l": InputUnpackModifier("l", f32=2),  # Convert low 16 bits from 16-bit float to 32-bit float.
        "h": InputUnpackModifier("h", f32=3),  # Convert high 16 bits from 16-bit float to 32-bit float.
        "sat": InputUnpackModifier("sat", f32=4),  # Saturate 32-bit floating point to [0.0, 1.0]
        "nsat": InputUnpackModifier("nsat", f32=5),  # Saturate 32-bit floating point to [-1.0, 1.0]
        "max0": InputUnpackModifier("max0", f32=6),  # Saturate 32-bit floating point to [0.0, +inf]
        "r32": InputUnpackModifier("r32", f16=1),  # Convert to 16f and replicate it to the high bits.
        "rl2h": InputUnpackModifier("rl2h", f16=2),  # Replicate low 16 bits to high
        "rh2l": InputUnpackModifier("rh2l", f16=3),  # Replicate high 16 bits to low
        "swap": InputUnpackModifier("swap", f16=4),  # Swap high and low 16 bits
        "ul": InputUnpackModifier("ul", i32=1),  # Convert low 16 bits from 16-bit integer to unsigned 32-bit int
        "uh": InputUnpackModifier("uh", i32=2),  # Convert high 16 bits from 16-bit integer to unsigned 32-bit int
        "il": InputUnpackModifier("il", i32=3),  # Convert low 16 bits from 16-bit integer to signed 32-bit int
        "ih": InputUnpackModifier("ih", i32=4),  # Convert high 16 bits from 16-bit integer to signed 32-bit int
    }

    _name: str
    _magic: int
    _waddr: int
    _unpack_modifier: InputUnpackModifier
    _pack_modifier: OutputPackModifier

    def __init__(
        self: Self,
        name: str,
        magic: int,
        waddr: int,
        pack: str = "none",
        unpack: str = "none",
    ) -> None:
        self._name = name
        self._magic = magic
        self._waddr = waddr
        self._pack_modifier = Register.OUTPUT_MODIFIER[pack]
        self._unpack_modifier = Register.INPUT_MODIFIER[unpack]

    def pack(self: Self, modifier: str) -> "Register":
        if self._pack_modifier != Register.OUTPUT_MODIFIER["none"]:
            raise AssembleError("Conflict pack")
        return Register(self._name, self._magic, self._waddr, pack=modifier)

    def unpack(self: Self, modifier: str) -> "Register":
        if self._unpack_modifier != Register.INPUT_MODIFIER["none"]:
            raise AssembleError("Conflict unpack")
        return Register(self._name, self._magic, self._waddr, unpack=modifier)

    @property
    def name(self: Self) -> str:
        return self._name

    @property
    def magic(self: Self) -> int:
        return self._magic

    @property
    def waddr(self: Self) -> int:
        return self._waddr

    @property
    def unpack_modifier(self: Self) -> InputUnpackModifier:
        return self._unpack_modifier

    @property
    def pack_modifier(self: Self) -> OutputPackModifier:
        return self._pack_modifier


class Signal:
    _name: str
    _dst: Register | None

    def __init__(self: Self, name: str, dst: Register | None = None) -> None:
        self._name = name
        self._dst = dst

    @property
    def is_load(self: Self) -> bool:
        """True if load to destination register signal."""
        return self._dst is not None

    @property
    def name(self: Self) -> str:
        return self._name

    @property
    def dst(self: Self) -> Register:
        assert self._dst is not None
        return self._dst


SignalArg = Signal | list[Signal] | None


class LoadSignal:
    _name: str

    def __init__(self: Self, name: str) -> None:
        self._name = name

    def __call__(self: Self, dst: Register) -> Signal:
        return Signal(self._name, dst=dst)

    @property
    def name(self: Self) -> str:
        return self._name


class Signals(set[Signal]):
    def __init__(self: Self) -> None:
        super().__init__()

    def add(self: Self, sigs: LoadSignal | Signal | Iterable[LoadSignal | Signal]) -> None:
        match sigs:
            case LoadSignal() as ldsignal:
                raise AssembleError(f'"{ldsignal.name}" requires destination register (ex. "{ldsignal.name}(r0)")')
            case Signal() as signal:
                if signal.name in [s.name for s in self]:
                    raise AssembleError(f'Signal "{signal.name}" is duplicated')
                super().add(signal)
                if len([s for s in self if s.is_load]) > 1:
                    raise AssembleError("Too many signals that require destination register")
            case Iterable() as sigs:
                for sig in sigs:
                    self.add(sig)
            case _:
                raise RuntimeError("unreachable")

    def pack(self: Self) -> int:
        sigset = frozenset([sig.name for sig in self])
        valid_sigset = {
            frozenset(): 0,
            frozenset(["thrsw"]): 1,
            frozenset(["ldunif"]): 2,
            frozenset(["thrsw", "ldunif"]): 3,
            frozenset(["ldtmu"]): 4,
            frozenset(["thrsw", "ldtmu"]): 5,
            frozenset(["ldtmu", "ldunif"]): 6,
            frozenset(["thrsw", "ldtmu", "ldunif"]): 7,
            frozenset(["ldvary"]): 8,
            frozenset(["thrsw", "ldvary"]): 9,
            frozenset(["ldvary", "ldunif"]): 10,
            frozenset(["thrsw", "ldvary", "ldunif"]): 11,
            frozenset(["ldunifrf"]): 12,
            frozenset(["thrsw", "ldunifrf"]): 13,
            frozenset(["smimm_a"]): 14,
            frozenset(["smimm_b"]): 15,
            frozenset(["ldtlb"]): 16,
            frozenset(["ldtlbu"]): 17,
            frozenset(["wrtmuc"]): 18,
            frozenset(["thrsw", "wrtmuc"]): 19,
            frozenset(["ldvary", "wrtmuc"]): 20,
            frozenset(["thrsw", "ldvary", "wrtmuc"]): 21,
            frozenset(["ucb"]): 22,
            # 23 reserved
            frozenset(["ldunifa"]): 24,
            frozenset(["ldunifarf"]): 25,
            frozenset(["ldtmu", "wrtmuc"]): 26,
            frozenset(["thrsw", "ldtmu", "wrtmuc"]): 27,
            # 28 reserved
            # 29 reserved
            frozenset(["smimm_c"]): 30,
            frozenset(["smimm_d"]): 31,
        }
        if sigset in valid_sigset:
            return valid_sigset[sigset] << 53
        raise AssembleError(f"Invalid signal set: {sigset}")

    @property
    def is_load(self: Self) -> bool:
        return any(sig.is_load for sig in self)

    @property
    def write_address(self: Self) -> int:
        assert self.is_load
        dst = [sig.dst for sig in self if sig.is_load][0]
        return (dst.magic << 6) | dst.waddr


class TMULookUpConfig:
    _per: int
    _op: int
    _type: int

    @staticmethod
    def default() -> int:
        return TMULookUpConfig.to_int()

    @staticmethod
    def sequential_read_write_vec(n: int) -> int:
        vecs = [
            TMULookUpConfig(),
            TMULookUpConfig().vec2,
            TMULookUpConfig().vec3,
            TMULookUpConfig().vec4,
        ]
        return TMULookUpConfig().to_int(vecs[:n])

    @staticmethod
    def to_int(configs: list["TMULookUpConfig"] = []) -> int:
        if len(configs) > 4:
            raise AssembleError("Too many TMU look-up configurations")
        c = 0xFFFFFFFF
        for config in configs:
            c = ((c << 8) | int(config)) & 0xFFFFFFFF
        return c

    def __init__(self: Self, per: int = 1, op: int = 15, type: int = 7) -> None:
        if per < 0 or 1 < per:
            raise AssembleError("Invalid per")
        if op < 0 or 15 < op:
            raise AssembleError("Invalid op")
        if type < 0 or 7 < type:
            raise AssembleError("Invalid type")
        self._per = per
        self._op = op
        self._type = type

    def __int__(self: Self) -> int:
        return (self._per << 7) | (self._op << 3) | (self._type)

    @property
    def quad(self: Self) -> Self:
        self._per = 0
        return self

    @property
    def pixel(self: Self) -> Self:
        self._per = 1
        return self

    @property
    def write_add_read_prefetch(self: Self) -> Self:
        self._op = 0
        return self

    @property
    def write_sub_read_clear(self: Self) -> Self:
        self._op = 1
        return self

    @property
    def write_xchg_read_flush(self: Self) -> Self:
        self._op = 2
        return self

    @property
    def write_cmpxchg_read_flush(self: Self) -> Self:
        self._op = 3
        return self

    @property
    def write_umin_full_l1_clear(self: Self) -> Self:
        self._op = 4
        return self

    @property
    def write_umax(self: Self) -> Self:
        self._op = 5
        return self

    @property
    def write_smin(self: Self) -> Self:
        self._op = 6
        return self

    @property
    def write_smax(self: Self) -> Self:
        self._op = 7
        return self

    @property
    def write_and_read_inc(self: Self) -> Self:
        self._op = 8
        return self

    @property
    def write_or_read_dec(self: Self) -> Self:
        self._op = 9
        return self

    @property
    def write_xor_read_not(self: Self) -> Self:
        self._op = 10
        return self

    @property
    def regular(self: Self) -> Self:
        self._op = 15
        return self

    @property
    def int8(self: Self) -> Self:
        self._type = 0
        return self

    @property
    def int16(self: Self) -> Self:
        self._type = 1
        return self

    @property
    def vec2(self: Self) -> Self:
        self._type = 2
        return self

    @property
    def vec3(self: Self) -> Self:
        self._type = 3
        return self

    @property
    def vec4(self: Self) -> Self:
        self._type = 4
        return self

    @property
    def uint8(self: Self) -> Self:
        self._type = 5
        return self

    @property
    def uint16(self: Self) -> Self:
        self._type = 6
        return self

    @property
    def uint32(self: Self) -> Self:
        self._type = 7
        return self


class Instruction:
    SIGNALS: dict[str, Signal | LoadSignal] = {
        "thrsw": Signal("thrsw"),
        "ldunif": Signal("ldunif"),
        "ldunifa": Signal("ldunifa"),
        "ldunifrf": LoadSignal("ldunifrf"),
        "ldunifarf": LoadSignal("ldunifarf"),
        "ldtmu": LoadSignal("ldtmu"),
        "ldvary": LoadSignal("ldvary"),
        "ldtlb": LoadSignal("ldtlb"),
        "ldtlbu": LoadSignal("ldtlbu"),
        "ucb": Signal("ucb"),
        "wrtmuc": Signal("wrtmuc"),
        "smimm_a": Signal("smimm_a"),
        "smimm_b": Signal("smimm_b"),
        "smimm_c": Signal("smimm_c"),
        "smimm_d": Signal("smimm_d"),
    }

    REGISTERS: dict[str, Register] = {
        name: Register(name, 1, addr)
        for addr, name in enumerate(
            [
                "",
                "",
                "",
                "",
                "",
                "quad",
                "null",
                "tlb",
                "tlbu",
                "unifa",
                "tmul",
                "tmud",
                "tmua",
                "tmuau",
                "vpm",
                "vpmu",
                "sync",
                "syncu",
                "syncb",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "tmuc",
                "tmus",
                "tmut",
                "tmur",
                "tmui",
                "tmub",
                "tmudref",
                "tmuoff",
                "tmuscm",
                "tmusf",
                "tmuslod",
                "tmuhs",
                "tmuhscm",
                "tmuhsf",
                "tmuhslod",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "rep",
            ]
        )
    }
    for i in range(64):
        REGISTERS[f"rf{i}"] = Register(f"rf{i}", 0, i)

    _serial: int

    def __init__(self: Self, asm: Assembly) -> None:
        self._serial = len(asm)
        asm.append(self)

    def __int__(self: Self) -> int:
        return self.pack()

    def pack(self: Self) -> int:
        assert False, "Bug: Not implemented"

    @property
    def serial(self: Self) -> int:
        return self._serial


ALUConditionLiteral = Literal[
    # push
    "pushz",
    "pushn",
    "pushc",
    # update
    "andz",
    "andnz",
    "nornz",
    "norz",
    "andn",
    "andnn",
    "nornn",
    "norn",
    "andc",
    "andnc",
    "nornc",
    "norc",
    # instruction
    "ifa",
    "ifb",
    "ifna",
    "ifnb",
]

ALUConditionArg = ALUConditionLiteral | None


class ALUConditions:
    _cond_add: ALUConditionLiteral | None
    _cond_mul: ALUConditionLiteral | None

    def __init__(self: Self, cond_add: ALUConditionLiteral | None, cond_mul: ALUConditionLiteral | None) -> None:
        self._cond_add = cond_add
        self._cond_mul = cond_mul

    def pack(self: Self, sigs: Signals) -> int:
        if sigs.is_load:
            if self._cond_add is not None:
                raise AssembleError(f'Conflict conditional flags "{self._cond_add}" and load signal')
            if self._cond_mul is not None:
                raise AssembleError(f'Conflict conditional flags "{self._cond_mul}" and load signal')
            return sigs.write_address << 46

        conds_push = {
            "pushz": 1,
            "pushn": 2,
            "pushc": 3,
        }
        conds_update = {
            "andz": 4,
            "andnz": 5,
            "nornz": 6,
            "norz": 7,
            "andn": 8,
            "andnn": 9,
            "nornn": 10,
            "norn": 11,
            "andc": 12,
            "andnc": 13,
            "nornc": 14,
            "norc": 15,
        }
        conds_insn = {
            "ifa": 0,
            "ifb": 1,
            "ifna": 2,
            "ifnb": 3,
        }

        add_insn = 1 * int(self._cond_add in conds_insn.keys())
        add_push = 2 * int(self._cond_add in conds_push.keys())
        add_update = 3 * int(self._cond_add in conds_update.keys())

        mul_insn = 1 * int(self._cond_mul in conds_insn.keys())
        mul_push = 2 * int(self._cond_mul in conds_push.keys())
        mul_update = 3 * int(self._cond_mul in conds_update.keys())

        add_cond = add_insn + add_push + add_update
        mul_cond = mul_insn + mul_push + mul_update

        none_____: int | None = None
        result: list[list[int | None]] = [
            #    none | add_insn | add_push | add_update
            [0b0000000, 0b0100000, 0b0000000, 0b0000000],  # none
            [0b0110000, 0b1000000, 0b0110000, 0b1000000],  # mul_insn
            [0b0010000, 0b0100000, none_____, none_____],  # mul_push
            [0b0010000, none_____, none_____, none_____],  # mul_update
        ]
        result = result[mul_cond][add_cond]

        if result is None:
            raise AssembleError(f'Conflict conditional flags "{self._cond_add}" and "{self._cond_mul}"')

        if add_push > 0 and self._cond_add is not None:
            result |= conds_push[self._cond_add]
        if mul_push > 0 and self._cond_mul is not None:
            result |= conds_push[self._cond_mul]
        if add_update > 0 and self._cond_add is not None:
            result |= conds_update[self._cond_add]
        if mul_update > 0 and self._cond_mul is not None:
            result |= conds_update[self._cond_mul]
        if mul_insn > 0 and self._cond_mul is not None:
            if add_insn > 0 and self._cond_add is not None:
                result |= conds_insn[self._cond_mul] << 4
                result |= conds_insn[self._cond_add]
            elif add_update > 0:
                result |= conds_insn[self._cond_mul] << 4
            else:
                result |= conds_insn[self._cond_mul] << 2
        elif add_insn > 0 and self._cond_add is not None:
            result |= conds_insn[self._cond_add] << 2

        return result << 46


class ALURaddr:
    _addr: Final[int | float | Register]

    def __init__(self: Self, addr: int | float | Register) -> None:
        self._addr = addr

    def has_smimm(self: Self) -> bool:
        return isinstance(self._addr, int | float)

    @property
    def modifier(self: Self) -> InputUnpackModifier:
        match self._addr:
            case Register() as reg:
                return reg.unpack_modifier
            case _:
                return Register.INPUT_MODIFIER["none"]

    def pack(self: Self) -> int:
        raddr: int = 0

        def pack_smimms_int(x: int) -> int:
            smimms_int = {}
            for i in range(16):
                smimms_int[i] = i
                smimms_int[i - 16] = i + 16
                smimms_int[pack_unpack("i", "I", i - 16)] = i + 16
                smimms_int[pack_unpack("f", "I", 2 ** (i - 8))] = i + 32
            return smimms_int[x]

        def pack_smimms_float(x: float) -> int:
            smimms_float = {}
            for i in range(16):
                # Denormal numbers
                smimms_float[pack_unpack("I", "f", i)] = i
                smimms_float[2 ** (i - 8)] = i + 32
            return smimms_float[x]

        match self._addr:
            case int():
                raddr = pack_smimms_int(self._addr)
            case float():
                raddr = pack_smimms_float(self._addr)
            case Register() as reg:
                raddr = reg.waddr
            case _:
                raise RuntimeError("unreachable")

        return raddr


class Operation:
    _name: Final[str]
    _opcode: Final[int]
    _has_dst: Final[bool]
    _has_a: Final[bool]
    _has_b: Final[bool]
    _raddr_mask: Final[int | None]

    def __init__(
        self: Self,
        name: str,
        opcode: int,
        has_dst: bool,
        has_a: bool,
        has_b: bool,
        raddr_mask: int | tuple[int, int] | None = None,
    ) -> None:
        self._name = name
        self._opcode = opcode
        self._has_dst = has_dst
        self._has_a = has_a
        self._has_b = has_b
        if has_b and raddr_mask is not None:
            raise AssembleError("raddr_mask is not supported for operation that has src2 operand")
        mask: int | None
        match raddr_mask:
            case int() as n:
                mask = 1 << n
            case (int() as bottom, int() as top):
                mask = sum(1 << i for i in range(bottom, top + 1))
            case None:
                mask = None
            case _:
                raise RuntimeError("unreachable")
        self._raddr_mask = mask

    @property
    def name(self: Self) -> str:
        return self._name

    @property
    def opcode(self: Self) -> int:
        return self._opcode

    @property
    def has_dst(self: Self) -> bool:
        return self._has_dst

    @property
    def has_a(self: Self) -> bool:
        return self._has_a

    @property
    def has_b(self: Self) -> bool:
        return self._has_b

    @property
    def raddr_mask(self: Self) -> int | None:
        return self._raddr_mask


_add_ops: Final[dict[str, Operation]] = {
    "fadd": Operation("fadd", 0, True, True, True),
    "faddnf": Operation("faddnf", 0, True, True, True),
    "vfpack": Operation("vfpack", 53, True, True, True),
    "add": Operation("add", 56, True, True, True),
    "sub": Operation("sub", 60, True, True, True),
    "fsub": Operation("fsub", 64, True, True, True),
    "imin": Operation("min", 120, True, True, True),
    "imax": Operation("max", 121, True, True, True),
    "umin": Operation("umin", 122, True, True, True),
    "umax": Operation("umax", 123, True, True, True),
    "shl": Operation("shl", 124, True, True, True),
    "shr": Operation("shr", 125, True, True, True),
    "asr": Operation("asr", 126, True, True, True),
    "ror": Operation("ror", 127, True, True, True),
    "fmin": Operation("fmin", 128, True, True, True),
    "fmax": Operation("fmax", 128, True, True, True),
    "vfmin": Operation("vfmin", 176, True, True, True),
    "band": Operation("and", 181, True, True, True),
    "bor": Operation("or", 182, True, True, True),
    "bxor": Operation("xor", 183, True, True, True),
    "vadd": Operation("vadd", 184, True, True, True),
    "vsub": Operation("vsub", 185, True, True, True),
    "bnot": Operation("not", 186, True, True, False, raddr_mask=0),
    "neg": Operation("neg", 186, True, True, False, raddr_mask=1),
    "flapush": Operation("flapush", 186, True, True, False, raddr_mask=2),
    "flbpush": Operation("flbpush", 186, True, True, False, raddr_mask=3),
    "flpop": Operation("flpop", 186, True, True, False, raddr_mask=4),
    "clz": Operation("clz", 186, True, True, False, raddr_mask=5),
    "setmsf": Operation("setmsf", 186, True, True, False, raddr_mask=6),
    "setrevf": Operation("setrevf", 186, True, True, False, raddr_mask=7),
    "nop": Operation("nop", 187, False, False, False, raddr_mask=0),
    "tidx": Operation("tidx", 187, True, False, False, raddr_mask=1),
    "eidx": Operation("eidx", 187, True, False, False, raddr_mask=2),
    "lr": Operation("lr", 187, True, False, False, raddr_mask=3),
    "vfla": Operation("vfla", 187, True, False, False, raddr_mask=4),
    "vflna": Operation("vflna", 187, True, False, False, raddr_mask=5),
    "vflb": Operation("vflb", 187, True, False, False, raddr_mask=6),
    "vflnb": Operation("vflnb", 187, True, False, False, raddr_mask=7),
    "xcd": Operation("xcd", 187, True, False, False, raddr_mask=8),
    "ycd": Operation("ycd", 187, True, False, False, raddr_mask=9),
    "msf": Operation("msf", 187, True, False, False, raddr_mask=10),
    "revf": Operation("revf", 187, True, False, False, raddr_mask=11),
    "iid": Operation("iid", 187, True, False, False, raddr_mask=12),
    "sampid": Operation("sampid", 187, True, False, False, raddr_mask=13),
    "barrierid": Operation("barrierid", 187, True, False, False, raddr_mask=14),
    "tmuwt": Operation("tmuwt", 187, True, False, False, raddr_mask=15),
    "vpmwt": Operation("vpmwt", 187, True, False, False, raddr_mask=16),
    "flafirst": Operation("flafirst", 187, True, False, False, raddr_mask=17),
    "flnafirst": Operation("flnafirst", 187, True, False, False, raddr_mask=18),
    "fxcd": Operation("fxcd", 187, True, False, False, raddr_mask=(32, 34)),
    "fycd": Operation("fycd", 187, True, False, False, raddr_mask=(36, 38)),
    "ldvpmv_in": Operation("ldvpmv_in", 188, True, True, False, raddr_mask=0),
    "ldvpmd_in": Operation("ldvpmd_in", 188, True, True, False, raddr_mask=1),
    "ldvpmp": Operation("ldvpmp", 188, True, True, False, raddr_mask=2),
    "recip": Operation("recip", 188, True, True, False, raddr_mask=32),
    "rsqrt": Operation("rsqrt", 188, True, True, False, raddr_mask=33),
    "exp": Operation("exp", 188, True, True, False, raddr_mask=34),
    "log": Operation("log", 188, True, True, False, raddr_mask=35),
    "sin": Operation("sin", 188, True, True, False, raddr_mask=36),
    "rsqrt2": Operation("rsqrt2", 188, True, True, False, raddr_mask=37),
    "ballot": Operation("ballot", 188, True, True, False, raddr_mask=38),
    "bcastf": Operation("bcastf", 188, True, True, False, raddr_mask=39),
    "alleq": Operation("alleq", 188, True, True, False, raddr_mask=40),
    "allfeq": Operation("allfeq", 188, True, True, False, raddr_mask=41),
    "ldvpmg_in": Operation("ldvpmg_in", 189, True, True, True),
    "stvpmv": Operation("stvpmv", 190, True, False, True),
    "stvpmd": Operation("stvpmd", 190, True, False, True),
    "stvpmp": Operation("stvpmp", 190, True, False, True),
    "fcmp": Operation("fcmp", 192, True, True, True),
    "vfmax": Operation("vfmax", 240, True, True, True),
    "fround": Operation("fround", 245, True, True, False, raddr_mask=(0, 2)),
    "ftoin": Operation("ftoin", 245, True, True, False, raddr_mask=3),
    "ftrunc": Operation("ftrunc", 245, True, True, False, raddr_mask=(16, 18)),
    "ftoiz": Operation("ftoiz", 245, True, True, False, raddr_mask=19),
    "ffloor": Operation("ffloor", 245, True, True, False, raddr_mask=(32, 34)),
    "ftouz": Operation("ftouz", 245, True, True, False, raddr_mask=35),
    "fceil": Operation("fceil", 245, True, True, False, raddr_mask=(48, 50)),
    "ftoc": Operation("ftoc", 245, True, True, False, raddr_mask=51),
    "fdx": Operation("fdx", 246, True, True, False, raddr_mask=(0, 2)),
    "fdy": Operation("fdy", 246, True, True, False, raddr_mask=(16, 18)),
    "itof": Operation("itof", 246, True, True, False, raddr_mask=(32, 34)),
    "utof": Operation("utof", 246, True, True, False, raddr_mask=(36, 38)),
    "vpack": Operation("vpack", 247, True, True, True),
    "v8pack": Operation("v8pack", 248, True, True, True),
    "fmov": Operation("fmov", 249, True, True, False, raddr_mask=(0, 2)),
    "mov": Operation("mov", 249, True, True, False, raddr_mask=3),
    "v10pack": Operation("v10pack", 250, True, True, True),
    "v11fpack": Operation("v11fpack", 251, True, True, True),
    "quad_rotate": Operation("quad_rotate", 252, True, True, True),
    "rotate": Operation("rotate", 253, True, True, True),
    "shuffle": Operation("shuffle", 254, True, True, True),
}

_mul_ops: Final[dict[str, Operation]] = {
    "add": Operation("add", 1, True, True, True),
    "sub": Operation("sub", 2, True, True, True),
    "umul24": Operation("umul24", 3, True, True, True),
    "vfmul": Operation("vfmul", 4, True, True, True),
    "smul24": Operation("smul24", 9, True, True, True),
    "multop": Operation("multop", 10, True, True, True),
    "fmov": Operation("fmov", 14, True, True, False, raddr_mask=(0, 2)),
    "mov": Operation("mov", 14, True, True, False, raddr_mask=3),
    "ftounorm16": Operation("ftounorm16", 14, True, True, False, raddr_mask=32),
    "ftosnorm16": Operation("ftosnorm16", 14, True, True, False, raddr_mask=33),
    "vftounorm8": Operation("vftounorm8", 14, True, True, False, raddr_mask=34),
    "vftosnorm8": Operation("vftosnorm8", 14, True, True, False, raddr_mask=35),
    "vftounorm10lo": Operation("vftounorm10lo", 14, True, True, False, raddr_mask=48),
    "vftounorm10hi": Operation("vftounorm10hi", 14, True, True, False, raddr_mask=49),
    "nop": Operation("nop", 14, False, False, False, raddr_mask=63),
    "fmul": Operation("fmul", 16, True, True, True),
}


class ALUInstructionPacking(ctypes.Structure):
    _fields_ = [
        ("raddr_b", ctypes.c_int, 6),
        ("raddr_a", ctypes.c_int, 6),
        ("raddr_d", ctypes.c_int, 6),
        ("raddr_c", ctypes.c_int, 6),
        ("op_add", ctypes.c_int, 8),
        ("waddr_add", ctypes.c_int, 6),
        ("waddr_mul", ctypes.c_int, 6),
        ("ma", ctypes.c_int, 1),
        ("mm", ctypes.c_int, 1),
        ("cond", ctypes.c_int, 7),
        ("signal", ctypes.c_int, 5),
        ("op_mul", ctypes.c_int, 6),
    ]

    @property
    def op_mul(self: Self) -> int:
        return self.op_mul

    @op_mul.setter
    def op_mul(self: Self, value: int | Operation) -> None:
        opcode: int
        match value:
            case int():
                opcode = value
            case Operation():
                opcode = value.opcode
            case _:
                raise RuntimeError("unreachable")
        if all(opcode != op.opcode for op in _mul_ops.values()):
            raise AssembleError(f'Invalid mul opcode "{opcode}"')
        if opcode < 0 or 2**6 <= opcode:
            raise AssembleError(f'Invalid mul opcode "{opcode}"')
        self.op_mul = opcode

    @property
    def signal(self: Self) -> int:
        return self.signal

    @signal.setter
    def signal(self: Self, value: int | Signals) -> None:
        sig: int
        match value:
            case int():
                sig = value
            case Signal():
                sig = value.pack()
            case _:
                raise RuntimeError("unreachable")
        if sig < 0 or 2**5 <= sig:
            raise AssembleError(f'Invalid signal "{value}"')
        self.signal = sig

    @property
    def cond(self: Self) -> int:
        return self.cond

    @cond.setter
    def cond(self: Self, value: int) -> None:
        cond: int
        match value:
            case int():
                cond = value
            case _:
                raise RuntimeError("unreachable")
        if cond < 0 or 2**7 <= cond:
            raise AssembleError(f'Invalid condition "{value}"')
        self.cond = cond

    @property
    def mm(self: Self) -> int:
        return self.mm

    @mm.setter
    def mm(self: Self, value: int) -> None:
        if value < 0 or 2**1 <= value:
            raise AssembleError(f'Invalid mm "{value}"')
        self.mm = value

    @property
    def ma(self: Self) -> int:
        return self.ma

    @ma.setter
    def ma(self: Self, value: int) -> None:
        if value < 0 or 2**1 <= value:
            raise AssembleError(f'Invalid ma "{value}"')
        self.ma = value

    @property
    def waddr_mul(self: Self) -> int:
        return self.waddr_mul

    @waddr_mul.setter
    def waddr_mul(self: Self, value: int) -> None:
        if value < 0 or 2**6 <= value:
            raise AssembleError(f'Invalid waddr_mul "{value}"')
        self.waddr_mul = value

    @property
    def waddr_add(self: Self) -> int:
        return self.waddr_add

    @waddr_add.setter
    def waddr_add(self: Self, value: int) -> None:
        if value < 0 or 2**6 <= value:
            raise AssembleError(f'Invalid waddr_add "{value}"')
        self.waddr_add = value

    @property
    def op_add(self: Self) -> int:
        return self.op_add

    @op_add.setter
    def op_add(self: Self, value: int | Operation) -> None:
        opcode: int
        match value:
            case int():
                opcode = value
            case Operation():
                opcode = value.opcode
            case _:
                raise RuntimeError("unreachable")
        if all(opcode != op.opcode for op in _add_ops.values()):
            raise AssembleError(f'Invalid add opcode "{opcode}"')
        if opcode < 0 or 2**8 <= opcode:
            raise AssembleError(f'Invalid add opcode "{opcode}"')
        self.op_add = opcode

    @property
    def raddr_c(self: Self) -> int:
        return self.raddr_c

    @raddr_c.setter
    def raddr_c(self: Self, value: int) -> None:
        if value < 0 or 2**6 <= value:
            raise AssembleError(f'Invalid raddr_c "{value}"')
        self.raddr_c = value

    @property
    def raddr_d(self: Self) -> int:
        return self.raddr_d

    @raddr_d.setter
    def raddr_d(self: Self, value: int) -> None:
        if value < 0 or 2**6 <= value:
            raise AssembleError(f'Invalid raddr_d "{value}"')
        self.raddr_d = value

    @property
    def raddr_a(self: Self) -> int:
        return self.raddr_a

    @raddr_a.setter
    def raddr_a(self: Self, value: int) -> None:
        if value < 0 or 2**6 <= value:
            raise AssembleError(f'Invalid raddr_a "{value}"')
        self.raddr_a = value

    @property
    def raddr_b(self: Self) -> int:
        return self.raddr_b

    @raddr_b.setter
    def raddr_b(self: Self, value: int) -> None:
        if value < 0 or 2**6 <= value:
            raise AssembleError(f'Invalid raddr_b "{value}"')
        self.raddr_b = value


class ALUInstruction(ctypes.Union):
    _anonymous_ = ("packed",)
    _fields_ = [
        ("packed", ALUInstructionPacking),
        ("raw", ctypes.c_uint64),
    ]

    def __int__(self: Self) -> int:
        return int(self.raw)


class ALUOp(ABC):
    OPERATIONS: dict[str, Operation] = {}

    _name: str
    _op: Operation
    _dst: Register
    _raddr_a: ALURaddr
    _raddr_b: ALURaddr
    _cond: ALUConditionLiteral | None
    _sigs: Signals

    def __init__(
        self: Self,
        opr: str,
        dst: Register = Instruction.REGISTERS["null"],
        src1: int | float | Register | None = None,
        src2: int | float | Register | None = None,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        assert opr in self.OPERATIONS

        self._name = opr
        self._op = self.OPERATIONS[opr]
        self._dst = dst

        match (self.op.has_a, src1):
            case (False, int() | float() | Register()):
                raise AssembleError(f'"{self.name}" rejects src1')
            case (True, None):
                raise AssembleError(f'"{self.name}" requires src1')
            case (True, int() | float() | Register()):
                self._raddr_a = ALURaddr(src1)
            case (False, None):
                self._raddr_a = ALURaddr(Register("_unused", 0, 0))

        match (self.op.has_b, src2):
            case (False, int() | float() | Register()):
                raise AssembleError(f'"{self.name}" rejects src2')
            case (True, None):
                raise AssembleError(f'"{self.name}" requires src2')
            case (True, int() | float() | Register()):
                self._raddr_b = ALURaddr(src2)
            case (False, None):
                self._raddr_b = ALURaddr(Register("_unused", 0, 0))

        self._cond = cond
        self._sigs = Signals()
        match sig:
            case Signal():
                self.sigs.add(sig)
            case list():
                for s in sig:
                    self.sigs.add(s)
            case None:
                pass

    @abstractmethod
    def pack(self: Self) -> int: ...

    @property
    def name(self: Self) -> str:
        return self._name

    @property
    def op(self: Self) -> Operation:
        return self._op

    @property
    def dst(self: Self) -> Register:
        return self._dst

    @property
    def raddr_a(self: Self) -> ALURaddr:
        return self._raddr_a

    @property
    def raddr_b(self: Self) -> ALURaddr:
        return self._raddr_b

    @property
    def cond(self: Self) -> ALUConditionLiteral | None:
        return self._cond

    @property
    def sigs(self: Self) -> Signals:
        return self._sigs

    def swap_signal(self: Self, sig1: Signal, sig2: Signal) -> None:
        sigs = Signals()
        for sig in self.sigs:
            if sig.name == sig1.name:
                sigs.add(sig2)
            elif sig.name == sig2.name:
                sigs.add(sig1)
            else:
                sigs.add(sig)
        self._sigs = sigs


class AddALUOp(ALUOp):
    def __init__(
        self: Self,
        opr: str,
        dst: Register = Instruction.REGISTERS["null"],
        src1: int | float | Register | None = None,
        src2: int | float | Register | None = None,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        super().__init__(opr=opr, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def pack(self: Self) -> int:
        op: int = self.op.opcode
        raddr_a = self.raddr_a.pack()

        smimm_a = cast(Signal, Instruction.SIGNALS["smimm_a"])
        smimm_b = cast(Signal, Instruction.SIGNALS["smimm_b"])
        self.sigs.discard(smimm_a)
        self.sigs.discard(smimm_b)
        if self.raddr_a.has_smimm():
            self.sigs.add(smimm_a)
        if self.raddr_b.has_smimm():
            self.sigs.add(smimm_b)

        match self.op.raddr_mask:
            case int() as raddr_mask:
                if raddr_mask == 0:
                    raddr_b = 0
                else:
                    raddr_b = (raddr_mask & -raddr_mask).bit_length() - 1
            case None:
                raddr_b = self.raddr_b.pack()
            case _:
                raise RuntimeError("unreachable")

        match self.name:
            case "fadd" | "faddnf" | "fsub" | "fmax" | "fmin" | "fcmp":
                if self.name != "fcmp":
                    pack = self.dst.pack_modifier.f32
                    if pack is None:
                        raise AssembleError(f'"{self.name}" requires dst as register with modifier from f32')
                    op |= pack << 4

                a_unpack = self.raddr_a.modifier.f32
                if a_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src1 as register with modifier to f32')
                b_unpack = self.raddr_b.modifier.f32
                if b_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src2 as register with modifier to f32')

                operand_a = int(self.raddr_a.has_smimm()) * 256 + a_unpack * 64 + raddr_a
                operand_b = int(self.raddr_b.has_smimm()) * 256 + b_unpack * 64 + raddr_b
                ordering = operand_a > operand_b
                if (self.name in ["fmin", "fadd"] and ordering) or (self.name in ["fmax", "faddnf"] and not ordering):
                    a_unpack, b_unpack = b_unpack, a_unpack
                    raddr_a, raddr_b = raddr_b, raddr_a
                    if isinstance(self.raddr_a._addr, int | float) or isinstance(self.raddr_b._addr, int | float):
                        assert self.raddr_a._addr != self.raddr_b._addr
                        self.swap_signal(smimm_a, smimm_b)

                op |= a_unpack << 2
                op |= b_unpack << 0
            case "vfpack":
                if self.raddr_a.modifier.name == "abs" or self.raddr_b.modifier.name == "abs":
                    raise AssembleError(f'"{self.name}" rejects "abs" modifier')

                a_unpack = self.raddr_a.modifier.f32
                if a_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src1 as register with modifier to f32')
                b_unpack = self.raddr_b.modifier.f32
                if b_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src2 as register with modifier to f32')

                op = (op & ~(0x3 << 2)) | (a_unpack << 2)
                op = (op & ~(0x3 << 0)) | (b_unpack << 0)
            case "fround" | "ftrunc" | "ffloor" | "fceil" | "fdx" | "fdy":
                pack = self.dst.pack_modifier.f32
                if pack is None:
                    raise AssembleError(f'"{self.name}" requires dst as register with modifier from f32')
                raddr_b |= pack

                if not isinstance(self.raddr_a._addr, Register):
                    raise AssembleError(f'"{self.name}" requires raddr_a as register')
                if self.raddr_a.modifier.name == "abs":
                    raise AssembleError(f'"{self.name}" rejects "abs" modifier')

                a_unpack = self.raddr_a.modifier.f32
                if a_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src as register with modifier to f32')

                raddr_b = (raddr_b & ~(3 << 2)) | a_unpack << 2
            case "ftoin" | "ftoiz" | "ftouz" | "ftoc":
                if self.dst.pack_modifier.name != "none":
                    raise AssembleError(f'"{self.name}" rejects dst with "{self.dst.pack_modifier.name}" modifier')
                if not isinstance(self.raddr_a._addr, Register):
                    raise AssembleError(f'"{self.name}" requires src as register')
                if self.raddr_a.modifier.name == "abs":
                    raise AssembleError(f'"{self.name}" rejects src with "{self.raddr_a.modifier.name}" modifier')

                a_unpack = self.raddr_a.modifier.f32
                if a_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src as register with modifier to f32')

                raddr_b |= (raddr_b & ~(3 << 2)) | a_unpack << 2
            case "vfmin" | "vfmax":
                if self.dst.pack_modifier.name != "none":
                    raise AssembleError(f'"{self.name}" rejects dst with "{self.dst.pack_modifier.name}" modifier')

                a_unpack = self.raddr_a.modifier.f16
                if a_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src as register with modifier to f16')

                op |= a_unpack
            case "mov":
                if self.dst.pack_modifier.name != "none":
                    raise AssembleError(f'"{self.name}" rejects dst with "{self.dst.pack_modifier.name}" modifier')

                a_unpack = self.raddr_a.modifier.i32
                if a_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src as register with modifier to i32')

                raddr_b |= a_unpack << 2
            case "fmov":
                pack = self.dst.pack_modifier.f32
                if pack is None:
                    raise AssembleError(f'"{self.name}" requires dst as register with modifier from f32')

                a_unpack = self.raddr_a.modifier.f32
                if a_unpack is None:
                    raise AssembleError(f'"{self.name}" requires src as register with modifier to f32')

                raddr_b = pack | (a_unpack << 2)
            case "nop":
                pass
            case "rotate" | "quad_rotate" | "shuffle":
                if self.dst.name in ["rep", "quad"]:
                    raise AssembleError(f'"{self.name}" rejects "{self.dst.name}" dst')
            case _:
                if self.dst.pack_modifier.name != "none":
                    raise AssembleError(f'"{self.name}" rejects dst with "{self.dst.pack_modifier.name}" modifier')
                if self.raddr_a.modifier.name != "none":
                    raise AssembleError(f'"{self.name}" rejects src1 with "{self.raddr_a.modifier.name}" modifier')
                if self.raddr_b.modifier.name != "none":
                    raise AssembleError(f'"{self.name}" rejects src2 with "{self.raddr_b.modifier.name}" modifier')

        inst = ALUInstruction()
        inst.ma = self.dst.magic
        inst.waddr_add = self.dst.waddr
        inst.op_add = op
        inst.raddr_a = raddr_a
        inst.raddr_b = raddr_b

        return int(inst)

    # v3d71_add_ops
    OPERATIONS = _add_ops


class MulALUOp(ALUOp):
    def __init__(
        self: Self,
        opr: str,
        dst: Register = Instruction.REGISTERS["null"],
        src1: int | float | Register | None = None,
        src2: int | float | Register | None = None,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        super().__init__(opr=opr, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def pack(self: Self) -> int:
        op = self.op.opcode

        smimm_c = cast(Signal, Instruction.SIGNALS["smimm_c"])
        smimm_d = cast(Signal, Instruction.SIGNALS["smimm_d"])
        self.sigs.discard(smimm_c)
        self.sigs.discard(smimm_d)
        if self.raddr_a.has_smimm():
            self.sigs.add(smimm_c)
        if self.raddr_b.has_smimm():
            self.sigs.add(smimm_d)

        raddr_c = self.raddr_a.pack()
        match self.op.raddr_mask:
            case int() as raddr_mask:
                if raddr_mask == 0:
                    raddr_d = 0
                else:
                    raddr_d = (raddr_mask & -raddr_mask).bit_length() - 1
            case None:
                raddr_d = self.raddr_b.pack()
            case _:
                raise RuntimeError("unreachable")

        if self.name in ["vfmul"]:
            a_unpack = self.raddr_a.modifier.f16
            if a_unpack is None:
                raise AssembleError(f'"{self.name}" requires src1 as register with modifier to f16')

            op += a_unpack

        if self.name in ["fmul"]:
            pack = self.dst.pack_modifier.f32
            if pack is None:
                raise AssembleError(f'"{self.name}" requires dst as register with modifier from f32')

            a_unpack = self.raddr_a.modifier.f32
            if a_unpack is None:
                raise AssembleError(f'"{self.name}" requires src1 as register with modifier to f32')
            b_unpack = self.raddr_b.modifier.f32
            if b_unpack is None:
                raise AssembleError(f'"{self.name}" requires src2 as register with modifier to f32')

            op += pack << 4
            op |= a_unpack << 2
            op |= b_unpack << 0

        if self.name in ["fmov"]:
            pack = self.dst.pack_modifier.f32
            if pack is None:
                raise AssembleError(f'"{self.name}" requires dst as register with modifier from f32')

            a_unpack = self.raddr_a.modifier.f32
            if a_unpack is None:
                raise AssembleError(f'"{self.name}" requires src as register with modifier to f32')

            raddr_d |= pack
            raddr_d |= a_unpack << 2

        inst = ALUInstruction()
        inst.mm = self.dst.magic
        inst.waddr_mul = self.dst.waddr
        inst.op_mul = op
        inst.raddr_c = raddr_c
        inst.raddr_d = raddr_d

        return int(inst)

    # v3d71_mul_ops
    OPERATIONS = _mul_ops


class ALU(Instruction):
    _add_op: AddALUOp
    _mul_op: MulALUOp | None
    _pack_result: int | None

    def __init__(
        self: Self,
        asm: Assembly,
        opr: str,
        dst: Register = Instruction.REGISTERS["null"],
        src1: int | float | Register | None = None,
        src2: int | float | Register | None = None,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        super().__init__(asm)

        if opr in AddALUOp.OPERATIONS:
            self._add_op = AddALUOp(opr=opr, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)
            self._mul_op = None
        elif opr in MulALUOp.OPERATIONS:
            self._add_op = AddALUOp("nop")
            self._mul_op = MulALUOp(opr=opr, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)
        else:
            raise AssembleError(f'"{opr}" is unknown operation')
        self._repack()

    def dual_issue(
        self: Self,
        opr: str,
        dst: Register = Instruction.REGISTERS["null"],
        src1: int | float | Register | None = None,
        src2: int | float | Register | None = None,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        if self._mul_op is not None:
            raise AssembleError(f'Conflict Mul ALU operation. "{self._mul_op.name}" is already issued.')
        self._mul_op = MulALUOp(opr=opr, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)
        self._repack()

    def _repack(self: Self) -> None:
        self._pack_result = None
        self._pack_result = self.pack()

    def pack(self: Self) -> int:
        if self._pack_result is not None:
            return self._pack_result

        add_op = self._add_op
        mul_op = self._mul_op
        if mul_op is None:
            mul_op = MulALUOp("nop")

        if add_op.op == AddALUOp.OPERATIONS["rotate"]:
            if add_op.dst in [
                Instruction.REGISTERS["rep"],
                Instruction.REGISTERS["quad"],
            ]:
                raise AssembleError(f'"{add_op.name}({add_op.dst.name}, ...)" has no effect')
            if add_op.dst in [
                Instruction.REGISTERS["tmud"],
                Instruction.REGISTERS["tmua"],
                Instruction.REGISTERS["tmuc"],
            ]:
                raise AssembleError(f'"{add_op.name}({add_op.dst.name}, ...)" is dangerous')
            if mul_op.dst in [
                Instruction.REGISTERS["tmud"],
                Instruction.REGISTERS["tmua"],
                Instruction.REGISTERS["tmuc"],
            ]:
                raise AssembleError(f'"{add_op.name}(...).op({mul_op.dst.name}, ...)" is dangerous')

        add_op_packed = add_op.pack()
        mul_op_packed = mul_op.pack()

        # ATTENTION: {add,mul}_op.pack() changes {add,mul}_op.sigs.
        # So it must be called before {add,mul}_op.sigs are referenced.

        sigs = Signals()
        sigs.add(add_op.sigs)
        sigs.add(mul_op.sigs)
        sigs_packed = sigs.pack()

        cond = ALUConditions(add_op.cond, mul_op.cond)
        cond_packed = cond.pack(sigs)

        return 0 | sigs_packed | cond_packed | add_op_packed | mul_op_packed


class ALUWithoutSMIMM(ALU):
    """ALU Instruction doesn't includes small immediate values yet."""

    def __init__(
        self: Self,
        asm: Assembly,
        opr: str,
        dst: Register = Instruction.REGISTERS["null"],
        src1: int | float | Register | None = None,
        src2: int | float | Register | None = None,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        super().__init__(asm, opr=opr, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    @overload
    def add(
        self: Self,
        dst: Register,
        src1: int,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def add(
        self: Self,
        dst: Register,
        src1: Register,
        src2: int,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def add(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def add(
        self: Self,
        dst: Register,
        src1: int | Register,
        src2: int | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("add", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    @overload
    def sub(
        self: Self,
        dst: Register,
        src1: int,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def sub(
        self: Self,
        dst: Register,
        src1: Register,
        src2: int,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def sub(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def sub(
        self: Self,
        dst: Register,
        src1: int | Register,
        src2: int | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("sub", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    @overload
    def umul24(
        self: Self,
        dst: Register,
        src1: int,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def umul24(
        self: Self,
        dst: Register,
        src1: Register,
        src2: int,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def umul24(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def umul24(
        self: Self,
        dst: Register,
        src1: int | Register,
        src2: int | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("umul24", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    @overload
    def vfmul(
        self: Self,
        dst: Register,
        src1: float,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def vfmul(
        self: Self,
        dst: Register,
        src1: Register,
        src2: float,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def vfmul(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def vfmul(
        self: Self,
        dst: Register,
        src1: float | Register,
        src2: float | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("vfmul", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    @overload
    def smul24(
        self: Self,
        dst: Register,
        src1: int,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def smul24(
        self: Self,
        dst: Register,
        src1: Register,
        src2: int,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def smul24(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def smul24(
        self: Self,
        dst: Register,
        src1: int | Register,
        src2: int | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("smul24", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    @overload
    def multop(
        self: Self,
        dst: Register,
        src1: int | float,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def multop(
        self: Self,
        dst: Register,
        src1: Register,
        src2: int | float,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def multop(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def multop(
        self: Self,
        dst: Register,
        src1: int | float | Register,
        src2: int | float | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("multop", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    @overload
    def fmov(
        self: Self,
        dst: Register,
        src1: float,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def fmov(
        self: Self,
        dst: Register,
        src1: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def fmov(
        self: Self,
        dst: Register,
        src1: float | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("fmov", dst=dst, src1=src1, src2=None, cond=cond, sig=sig)

    def nop(
        self: Self,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("nop", dst=Instruction.REGISTERS["null"], src1=None, src2=None, cond=cond, sig=sig)

    @overload
    def mov(
        self: Self,
        dst: Register,
        src1: int | float,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def mov(
        self: Self,
        dst: Register,
        src1: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def mov(
        self: Self,
        dst: Register,
        src1: int | float | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("mov", dst=dst, src1=src1, src2=None, cond=cond, sig=sig)

    @overload
    def fmul(
        self: Self,
        dst: Register,
        src1: float,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def fmul(
        self: Self,
        dst: Register,
        src1: Register,
        src2: float,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    @overload
    def fmul(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None: ...
    def fmul(
        self: Self,
        dst: Register,
        src1: int | float | Register,
        src2: int | float | Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("fmul", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)


class ALUWithSMIMM(ALU):
    """ALU Instruction already includes small immediate values."""

    def __init__(
        self: Self,
        asm: Assembly,
        opr: str,
        dst: Register = Instruction.REGISTERS["null"],
        src1: int | float | Register | None = None,
        src2: int | float | Register | None = None,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        super().__init__(asm, opr=opr, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def add(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("add", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def sub(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("sub", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def umul24(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("umul24", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def vfmul(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("vfmul", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def smul24(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("smul24", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def multop(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("multop", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)

    def fmov(
        self: Self,
        dst: Register,
        src1: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("fmov", dst=dst, src1=src1, src2=None, cond=cond, sig=sig)

    def nop(
        self: Self,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("nop", dst=Instruction.REGISTERS["null"], src1=None, src2=None, cond=cond, sig=sig)

    def mov(
        self: Self,
        dst: Register,
        src1: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("mov", dst=dst, src1=src1, src2=None, cond=cond, sig=sig)

    def fmul(
        self: Self,
        dst: Register,
        src1: Register,
        src2: Register,
        cond: ALUConditionArg = None,
        sig: SignalArg = None,
    ) -> None:
        return self.dual_issue("fmul", dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)


class Link:
    def __init__(self: Self) -> None:
        pass


BranchConditionLiteral = Literal["always", "a0", "na0", "alla", "anyna", "anya", "allna"]


class BranchCondition:
    _name: BranchConditionLiteral
    _code: int

    def __init__(self: Self, name: BranchConditionLiteral, code: int) -> None:
        self._name = name
        self._code = code

    @property
    def name(self: Self) -> str:
        return self._name

    @property
    def code(self: Self) -> int:
        return self._code


_conditions: dict[BranchConditionLiteral, BranchCondition] = {
    "always": BranchCondition("always", 0),
    "a0": BranchCondition("a0", 2),
    "na0": BranchCondition("na0", 3),
    "alla": BranchCondition("alla", 4),
    "anyna": BranchCondition("anyna", 5),
    "anya": BranchCondition("anya", 6),
    "allna": BranchCondition("allna", 7),
}


class Branch(Instruction):
    _cond: BranchCondition
    _raddr_a: int | None
    _addr_label: Reference | None
    _addr: int | None
    _set_link: bool
    _ub: int
    _bdu: int
    _bdi: int

    def __init__(
        self: Self,
        asm: Assembly,
        src: int | Register | Reference | Link | None,
        *,
        cond: BranchConditionLiteral,
        absolute: bool = False,
        set_link: bool = False,
    ) -> None:
        super().__init__(asm)

        if cond not in _conditions:
            raise AssembleError(f'"{cond}" is unknown condition')

        self._cond = _conditions[cond]
        self._raddr_a = None
        self._addr_label = None
        self._addr = None
        self._set_link = set_link

        self._ub = 0
        self._bdu = 1

        match src:
            case Link():  # Branch to link_reg
                self._bdi = 2
            case Register() as reg:  # Branch to rf
                if reg.magic == 0:
                    self._bdi = 3
                    self._raddr_a = reg.waddr
            case Reference() as ref:  # Branch to label
                self._bdi = 1
                self._addr_label = ref
            case int() as imm:  # Branch to imm
                self._bdi = 0 if absolute else 1
                self._addr = imm
            case _:
                raise AssembleError("Invalid src object")

    def unif_addr(self: Self, src: Register | None = None, absolute: bool = False) -> None:
        self._ub = 1
        self._bdu = 1
        if src is None:
            self._bdu = 0 if absolute else 1
        elif isinstance(src, Register) and src.magic == 0:
            # Branch to reg
            self._bdu = 3
            if self._raddr_a is None or self._raddr_a == src.waddr:
                self._raddr_a = src.waddr
            else:
                raise AssembleError("Conflict registers")
        else:
            raise AssembleError("Invalid src object")

    def pack(self: Self) -> int:
        if self._addr_label is not None:
            addr = cast(int, pack_unpack("i", "I", (int(self._addr_label) - self.serial - 4) * 8))
        elif self._addr is not None:
            addr = self._addr
        else:
            addr = 0

        set_link = 1 if self._set_link else 0

        msfign = 0b00

        return (
            0
            | (0b10 << 56)
            | (((addr & ((1 << 24) - 1)) >> 3) << 35)
            | (self._cond.code << 32)
            | ((addr >> 24) << 24)
            | (set_link << 23)
            | (msfign << 21)
            | (self._bdu << 15)
            | (self._ub << 14)
            | (self._bdi << 12)
            | ((self._raddr_a if self._raddr_a is not None else 0) << 6)
        )


class Raw(Instruction):
    _packed_code: int

    def __init__(self: Self, asm: Assembly, packed_code: int) -> None:
        super().__init__(asm)
        self._packed_code = packed_code

    def pack(self) -> int:
        return self._packed_code


_alias_regs: dict[str, Register] = {
    "broadcast": Instruction.REGISTERS["rep"],
    "quad_broadcast": Instruction.REGISTERS["quad"],
}


class Loop:
    asm: Assembly
    name: str

    def __init__(self: Self, asm: Assembly, name: str) -> None:
        self.asm = asm
        self.name = name

    def b(
        self: Self,
        *,
        cond: BranchConditionLiteral,
        absolute: bool = False,
        set_link: bool = False,
    ) -> Branch:
        return Branch(self.asm, Reference(self.asm, self.name), cond=cond, absolute=absolute, set_link=set_link)


class LoopHelper:
    asm: Assembly

    def __init__(self: Self, asm: Assembly):
        self.asm = asm

    def __enter__(self: Self) -> Loop:
        name = self.asm.gen_unused_label("__generated_loop_label_{}")
        Label(self.asm).__getattr__(name)
        return Loop(self.asm, name)

    def __exit__(
        self: Self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: type[TracebackType] | None,
    ) -> None:
        pass


def binary_add_inst(
    name: str,
    asm: Assembly,
    dst: Register,
    src1: int | float | Register,
    src2: int | float | Register,
    cond: ALUConditionArg = None,
    sig: SignalArg = None,
) -> ALUWithSMIMM | ALUWithoutSMIMM:
    match (src1, src2):
        case (Register(), Register()):
            return ALUWithoutSMIMM(asm, name, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)
        case _:
            return ALUWithSMIMM(asm, name, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)


def unary_add_inst(
    name: str,
    asm: Assembly,
    dst: Register,
    src: int | float | Register,
    cond: ALUConditionArg = None,
    sig: SignalArg = None,
) -> ALUWithSMIMM | ALUWithoutSMIMM:
    match src:
        case Register():
            return ALUWithoutSMIMM(asm, name, dst=dst, src1=src, cond=cond, sig=sig)
        case _:
            return ALUWithSMIMM(asm, name, dst=dst, src1=src, cond=cond, sig=sig)


def nullary_add_inst(
    name: str,
    asm: Assembly,
    dst: Register = Instruction.REGISTERS["null"],
    cond: ALUConditionArg = None,
    sig: SignalArg = None,
) -> ALUWithSMIMM | ALUWithoutSMIMM:
    return ALUWithoutSMIMM(asm, name, dst=dst, cond=cond, sig=sig)


def binary_mul_inst(
    name: str,
    asm: Assembly,
    dst: Register,
    src1: int | float | Register,
    src2: int | float | Register,
    cond: ALUConditionArg = None,
    sig: SignalArg = None,
) -> None:
    match (src1, src2):
        case (Register(), Register()):
            ALUWithoutSMIMM(asm, name, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)
        case _:
            ALUWithSMIMM(asm, name, dst=dst, src1=src1, src2=src2, cond=cond, sig=sig)


def unary_mul_inst(
    name: str,
    asm: Assembly,
    dst: Register,
    src: int | float | Register,
    cond: ALUConditionArg = None,
    sig: SignalArg = None,
) -> None:
    match src:
        case Register():
            ALUWithoutSMIMM(asm, name, dst=dst, src1=src, cond=cond, sig=sig)
        case _:
            ALUWithSMIMM(asm, name, dst=dst, src1=src, cond=cond, sig=sig)


def qpu(func):
    @functools.wraps(func)
    def decorator(asm, *args, **kwargs):
        g = func.__globals__
        g_orig = g.copy()
        g["L"] = Label(asm)
        g["R"] = ReferenceHelper(asm)
        g["loop"] = LoopHelper(asm)
        g["b"] = functools.partial(Branch, asm)
        g["link"] = Link()
        g["raw"] = functools.partial(Raw, asm)
        g["namespace"] = functools.partial(LabelNameSpace, asm)
        for op_name, op in MulALUOp.OPERATIONS.items():
            if op.has_dst and op.has_a and op.has_b:
                g[op_name] = functools.partial(binary_mul_inst, op_name, asm)
            elif op.has_dst and op.has_a and not op.has_b:
                g[op_name] = functools.partial(unary_mul_inst, op_name, asm)
        for op_name, op in AddALUOp.OPERATIONS.items():
            if op.has_dst and op.has_a and op.has_b:
                g[op_name] = functools.partial(binary_add_inst, op_name, asm)
            elif op.has_dst and op.has_a and not op.has_b:
                g[op_name] = functools.partial(unary_add_inst, op_name, asm)
            elif not op.has_a and not op.has_b:
                g[op_name] = functools.partial(nullary_add_inst, op_name, asm)
        for waddr, reg in Instruction.REGISTERS.items():
            g[waddr] = reg
        g["rf"] = [Instruction.REGISTERS[f"rf{i}"] for i in range(64)]
        for alias_name, alias_reg in _alias_regs.items():
            g[alias_name] = alias_reg
        for name, sig in Instruction.SIGNALS.items():
            if not name.startswith("smimm_"):  # smimm signal is automatically derived
                g[name] = sig
        result = func(asm, *args, **kwargs)
        g.clear()
        for key, value in g_orig.items():
            g[key] = value
        return result

    return decorator


def _assemble(f, *args, **kwargs):
    """Assemble QPU program to byte string."""
    asm = Assembly()
    f(asm, *args, **kwargs)
    return asm


def assemble(f, *args, **kwargs):
    return list(map(int, _assemble(f, *args, **kwargs)))


def get_label_positions(f, *args, **kwargs
) -> dict[str, int]:
    return {label: 8 * n for label, n in _assemble(f, *args, **kwargs).labels.items()}
