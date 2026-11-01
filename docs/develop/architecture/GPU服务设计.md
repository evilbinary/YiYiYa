# GPU 服务设计（gpud）

> 状态：设计定稿，按阶段落地中
> 相关代码：`foot/service/gpud.c`、`foot/driver/gpu/`、`eggs/libgpu/`

## 1. 背景与目标

YiYiYa 的 GPU 驱动是**用户态服务**（不是内核模块），通过 foot 协议给应用提供图形能力。

| 目标 | 约束 |
|---|---|
| 应用不感知具体 GPU | 只认服务名 `"gpu"`，不出现 `v3d`/`mali` 字样 |
| 后端可插拔 | v3d 是第一个实现，以后加 mali/adreno/软件回退，接口不变 |
| 支持 2D 与 3D | 高层便捷 2D（FILL/BLIT）+ 低层命令流（为 Mesa3D 接入准备） |
| 与 vfsd 对称 | `gpud` 是"服务进程 + 内部使用 driver 能力"，和 `vfsd` 同构 |
| 不破坏现有程序 | 拿不到 gpud 时自动回退 CPU 路径 |

## 2. 分层架构

```
┌──────────────────────────────────────────────────────────────┐
│ 应用层                                                        │
│   Vulkan/GL 应用              wayland 客户端                   │
├──────────────────────────────────────────────────────────────┤
│ Mesa3D (v3dv/v3d Gallium)      weston（compositor）            │
│   │ libdrm-yiyiya               │ gpud-backend（自研）          │
│   │  drmIoctl → client_call     │ gpud_blit/submit              │
├───┴─────────────────────────────┴────────────────────────────┤
│ foot 服务 "gpu"（gpud 进程，唯一持有 V3D 寄存器 + MMU）        │
│   高层：QUERY / FILL / BLIT / SYNC                             │
│   低层：BO_CREATE/OFFSET/DESTROY                                │
│         SUBMIT_TFU / SUBMIT_CSD / SUBMIT_CL / WAIT_BO          │
├──────────────────────────────────────────────────────────────┤
│ gpu_backend_t（v3d / mali / sw）                               │
├──────────────────────────────────────────────────────────────┤
│ V3D 硬件：TFU（拷贝/填充）+ CSD（计算）+ CL 管线（3D 渲染）    │
└──────────────────────────────────────────────────────────────┘
```

**职责边界**：

- **应用**：只描述"要画什么"，不碰寄存器
- **libgpu**：把调用打包成 `client_call`，管超时/错误码
- **gpud**：唯一持有 GPU 寄存器窗口和 MMU 页表的进程，负责 VA 分配、cache 维护、提交
- **backend**：具体 GPU 的实现（v3d 用 TFU/CSD，sw 用 memcpy）

## 3. gpud 服务进程

和 `vfsd` 完全同构：

```c
int main(void) {
  const gpu_backend_t* g_be = &v3d_backend;   /* 按平台/探测选后端 */
  return gpu_service_run(g_be, "gpu") == 0 ? 0 : 1;
}
```

- 服务名固定 `"gpu"`（不是 `"gpu.v3d"`），应用不感知后端
- 硬件初始化（SMS/PM/MMU/L2C/屏幕）全部在 `backend->init()` 里
- 通用逻辑（方法号分发 + `client_regist` + `client_run`）在 `gpu_service.c`

## 4. 后端抽象（`gpu_backend_t`）

```c
typedef struct gpu_backend {
  const char* name;        /* "v3d" */
  unsigned int backend_id; /* GPU_BACKEND_V3D */

  int (*init)(void);
  int (*query)(void* out);

  /* 高层（所有后端都实现，sw 也要有） */
  int (*fill)(void* args);
  int (*blit)(void* args);
  int (*sync)(void* args);

  /* 低层（可选，NULL = 不支持；为 Mesa 准备） */
  int (*bo_create)(void* args);  /* 导入已有 pa，返回 handle */
  int (*bo_mmap)(void* args);    /* 未用（CPU 映射在应用侧） */
  int (*bo_destroy)(void* args);
  int (*bo_offset)(void* args);  /* handle → GPU VA */
  int (*submit_cl)(void* args);
  int (*submit_csd)(void* args);
  int (*submit_tfu)(void* args);
  int (*wait_bo)(void* args);
} gpu_backend_t;
```

## 5. 通用接口

### 5.1 高层（便捷 2D）

```c
enum gpu_fn {
  GPU_FN_QUERY = 0,   /* 返回值：(screen_w << 16) | screen_h */
  GPU_FN_FILL  = 1,   /* gpu_fill_req */
  GPU_FN_BLIT  = 2,   /* gpu_blit_req */
  GPU_FN_SYNC  = 3,
};

typedef struct {
  unsigned int x, y, w, h;
  unsigned int color;   /* 阶段 1 按屏幕格式解释；阶段 2 统一 0xAARRGGBB */
} gpu_fill_req;
```

