# GPU 服务设计（gpud）

> 状态：设计定稿，按阶段落地中
> 相关代码：`foot/service/gpud.c`、`foot/driver/gpu/`、`foot/driver/v3d/`、`eggs/libgpu/`

## 1. 背景与目标

YiYiYa 的 GPU 驱动是**用户态服务**（不是内核模块），通过 foot 协议给应用提供图形能力。

目标：

| 目标 | 约束 |
|---|---|
| 应用不感知具体 GPU | 只认服务名 `"gpu"`，不出现 `v3d`/`mali` 字样 |
| 后端可插拔 | v3d 是第一个实现，以后加 mali/adreno/软件回退，接口不变 |
| 支持 2D 与 3D | 高层便捷 2D（FILL/BLIT）+ 低层命令流（为 Mesa3D 接入准备） |
| 与 vfsd 对称 | `gpud` 是"服务进程 + 内部使用 driver 能力"，和 `vfsd` 同构 |

## 2. 分层架构

```
┌──────────────────────────────────────────────────────────┐
│  应用 / GUI / 合成器 / Mesa3D                             │
├──────────────────────────────────────────────────────────┤
│  eggs/libgpu（应用侧通用客户端）                          │
│    高层：gpu_query/gpu_fill/gpu_blit/gpu_sync             │
│    低层：gpu_bo_create/gpu_submit_cl/gpu_wait_bo          │
├──────────────────────────────────────────────────────────┤
│  foot/service/gpud（独立进程，注册服务名 "gpu"）          │
│    gpu_fn 分发 → gpu_backend_t 方法                       │
├──────────────────────────────────────────────────────────┤
│  gpu_backend_t 后端抽象（foot/driver/gpu/gpu_backend.h）  │
│    v3d_backend / mali_backend / sw_backend                │
├──────────────────────────────────────────────────────────┤
│  硬件（V3D TFU/CSD、Mali、或 CPU memcpy）                 │
└──────────────────────────────────────────────────────────┘
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
  g_be = &v3d_backend;          /* 按平台/探测选后端 */
  if (g_be->init() != 0) return 1;

  client_t* c = client_regist("gpu");
  client_run(c, gpu_fn);        /* gpu_fn 分发到 g_be->xxx() */
  return 0;
}
```

- 服务名固定 `"gpu"`（不是 `"gpu.v3d"`），应用不感知后端
- 后端选择在 gpud 内部（第一阶段固定 v3d，以后按平台宏/探测）
- 硬件初始化（SMS/PM/MMU/L2C/屏幕）全部在 `backend->init()` 里

## 4. 后端抽象（`gpu_backend_t`）

```c
typedef struct gpu_backend {
  const char* name;        /* "v3d" */
  u32 backend_id;          /* GPU_BACKEND_V3D */

  int (*init)(void);
  int (*query)(void* out);         /* 填 gpu_info_t */

  /* 高层（所有后端都实现，sw 也要有） */
  int (*fill)(void* args);
  int (*blit)(void* args);
  int (*sync)(void* args);

  /* 低层（可选，NULL = 不支持；为 Mesa 准备） */
  int (*bo_create)(void* args);
  int (*bo_mmap)(void* args);
  int (*bo_destroy)(void* args);
  int (*submit_cl)(void* args);
  int (*submit_csd)(void* args);
  int (*wait_bo)(void* args);
} gpu_backend_t;
```

## 5. 通用接口

### 5.1 高层（便捷 2D）

```c
enum gpu_fn {
  GPU_FN_QUERY = 0,   /* 返回 gpu_info_t（能力 + 屏幕信息） */
  GPU_FN_FILL  = 1,   /* 矩形填充 */
  GPU_FN_BLIT  = 2,   /* 位块搬移 */
  GPU_FN_SYNC  = 3,   /* 等 GPU 空闲 */
};

typedef struct {
  u32 x, y, w, h;
  u32 color;          /* 统一 0xAARRGGBB，后端按自己格式转换 */
} gpu_fill_req;
```

### 5.2 低层（为 Mesa 接入准备）

对应 `drm_v3d` 的 ioctl，命令流放 BO、提交只传句柄+偏移：

