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


import os
from ctypes import Structure, c_uint32, c_uint64
from fcntl import ioctl
from types import TracebackType
from typing import Final, Self

from ioctl_opt import IOW, IOWR

c_uint32x7 = tuple[
    c_uint32,
    c_uint32,
    c_uint32,
    c_uint32,
    c_uint32,
    c_uint32,
    c_uint32,
]

c_uint32x4 = tuple[
    c_uint32,
    c_uint32,
    c_uint32,
    c_uint32,
]


class DRM_V3D:  # noqa: N801
    _fd: int | None

    @property
    def fd(self: Self) -> int | None:
        return self._fd

    def __init__(self: Self, path: str = "/dev/dri/by-path/platform-1002000000.v3d-card") -> None:
        self._fd = os.open(path, os.O_RDWR)

    def close(self: Self) -> None:
        if self._fd is not None:
            os.close(self._fd)
        self._fd = None

    def __enter__(self: Self) -> Self:
        return self

    def __exit__(
        self: Self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: type[TracebackType] | None,
    ) -> bool:
        self.close()
        return exc_value is None

    # Derived from linux/include/uapi/drm/drm.h
    DRM_IOCTL_BASE: Final[int] = ord("d")
    DRM_COMMAND_BASE: Final[int] = 0x40
    DRM_GEM_CLOSE: Final[int] = 0x09

    # Derived from linux/include/uapi/drm/v3d_drm.h
    DRM_V3D_WAIT_BO: Final[int] = DRM_COMMAND_BASE + 0x01
    DRM_V3D_CREATE_BO: Final[int] = DRM_COMMAND_BASE + 0x02
    DRM_V3D_MMAP_BO: Final[int] = DRM_COMMAND_BASE + 0x03
    DRM_V3D_GET_PARAM: Final[int] = DRM_COMMAND_BASE + 0x04
    DRM_V3D_SUBMIT_CSD: Final[int] = DRM_COMMAND_BASE + 0x07

    V3D_PARAM_V3D_UIFCFG: Final[int] = 0
    V3D_PARAM_V3D_HUB_IDENT1: Final[int] = 1
    V3D_PARAM_V3D_HUB_IDENT2: Final[int] = 2
    V3D_PARAM_V3D_HUB_IDENT3: Final[int] = 3
    V3D_PARAM_V3D_CORE0_IDENT0: Final[int] = 4
    V3D_PARAM_V3D_CORE0_IDENT1: Final[int] = 5
    V3D_PARAM_V3D_CORE0_IDENT2: Final[int] = 6
    V3D_PARAM_SUPPORTS_TFU: Final[int] = 7
    V3D_PARAM_SUPPORTS_CSD: Final[int] = 8

    class _st_gem_close(Structure):  # noqa: N801
        _fields_ = [
            ("handle", c_uint32),
            ("pad", c_uint32),
        ]

    class _st_v3d_wait_bo(Structure):  # noqa: N801
        _fields_ = [
            ("handle", c_uint32),
            ("pad", c_uint32),
            ("timeout_ns", c_uint64),
        ]

    class _st_v3d_create_bo(Structure):  # noqa: N801
        _fields_ = [
            ("size", c_uint32),
            ("flags", c_uint32),
            ("handle", c_uint32),
            ("offset", c_uint32),
        ]

    class _st_v3d_mmap_bo(Structure):  # noqa: N801
        _fields_ = [
            ("handle", c_uint32),
            ("flags", c_uint32),
            ("offset", c_uint64),
        ]

    class _st_v3d_get_param(Structure):  # noqa: N801
        _fields_ = [
            ("param", c_uint32),
            ("pad", c_uint32),
            ("value", c_uint64),
        ]

    class _st_v3d_submit_csd(Structure):  # noqa: N801
        _fields_ = [
            ("cfg", c_uint32 * 7),
            ("coef", c_uint32 * 4),
            ("bo_handles", c_uint64),
            ("bo_handle_count", c_uint32),
            ("in_sync", c_uint32),
            ("out_sync", c_uint32),
            ("perfmon_id", c_uint32),
            ("extensions", c_uint64),
            ("flags", c_uint32),
            ("pad", c_uint32),
        ]

        # NOTE: Since mypy treats the types of fields in _fields_ as Any,
        # explicit getter and setter methods are defined to ensure proper type annotations.

        @property
        def cfg(self: Self) -> c_uint32x7:
            return self.cfg

        @cfg.setter
        def cfg(self: Self, value: c_uint32x7) -> None:
            self.cfg = value

        @property
        def coef(self: Self) -> c_uint32x4:
            return self.coef

        @coef.setter
        def coef(self: Self, value: c_uint32x4) -> None:
            self.coef = value

        @property
        def bo_handles(self: Self) -> c_uint64:
            return self.bo_handles

        @bo_handles.setter
        def bo_handles(self: Self, value: c_uint64) -> None:
            self.bo_handles = value

        @property
        def bo_handle_count(self: Self) -> c_uint32:
            return self.bo_handle_count

        @bo_handle_count.setter
        def bo_handle_count(self: Self, value: c_uint32) -> None:
            self.bo_handle_count = value

        @property
        def in_sync(self: Self) -> c_uint32:
            return self.in_sync

        @in_sync.setter
        def in_sync(self: Self, value: c_uint32) -> None:
            self.in_sync = value

        @property
        def out_sync(self: Self) -> c_uint32:
            return self.out_sync

        @out_sync.setter
        def out_sync(self: Self, value: c_uint32) -> None:
            self.out_sync = value

    IOCTL_GEM_CLOSE: Final[int] = IOW(DRM_IOCTL_BASE, DRM_GEM_CLOSE, _st_gem_close)

    IOCTL_V3D_WAIT_BO: Final[int] = IOWR(DRM_IOCTL_BASE, DRM_V3D_WAIT_BO, _st_v3d_wait_bo)
    IOCTL_V3D_CREATE_BO: Final[int] = IOWR(DRM_IOCTL_BASE, DRM_V3D_CREATE_BO, _st_v3d_create_bo)
    IOCTL_V3D_MMAP_BO: Final[int] = IOWR(DRM_IOCTL_BASE, DRM_V3D_MMAP_BO, _st_v3d_mmap_bo)
    IOCTL_V3D_GET_PARAM: Final[int] = IOWR(DRM_IOCTL_BASE, DRM_V3D_GET_PARAM, _st_v3d_get_param)
    IOCTL_V3D_SUBMIT_CSD: Final[int] = IOW(DRM_IOCTL_BASE, DRM_V3D_SUBMIT_CSD, _st_v3d_submit_csd)

    def gem_close(self: Self, handle: int) -> None:
        if self._fd is None:
            raise ValueError("already closed")
        if handle < 0 or 0xFFFFFFFF < handle:
            raise ValueError("handle is out of range")
        st = self._st_gem_close(
            handle=handle,
            pad=0,
        )
        ioctl(self._fd, self.IOCTL_GEM_CLOSE, st)

    def v3d_wait_bo(self: Self, handle: int, timeout_ns: int) -> None:
        if self._fd is None:
            raise ValueError("already closed")
        if handle < 0 or 0xFFFFFFFF < handle:
            raise ValueError("handle is out of range")
        if timeout_ns < 0 or 0xFFFFFFFFFFFFFFFF < timeout_ns:
            raise ValueError("timeout_ns is out of range")
        st = self._st_v3d_wait_bo(
            handle=handle,
            pad=0,
            timeout_ns=timeout_ns,
        )
        ioctl(self._fd, self.IOCTL_V3D_WAIT_BO, st)

    def v3d_create_bo(self: Self, size: int, flags: int = 0) -> tuple[int, int]:
        if self._fd is None:
            raise ValueError("already closed")
        if size < 0 or 0xFFFFFFFF < size:
            raise ValueError("size is out of range")
        if flags < 0 or 0xFFFFFFFF < flags:
            raise ValueError("flags is out of range")
        st = self._st_v3d_create_bo(
            size=size,
            flags=flags,
            handle=0,
            offset=0,
        )
        ioctl(self._fd, self.IOCTL_V3D_CREATE_BO, st)
        return int(st.handle), int(st.offset)

    def v3d_mmap_bo(self: Self, handle: int, flags: int = 0) -> int:
        if self._fd is None:
            raise ValueError("already closed")
        if handle < 0 or 0xFFFFFFFF < handle:
            raise ValueError("handle is out of range")
        if flags < 0 or 0xFFFFFFFF < flags:
            raise ValueError("flags is out of range")
        st = self._st_v3d_mmap_bo(
            handle=handle,
            flags=flags,
            offset=0,
        )
        ioctl(self._fd, self.IOCTL_V3D_MMAP_BO, st)
        return int(st.offset)

    def v3d_get_param(self: Self, param: int) -> int:
        if self._fd is None:
            raise ValueError("already closed")
        if param < 0 or 0xFFFFFFFF < param:
            raise ValueError("param is out of range")
        st = self._st_v3d_get_param(
            param=param,
            pad=0,
            value=0,
        )
        ioctl(self._fd, self.IOCTL_V3D_GET_PARAM, st)
        return int(st.value)

    def v3d_submit_csd(
        self: Self,
        cfg: tuple[int, int, int, int, int, int, int],
        coef: tuple[int, int, int, int],
        bo_handles: int,
        bo_handle_count: int,
        in_sync: int,
        out_sync: int,
    ) -> None:
        if self._fd is None:
            raise ValueError(f"fd({self._fd}) already closed")
        if len(cfg) != 7:
            raise ValueError(f"cfg has invalid length({len(cfg)})")
        if len(coef) != 4:
            raise ValueError(f"coef has invalid length({len(coef)})")
        for i, x in enumerate(cfg):
            if x < 0 or 0xFFFFFFFF < x:
                raise ValueError(f"cfg[{i}]({x}) has out of range value")
        for i, x in enumerate(coef):
            if x < 0 or 0xFFFFFFFF < x:
                raise ValueError(f"coef[{i}]({x}) has out of range value")
        if bo_handles < 0 or 0xFFFFFFFFFFFFFFFF < bo_handles:
            raise ValueError(f"bo_handles({bo_handles}) is out of range")
        if bo_handle_count < 0 or 0xFFFFFFFF < bo_handle_count:
            raise ValueError(f"bo_handle_count({bo_handle_count}) is out of range")
        if in_sync < 0 or 0xFFFFFFFF < in_sync:
            raise ValueError(f"in_sync({in_sync}) is out of range")
        if out_sync < 0 or 0xFFFFFFFF < out_sync:
            raise ValueError(f"out_sync({out_sync}) is out of range")
        st = self._st_v3d_submit_csd(
            # XXX: Dirty hack!
            cfg=(c_uint32 * 7)(*cfg),
            coef=(c_uint32 * 4)(*coef),
            bo_handles=bo_handles,
            bo_handle_count=bo_handle_count,
            in_sync=in_sync,
            out_sync=out_sync,
            perfmon_id=0,
            extensions=0,
            flags=0,
            pad=0,
        )
        ioctl(self._fd, self.IOCTL_V3D_SUBMIT_CSD, st)
