# Copyright (c) 2014-2018 Broadcom
# Copyright (c) 2025- Idein Inc.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
# Street, Fifth Floor, Boston, MA 02110-1301 USA.

import mmap
import os
from ctypes import c_uint32, c_void_p
from types import TracebackType
from typing import Final, Protocol, Self

import numpy as np

from _videocore7.readwrite4 import read4, write4


class SupportsValue(Protocol):
    @property
    def value(self: Self) -> int: ...
    @value.setter
    def value(self: Self, value: int) -> None: ...


class Register:
    _ptr: int
    _offset: int

    def __init__(self: Self, ptr: int, offset: int) -> None:
        self._ptr = ptr
        self._offset = offset

    @property
    def value(self: Self) -> int:
        return int(read4(c_void_p(self._ptr + self._offset)))

    @value.setter
    def value(self: Self, value: int) -> None:
        write4(c_void_p(self._ptr + self._offset), c_uint32(value))

    def _proof_compliant(self: Self) -> None:
        _proof_register: SupportsValue = self


class Field:
    _reg: Register
    _mask: int
    _shift: int

    def __init__(self: Self, reg: Register, high: int, low: int) -> None:
        self._reg = reg
        self._mask = ((1 << (high - low + 1)) - 1) << low
        self._shift = low

    @property
    def value(self: Self) -> int:
        return (self._reg.value & self._mask) >> self._shift

    @value.setter
    def value(self: Self, value: int) -> None:
        self._reg.value = (self._reg.value & ~self._mask) | ((int(value) << self._shift) & self._mask)

    def _proof_compliant(self: Self) -> None:
        _proof_register: SupportsValue = self


# V3D register definitions derived from linux/drivers/gpu/drm/v3d/v3d_regs.h


class HubIdent1(Register):
    _with_mso: Field
    _with_tsy: Field
    _with_tfu: Field
    _with_l3c: Field
    _nhosts: Field
    _ncores: Field
    _rev: Field
    _tver: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x0000C)
        self._with_mso = Field(self, 19, 19)
        self._with_tsy = Field(self, 18, 18)
        self._with_tfu = Field(self, 17, 17)
        self._with_l3c = Field(self, 16, 16)
        self._nhosts = Field(self, 15, 12)
        self._ncores = Field(self, 11, 8)
        self._rev = Field(self, 7, 4)
        self._tver = Field(self, 3, 0)

    @property
    def WITH_MSO(self: Self) -> Field:  # noqa: N802
        return self._with_mso

    @property
    def WITH_TSY(self: Self) -> Field:  # noqa: N802
        return self._with_tsy

    @property
    def WITH_TFU(self: Self) -> Field:  # noqa: N802
        return self._with_tfu

    @property
    def WITH_L3C(self: Self) -> Field:  # noqa: N802
        return self._with_l3c

    @property
    def NHOSTS(self: Self) -> Field:  # noqa: N802
        return self._nhosts

    @property
    def NCORES(self: Self) -> Field:  # noqa: N802
        return self._ncores

    @property
    def REV(self: Self) -> Field:  # noqa: N802
        return self._rev

    @property
    def TVER(self: Self) -> Field:  # noqa: N802
        return self._tver


class HubIdent2(Register):
    _mmu: Field
    _l3c_nkb: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00010)
        self._mmu = Field(self, 8, 8)
        self._l3c_nkb = Field(self, 7, 0)

    @property
    def WITH_MMU(self: Self) -> Field:  # noqa: N802
        return self._mmu

    @property
    def L3C_NKB(self: Self) -> Field:  # noqa: N802
        return self._l3c_nkb


class HubIdent3(Register):
    _iprev: Field
    _ipidx: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00014)
        self._iprev = Field(self, 15, 8)
        self._ipidx = Field(self, 7, 0)

    @property
    def IPREV(self: Self) -> Field:  # noqa: N802
        return self._iprev

    @property
    def IPIDX(self: Self) -> Field:  # noqa: N802
        return self._ipidx