```c
enum gpu_fn_low {
  GPU_LOW_GET_PARAM     = 100,
  GPU_LOW_GET_BO_OFFSET = 101,
  GPU_LOW_BO_CREATE     = 110,
  GPU_LOW_BO_MMAP       = 111,
  GPU_LOW_BO_DESTROY    = 112,
  GPU_LOW_SUBMIT_CL     = 120,
  GPU_LOW_SUBMIT_CSD    = 121,
  GPU_LOW_SUBMIT_TFU    = 122,
  GPU_LOW_WAIT_BO       = 130,
};
```

**为什么照 DRM 形状设计**：Mesa 的 `v3dv` 内部写死调用 `drmIoctl(fd, DRM_IOCTL_V3D_*, ...)`。要么改 libdrm（把 ioctl 换成 `client_call`），要么内核做 DRM 兼容层。两种都要求底层语义与 `drm_v3d` 一一对应，不能自创抽象。

### 5.3 能力协商

```c
typedef struct {
  u32 api_version;
  u32 backend_id;          /* GPU_BACKEND_V3D */
  u32 flags;               /* GPU_CAP_* */
  u32 max_width, max_height;
  u32 fmt_mask;            /* GPU_FMT_* 位图 */
  u32 screen_w, screen_h, screen_fmt;
} gpu_info_t;

#define GPU_CAP_FILL    (1u << 0)
#define GPU_CAP_BLIT    (1u << 1)
#define GPU_CAP_COMPUTE (1u << 4)
```

应用先 `GPU_FN_QUERY`，按能力位决定走硬件还是回退 CPU。

## 6. Mesa3D 接入规划

```
Mesa v3dv → libdrm(适配层) → client_call(gpu, GPU_LOW_*) → gpud → backend
```

- Mesa 负责生成 CL 命令流、编译 QPU shader（全是**不透明 blob**）
- gpud/backend 只负责 BO 管理、提交、同步
- **3D 的复杂性全部在 Mesa，GPU 服务不重新发明渲染 API**

## 7. 目录结构

```
eggs/libgpu/                # 应用侧通用客户端库
    gpu.h                   # 通用接口定义
    libgpu.c                # gpu_open/query/fill/blit

foot/service/
    gpud.c                  # 独立进程：注册 "gpu"，链接后端库
    vfsd.c / systemd.c      # 既有服务

foot/driver/gpu/
    gpu_backend.h           # gpu_backend_t 接口定义

foot/driver/v3d/
    v3d_backend.c           # 实现 gpu_backend_t（SMS/PM/MMU/屏幕/TFU）
    v3d_backend.h           # extern const gpu_backend_t v3d_backend;
    v3d_hw.c / v3d_mmu.c / v3d_tfu.c / v3d_csd.c
```

**依赖方向**：`gpud` → `v3d`（库）→ `gpu_backend.h` → `libfoot`，单向无环。

## 8. 分阶段落地

| 阶段 | 内容 | 验证 |
|---|---|---|
| **1** | `gpu_backend.h` + `v3d_backend.c` + `gpud.c` + `libgpu` 高层 | gpud 启动，应用 `FILL` 填四象限 |
| **2** | 低层 `BO_CREATE/MMAP/DESTROY` + `SUBMIT_TFU` | BO 分配 + TFU 走 BO |
| **3** | 低层 `SUBMIT_CSD` + fence | compute shader 写 BO |
| **4** | 低层 `SUBMIT_CL`（binning+render） | 最小 CL 命令流清屏 |
| **5** | Mesa v3dv + libdrm 适配层 | 跑 Vulkan 三角形 |

## 9. 与现有 v3dd 的关系

`foot/driver/v3d/v3dd.c` 拆分：

| 现在 | 迁移后 |
|---|---|
| `main` 前 90%（capability→屏幕初始化） | `v3d_backend.c` 的 `v3d_init()` |
| `client_regist("v3d")` + `client_run(v3d_fn)` | `gpud.c` 的 `main` |
| `v3d_fn` 分发 | `gpud.c` 的 `gpu_fn` |
| `screen_init` / `screen_fill` | 保留在 `v3d_backend.c` |
| `target("v3dd") cli` | `target("v3d") lib` + `target("gpud") cli` |

**部署变化**：`/bin/v3dd` → `/bin/gpud`，服务名 `"v3d"` → `"gpu"`。