> **为什么 QUERY 用返回值而不是 `gpu_info_t` 输出**：foot 的 `client_call`
> 把 args 复制到共享页，语义是"只读输入"（`client_call_ex` 的 args 是
> const，服务端改共享页后客户端拿不到）。完整信息留到阶段 2 用共享内存传。

### 5.2 低层（为 Mesa 准备）

对应 `drm_v3d` 的 ioctl，命令流放 BO、提交只传句柄+偏移：

```c
enum gpu_fn_low {
  /* BO 管理（已实现） */
  GPU_LOW_BO_CREATE     = 110,  /* gpu_bo_import_req → handle */
  GPU_LOW_GET_BO_OFFSET = 101,  /* gpu_bo_ref → GPU VA */
  GPU_LOW_BO_DESTROY    = 112,  /* gpu_bo_ref → 0 */

  /* 提交（待实现） */
  GPU_LOW_SUBMIT_TFU    = 122,
  GPU_LOW_SUBMIT_CSD    = 121,
  GPU_LOW_SUBMIT_CL     = 120,

  /* 同步（待实现） */
  GPU_LOW_WAIT_BO       = 130,
};
```

**为什么照 DRM 形状设计**：Mesa 的 `v3dv` 内部写死调用
`drmIoctl(fd, DRM_IOCTL_V3D_*, ...)`。要么改 libdrm（把 ioctl 换成
`client_call`），要么内核做 DRM 兼容层。两种都要求底层语义与 `drm_v3d`
一一对应，不能自创抽象。

### 5.3 BO 模型（已实现）

```
应用/Mesa                     gpud
  foot_cap_alloc_frame(size)     BO_CREATE(pa, size)
    → pa + 用户 VA                 → 逐页 v3d_mmu_map_page(pt, gva+i*4K, pa+i*4K)
    → CPU 画/shader                 → 记录 {handle → gva, pa, size}
  BO_OFFSET(handle)             → 返回 gva（0x30000000 起，256MB 空间）
  BO_DESTROY(handle)            → 清页表项
```

- **CPU 映射始终在应用侧**（应用不需要把 VA 跨进程传）
- gpud 只管 GPU 侧地址，`gva` 从 `0x30000000` 起 bump 分配（避开屏幕
  `0x20000000` 和 shader `0x10000000`）
- 句柄表 32 项，`handle = slot + 1`（0 表示"无"）

## 6. Wayland + Mesa3D 接入方案

### 6.1 现状

| 组件 | 现状 |
|---|---|
| weston | pixman 渲染器 + `fbdev-backend`（CPU 直接写 `/dev/fb0` mmap） |
| Mesa | **无**（`eggs/` 里只有 DRM 头文件桩 `drm-stubs.c`） |
| libdrm | 无 |
| gpud | 高层 FILL/QUERY/SYNC + 低层 BO 管理 |

### 6.2 三条数据流

**流 A：wayland 合成上屏（阶段 2-3，不依赖 Mesa）**

```
weston pixman 渲染 → wl_shm buffer（CPU，应用侧 pa）
  → gpud backend: BO_CREATE(pa) → handle
  → SUBMIT_TFU(handle → 屏幕)  ← TFU 硬件拷贝
  → 屏幕
```

pixman 仍是 CPU 渲染，GPU 只负责"buffer → 屏幕"的拷贝。收益是减少
CPU memcpy（TFU 是 DMA）。

**流 B：wayland 用 GL 渲染器（阶段 6-7，依赖 Mesa）**

```
weston GL renderer → EGL/GBM → Mesa → libdrm-yiyiya
  → gpud: BO_CREATE + SUBMIT_CL（bcl/rcl 命令流）
  → V3D CL 管线（binning + rendering）
  → 渲染目标 BO → SUBMIT_TFU 上屏
```

**流 C：Vulkan/GL 应用（阶段 5-7）**

```
Vulkan app → v3dv → libdrm-yiyiya → gpud 低层
  → V3D CL 管线
```

### 6.3 提交结构

```c
/* SUBMIT_TFU：按 BO 句柄提交 TFU（阶段 2） */
typedef struct {
  unsigned int src_handle, src_offset;  /* 0 = 屏幕 */
  unsigned int dst_handle, dst_offset;
  unsigned int w, h;
  unsigned int src_fmt, dst_fmt;        /* GPU_FMT_* */
  unsigned int flags;                   /* GPU_SUBMIT_ASYNC */
} gpu_submit_tfu_req;

/* SUBMIT_CL：3D 渲染（阶段 5） */
typedef struct {
  unsigned int bcl_handle, bcl_offset, bcl_size;  /* binning 命令流（在 BO 内） */
  unsigned int rcl_handle, rcl_offset, rcl_size;  /* render 命令流 */
  unsigned int qma_handle, qms_handle;            /* QPU 执行列表/中间存储 */
  unsigned int bo_handles[16];                    /* 本次提交涉及的 BO */
  unsigned int bo_count;
  unsigned int in_fence, out_fence;
  unsigned int flags;
} gpu_submit_cl_req;
```