class Hub:
    _axicfg: Register
    _uifcfg: Register
    _ident0: Register
    _ident1: HubIdent1
    _ident2: HubIdent2
    _ident3: HubIdent3
    _tfu_cs: Register

    def __init__(self: Self, ptr: int) -> None:
        self._axicfg = Register(ptr, 0x00000)
        self._uifcfg = Register(ptr, 0x00004)
        self._ident0 = Register(ptr, 0x00008)
        self._ident1 = HubIdent1(ptr)
        self._ident2 = HubIdent2(ptr)
        self._ident3 = HubIdent3(ptr)
        self._tfu_cs = Register(ptr, 0x00700)

    @property
    def AXICFG(self: Self) -> Register:  # noqa: N802
        return self._axicfg

    @property
    def UIFCFG(self: Self) -> Register:  # noqa: N802
        return self._uifcfg

    @property
    def IDENT0(self: Self) -> Register:  # noqa: N802
        return self._ident0

    @property
    def IDENT1(self: Self) -> HubIdent1:  # noqa: N802
        return self._ident1

    @property
    def IDENT2(self: Self) -> HubIdent2:  # noqa: N802
        return self._ident2

    @property
    def IDENT3(self: Self) -> HubIdent3:  # noqa: N802
        return self._ident3

    @property
    def TFU_CS(self: Self) -> Register:  # noqa: N802
        return self._tfu_cs


class CoreIdent0(Register):
    _ver: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00000)
        self._ver = Field(self, 31, 24)

    @property
    def VER(self: Self) -> Field:  # noqa: N802
        return self._ver


class CoreIdent1(Register):
    _vpm_size: Field
    _nsem: Field
    _ntmu: Field
    _qups: Field
    _nslc: Field
    _rev: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00004)
        self._vpm_size = Field(self, 31, 28)
        self._nsem = Field(self, 23, 16)
        self._ntmu = Field(self, 15, 12)
        self._qups = Field(self, 11, 8)
        self._nslc = Field(self, 7, 4)
        self._rev = Field(self, 3, 0)

    @property
    def VPM_SIZE(self: Self) -> Field:  # noqa: N802
        return self._vpm_size

    @property
    def NSEM(self: Self) -> Field:  # noqa: N802
        return self._nsem

    @property
    def NTMU(self: Self) -> Field:  # noqa: N802
        return self._ntmu

    @property
    def QUPS(self: Self) -> Field:  # noqa: N802
        return self._qups

    @property
    def NSLC(self: Self) -> Field:  # noqa: N802
        return self._nslc

    @property
    def REV(self: Self) -> Field:  # noqa: N802
        return self._rev


class CoreIdent2(Register):
    _bcg: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00008)
        self._bcg = Field(self, 28, 28)

    @property
    def BCG(self: Self) -> Field:  # noqa: N802
        return self._bcg


class CoreMiscCfg(Register):
    _qrmaxcnt: Field
    _ovrtmuout: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00018)
        self._qrmaxcnt = Field(self, 3, 1)
        self._ovrtmuout = Field(self, 0, 0)

    @property
    def QRMAXCNT(self: Self) -> Field:  # noqa: N802
        return self._qrmaxcnt

    @property
    def OVRTMUOUT(self: Self) -> Field:  # noqa: N802
        return self._ovrtmuout


class CoreL2CACTL(Register):
    _l2cclr: Field
    _l2cdis: Field
    _l2cena: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00020)
        self._l2cclr = Field(self, 2, 2)
        self._l2cdis = Field(self, 1, 1)
        self._l2cena = Field(self, 0, 0)

    @property
    def L2CCLR(self: Self) -> Field:  # noqa: N802
        return self._l2cclr

    @property
    def L2CDIS(self: Self) -> Field:  # noqa: N802
        return self._l2cdis

    @property
    def L2CENA(self: Self) -> Field:  # noqa: N802
        return self._l2cena


class CoreSLCACTL(Register):
    _tvccs: Field
    _tdccs: Field
    _ucc: Field
    _icc: Field

    def __init__(self: Self, ptr: int) -> None:
        super().__init__(ptr, 0x00024)
        self._tvccs = Field(self, 27, 24)
        self._tdccs = Field(self, 19, 16)
        self._ucc = Field(self, 11, 8)
        self._icc = Field(self, 3, 0)

    @property
    def TVCCS(self: Self) -> Field:  # noqa: N802
        return self._tvccs

    @property
    def TDCCS(self: Self) -> Field:  # noqa: N802
        return self._tdccs

    @property
    def UCC(self: Self) -> Field:  # noqa: N802
        return self._ucc

    @property
    def ICC(self: Self) -> Field:  # noqa: N802
        return self._icc


class CorePctr0Src(Register):
    _s: list[Field]

    def __init__(self: Self, ptr: int, n: int) -> None:
        super().__init__(ptr, 0x00660 + n)
        self._s = [
            Field(self, 7, 0),
            Field(self, 15, 8),
            Field(self, 23, 16),
            Field(self, 31, 24),
        ]

    @property
    def S(self: Self) -> list[Field]:  # noqa: N802
        return self._s

    @property
    def S0(self: Self) -> Field:  # noqa: N802
        return self._s[0]

    @property
    def S1(self: Self) -> Field:  # noqa: N802
        return self._s[1]

    @property
    def S2(self: Self) -> Field:  # noqa: N802
        return self._s[2]

    @property
    def S3(self: Self) -> Field:  # noqa: N802
        return self._s[3]


class Core:
    _ident0: CoreIdent0
    _ident1: CoreIdent1
    _ident2: CoreIdent2
    _misccfg: CoreMiscCfg
    _l2cactl: CoreL2CACTL
    _slcactl: CoreSLCACTL
    _pctr_0_en: Register
    _pctr_0_clr: Register
    _pctr_0_overflow: Register
    _pctr_0_src: list[CorePctr0Src]
    _pctr_0_pctr: list[Register]

    def __init__(self: Self, ptr: int, core_id: int) -> None:
        self._ident0 = CoreIdent0(ptr)
        self._ident1 = CoreIdent1(ptr)
        self._ident2 = CoreIdent2(ptr)
        self._misccfg = CoreMiscCfg(ptr)
        self._l2cactl = CoreL2CACTL(ptr)
        self._slcactl = CoreSLCACTL(ptr)
        self._pctr_0_en = Register(ptr, 0x00650)
        self._pctr_0_clr = Register(ptr, 0x00654)
        self._pctr_0_overflow = Register(ptr, 0x00658)
        self._pctr_0_src = [CorePctr0Src(ptr, i) for i in range(0, 32, 4)]
        self._pctr_0_pctr = [Register(ptr, 0x00680 + 4 * i) for i in range(32)]

    @property
    def IDENT0(self: Self) -> CoreIdent0:  # noqa: N802
        return self._ident0

    @property
    def IDENT1(self: Self) -> CoreIdent1:  # noqa: N802
        return self._ident1

    @property
    def IDENT2(self: Self) -> CoreIdent2:  # noqa: N802
        return self._ident2

    @property
    def MISCCFG(self: Self) -> CoreMiscCfg:  # noqa: N802
        return self._misccfg

    @property
    def L2CACTL(self: Self) -> CoreL2CACTL:  # noqa: N802
        return self._l2cactl

    @property
    def SLCACTL(self: Self) -> CoreSLCACTL:  # noqa: N802
        return self._slcactl

    @property
    def PCTR_0_EN(self: Self) -> Register:  # noqa: N802
        return self._pctr_0_en

    @property
    def PCTR_0_CLR(self: Self) -> Register:  # noqa: N802
        return self._pctr_0_clr

    @property
    def PCTR_0_OVERFLOW(self: Self) -> Register:  # noqa: N802
        return self._pctr_0_overflow

    @property
    def PCTR_0_SRC(  # noqa: N802
        self: Self,
    ) -> list[CorePctr0Src]:
        return self._pctr_0_src

    @property
    def PCTR_0_SRC_0_3(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[0]

    @property
    def PCTR_0_SRC_4_7(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[1]

    @property
    def PCTR_0_SRC_8_11(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[2]

    @property
    def PCTR_0_SRC_12_15(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[3]

    @property
    def PCTR_0_SRC_16_19(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[4]

    @property
    def PCTR_0_SRC_20_23(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[5]

    @property
    def PCTR_0_SRC_24_27(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[6]

    @property
    def PCTR_0_SRC_28_31(self: Self) -> CorePctr0Src:  # noqa: N802
        return self._pctr_0_src[7]

    @property
    def PCTR_0_PCTR(  # noqa: N802
        self: Self,
    ) -> list[Register]:
        return self._pctr_0_pctr


CORE_PCTR_CYCLE_COUNT: Final[int] = 0


class RegisterMapping:
    NUM_OF_CORES: Final[int] = 1

    _hub: Hub
    _core: list[Core]
    _map: list[mmap.mmap]

    def __init__(self: Self) -> None:
        self._core = []
        self._map = []

        fd = os.open("/dev/mem", os.O_RDWR)

        path: str
        names: list[str]
        reg_addrs: dict[str, tuple[int, int]] = {}
        with open("/proc/device-tree/__symbols__/v3d", "rb") as f:
            path = f.read().decode("utf-8").split("\0")[0]
        with open("/proc/device-tree" + path + "/reg-names", "rb") as f:
            names = [name for name in f.read().decode("utf-8").split("\0") if len(name) > 0]
        with open("/proc/device-tree" + path + "/reg", "rb") as f:
            for name in names:
                addr = int.from_bytes(f.read(8))
                size = int.from_bytes(f.read(8))
                reg_addrs[name] = (addr, size)

        if "hub" not in reg_addrs:
            raise RuntimeError("Failed to get hub register addresses.")
        map_hub = mmap.mmap(
            offset=reg_addrs["hub"][0],
            length=reg_addrs["hub"][1],
            fileno=fd,
            flags=mmap.MAP_SHARED,
            prot=mmap.PROT_READ | mmap.PROT_WRITE,
        )
        ptr_hub = np.frombuffer(map_hub).ctypes.data
        self._hub = Hub(ptr_hub)
        self._map.append(map_hub)

        self._core = []
        for core in range(RegisterMapping.NUM_OF_CORES):
            if f"core{core}" not in reg_addrs:
                raise RuntimeError(f"Failed to get core{core} register addresses.")
            map_core = mmap.mmap(
                offset=reg_addrs[f"core{core}"][0],
                length=reg_addrs[f"core{core}"][1],
                fileno=fd,
                flags=mmap.MAP_SHARED,
                prot=mmap.PROT_READ | mmap.PROT_WRITE,
            )
            ptr_core = np.frombuffer(map_core).ctypes.data
            self._core.append(Core(ptr_core, core))
            self._map.append(map_core)

        os.close(fd)

    def __enter__(self: Self) -> Self:
        return self

    def __exit__(
        self: Self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: type[TracebackType] | None,
    ) -> None:
        for m in self._map:
            m.close()

    @property
    def HUB(self: Self) -> Hub:  # noqa: N802
        return self._hub

    @property
    def CORE(self: Self) -> list[Core]:  # noqa: N802
        return self._core


class PerformanceCounter:
    _reg: RegisterMapping
    _srcs: list[int]
    _core: int
    _mask: int

    def __init__(self: Self, _regmap: RegisterMapping, srcs: list[int], core: int = 0) -> None:
        self._reg = _regmap
        self._srcs = srcs
        self._core = core  # Sufficient for now.
        self._mask = (1 << len(self._srcs)) - 1

    def __enter__(self: Self) -> Self:
        pctr_srcs = self._reg.CORE[self._core].PCTR_0_SRC
        for reg, value in zip(sum([s.S for s in pctr_srcs], []), self._srcs):
            reg.value = value

        self._reg.CORE[self._core].PCTR_0_EN.value = self._mask
        self._reg.CORE[self._core].PCTR_0_CLR.value = self._mask
        self._reg.CORE[self._core].PCTR_0_OVERFLOW.value = self._mask

        return self

    def __exit__(
        self: Self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: type[TracebackType] | None,
    ) -> None:
        self._reg.CORE[self._core].PCTR_0_EN.value = 0
        self._reg.CORE[self._core].PCTR_0_CLR.value = self._mask
        self._reg.CORE[self._core].PCTR_0_OVERFLOW.value = self._mask

    def result(self: Self):
        return [reg.value for reg in self._reg.CORE[self._core].PCTR_0_PCTR[: len(self._srcs)]]