**为什么命令流放 BO**：foot 的 `client_call` 只有 240 字节参数，几十 KB 的
命令流必须放 BO，提交只传 `{handle, offset, size}`。

### 6.4 同步模型

| 模式 | 行为 | 适用 |
|---|---|---|
| 同步（默认） | SUBMIT 内部等 TFU/CSD BUSY 清零再返回 | 单次操作、UI 主循环 |
| 异步（`GPU_SUBMIT_ASYNC`） | 立即返回 fence | 一帧几十条命令，最后统一 `WAIT_BO` |

阶段 2 先做同步，阶段 4 加异步。

## 7. 目录结构

```
eggs/libgpu/                # 应用侧通用客户端库（阶段 3+）
    gpu.h                   # 通用接口定义
    libgpu.c                # gpu_open/query/fill/blit

foot/service/
    gpud.c                  # 独立进程：选后端 + gpu_service_run
    vfsd.c / systemd.c      # 既有服务

foot/driver/gpu/            # GPU 驱动根目录
    gpu_backend.h           # gpu_backend_t 接口定义
    gpu_service.c           # 通用服务壳（方法号分发 + client_regist + client_run）
    ya.py                   # target("gpu") + add_subs('./*/ya.py')
    v3d/                    # V3D 后端（VideoCore VII）
        v3d_backend.c       # 实现 gpu_backend_t（init/query/fill/BO）
        v3d_backend.h       # extern const gpu_backend_t v3d_backend;
        v3d_hw.c / v3d_mmu.c / v3d_tfu.c / v3d_csd.c
    mali/                   # Mali 后端（占位，init 返回 -1）

eggs/libdrm-yiyiya/         # 阶段 6：drmIoctl → client_call（待建）
eggs/weston/compat/backend-gpud.c   # 阶段 3：weston 的 gpud 后端（待建）
```

**依赖方向**：`gpud` → `v3d`（库）→ `gpu_backend.h` → `libfoot`，单向无环。

## 8. 分阶段落地

| 阶段 | 内容 | 状态 | 验证 |
|---|---|---|---|
| **1** | `gpu_backend.h` + `v3d_backend.c` + `gpud.c` + 高层 FILL | ✅ 完成 | `test-v3d` 四象限 `pass=6 fail=0` |
| **1.5** | 低层 BO 管理（`BO_CREATE/OFFSET/DESTROY`） | ✅ 完成 | 句柄表 + GPU VA 映射 |
| **2** | `SUBMIT_TFU`（按 BO 句柄提交拷贝/填充） | 待做 | BO → 屏幕，读回验证 |
| **3** | weston `gpud-backend` | 待做 | weston 用 GPU 上屏（替换 fbdev） |
| **4** | `SUBMIT_CSD` + fence | 待做 | compute shader 写 BO |
| **5** | `SUBMIT_CL`（binning+render） | 待做 | 最小 CL 命令流清屏 |
| **6** | `libdrm-yiyiya` 适配层 | 待做 | `drmIoctl` 转发到 gpud |
| **7** | Mesa v3dv 移植 | 待做 | 跑 Vulkan demo |

## 9. 关键风险与取舍

| 风险 | 说明 | 对策 |
|---|---|---|
| **Mesa 移植工作量** | v3dv 需要 shader 编译（NIR→QPU）、Vulkan runtime、libdrm 适配，数千行 + 大量调试 | 放到最后，先做 2-5 让 2D/wayland 能用 |
| **CL 命令流格式** | bcl/rcl 是 V3D 私有格式（`v3d_packet.xml`），手写极易出错 | 先做最小命令流（清屏），逐步扩展 |
| **无 DRM/GEM** | Mesa 假设有 `/dev/dri` 和 GEM ioctl | 用 `libdrm-yiyiya` 把 ioctl 映射到 `client_call`，fd 用假句柄 |
| **pixman 仍是 CPU** | 流 A 只能省 memcpy，不能真正加速合成 | 真正的 GPU 合成要等流 B（Mesa） |
| **240 字节参数限制** | 提交涉及的 BO 列表不能太大 | 16 个/次 + 分批注册 |
| **foot args 只读** | `client_call` 的 args 不能回写输出结构 | 输出走返回值；复杂结构留到用共享内存 |

## 10. 与旧 v3dd 的关系（已迁移）

`foot/driver/v3d/v3dd.c` 已拆分：

| 旧 | 新 |
|---|---|
| `main` 前 90%（capability→屏幕初始化） | `v3d_backend.c` 的 `v3d_init()` |
| `client_regist("v3d")` + `client_run(v3d_fn)` | `gpud.c` 的 `main` + `gpu_service.c` |
| `v3d_fn` 分发 | `gpu_service.c` 的 `gpu_service_fn` |
| `screen_init` / `screen_fill` | 保留在 `v3d_backend.c` |
| `target("v3dd") cli` | `target("v3d") lib` + `target("gpud") cli` |

**部署变化**：`/bin/v3dd` → `/bin/gpud`，服务名 `"v3d"` → `"gpu"`。
