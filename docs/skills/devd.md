# devd —— 设备文件传输 / 远程执行 / 日志回读

> 一台跑 YiYiYa 的板子（如树莓派 5）只要开着 `devd`，就不需要再拔 SD 卡、插读卡器、
> 烧写、重启来验证改动。改完代码直接 `curl` 推过去、`curl` 拉日志回来。
>
> 源码：`app/cmd/devd.c`（19KB），target：`devd`，默认端口 **9004**。

---

## 1. 它解决什么问题

嵌入式开发的循环通常是：

```
改代码 → 编译 → 拔卡 → 读卡器 → 拷文件 → 插回 → 上电 → 看串口
```

一轮 5~10 分钟，而且串口日志只能靠人看、靠手抄。

`devd` 把这一圈压缩成三条命令：

```sh
curl -T kernel8.img http://<ip>:9004/kernel8.img     # 推（含内核）
curl -X POST http://<ip>:9004/reboot                 # 重启生效
curl http://<ip>:9004/etc/sshd.log -o /tmp/log       # 拉日志
```

一轮 20~30 秒，且日志能落到本地文件里 grep。

---

## 2. 快速开始

设备端（只需要一次）：

```sh
/bin/devd
# devd: starting on port 9004
# devd: bound port 9004
# devd: ready  curl -T <file> http://<ip>:9004/<path>
# devd: background pid=10
```

想开机自启，在 `/conf/init.conf` 里加一行：

```
/bin/devd
```

---

## 3. 接口速查

| 方法 | 路径 | 作用 |
|---|---|---|
| `PUT` | `/<path>` | 上传文件（FAT 上的相对路径，如 `/bin/xx`、`/kernel8.img`） |
| `GET` | `/<path>` | 下载文件（日志、配置、二进制校验） |
| `POST` | `/run` | body 是要执行的命令，返回 stdout（+ 前台命令的 stderr） |
| `POST` | `/reboot` | 走 PSCI 复位设备（配合推内核实现全自动） |

### 上传

```sh
curl -T app/resource/kernel8.img http://192.168.1.250:9004/kernel8.img
# HTTP 200 OK
```

先写 `<path>.new` 再 rename，**不会把正在运行的程序写坏**。

### 直接写（绕过 rename）

FAT 根目录的 `rename` 目前是坏的（根节点 `fat_path` 为空，`fat_volume_path`
直接返回 -1），而 `kernel8.img` 必须在根目录。加 `X-Direct: 1` 跳过
tmp+rename 直接写目标文件：

```sh
curl -H "X-Direct: 1" -T kernel8.img http://192.168.1.250:9004/kernel8.img
```

**先备份再直写**，中断不会把启动盘写废：

```sh
curl http://192.168.1.250:9004/kernel8.img -o /tmp/k8.bak
```

### 下载

```sh
curl http://192.168.1.250:9004/etc/sshd.log -o /tmp/sshd.log
curl http://192.168.1.250:9004/bin/sshd-session -o /tmp/sess \
     -w "size=%{size_download}\n"        # 顺便校验大小
```

### 远程执行

```sh
curl -X POST --data-binary "/bin/ls /bin" http://192.168.1.250:9004/run
curl -X POST --data-binary "ps"           http://192.168.1.250:9004/run
```

注意：这个 `sh` 没有 `echo` 内建、不支持 `>` `&` 之外的重定向，外部命令都正常。

### 重启

```sh
curl -X POST http://192.168.1.250:9004/reboot
```

内核侧 `sys_reboot` 走 PSCI `SYSTEM_RESET`（aarch64 编号 142）。

---

## 4. 踩坑记录（改之前务必看）

这些都是实机调试中真金白银换来的，重写时容易再犯。

### 4.1 后台进程绝不能继承 pipe

`do_run` 用 pipe 收集子进程输出，请求处理完就关闭。**后台进程**（`cmd &`）
若继承这个 pipe，之后往 stdout/stderr 写就会收到 **SIGPIPE 直接死**：

实测 `sshd -ddd &` 起来 3 秒后 22 端口从 OPEN 变 refused，日志停在
`send_rexec_state: done`，排查了很久。

修法：后台分支把 fd 1/2 指向 `/dev/null`，并 `setsid()` + 关闭 3..63：

```c
if (strchr(cmd, '&') != NULL) {
    int nul = open("/dev/null", O_WRONLY);
    dup2(nul, 1); dup2(nul, 2);
    if (nul > 2) close(nul);
    setsid();
    for (k = 3; k < 64; k++) close(k);   /* 不继承监听 fd */
}
```

### 4.2 绝不 `waitpid(pid, NULL, 0)`

本 shell 对 `cmd &` 的行为不确定。如果它不退出，`waitpid` 永久阻塞，
而 devd 是**单线程串行**的 —— 一个请求卡住，后面所有 HTTP 请求全部排队超时
（表现为「端口 OPEN 但任何 GET/PUT 都超时」）。

改成前台才等、且最多轮询 3 秒：

```c
if (strchr(cmd, '&') == NULL) {
    int waited = 0;
    while (waited < 300) {
        if (waitpid(pid, NULL, WNOHANG) != 0) break;
        usleep(10000); waited++;
    }
}
```

### 4.3 `accept` 返回 EAGAIN 时必须短睡

监听 socket 非阻塞，无新连接时 `accept` 返回 `EAGAIN`。直接 `continue`
是纯忙等：`ps` 里两个 devd 分别占 23% / 9% CPU，`calls` 全是 `202`（accept）。

```c
int c = accept(...);
if (c < 0) { usleep(5000); continue; }
```

### 4.4 端口冲突要能自动递增

热更新时必须「先起新的、再停旧的」，否则杀掉旧的就没有通道起新的了。
所以 bind 失败要顺延端口，并且 **`SO_REUSEADDR` 会让 bind 假成功**，
到 `listen` 才报 EADDRINUSE —— 端口递增逻辑走不到。

```c
/* 不要 setsockopt(SO_REUSEADDR) */
for (port = PUSHD_PORT; port < PUSHD_PORT + 16; port++) {
    addr.sin_port = htons(port);
    if (bind(...) == 0) break;
}
```

### 4.5 socket 的 `read` 返回 EAGAIN 不代表 EOF

内核里 `lwip_recv` / unix socket 在「暂时没数据」时会返回 **0**，
而 musl 的 `read` 把 0 当 EOF —— 上层（PUT 的 body 循环、sshd 的 KEX）
会直接放弃。

内核侧已修（`socket_node_read` 把 0 转成 `EAGAIN`），devd 侧对应的写法是
**`EAGAIN` 短睡重试 + 总超时**，不要用 `poll` 等（`poll` 对 TCP 会谎报可读）：

```c
for (n = 0; n < IDLE_MAX; n++) {
    r = read(fd, buf, want);
    if (r > 0) break;
    if (r == 0) break;              /* 真 EOF */
    if (errno != EAGAIN) break;
    usleep(5000);
}
```

`IDLE_MAX = 12000`（60 秒）：devd 写 FAT 慢时 TCP 窗口会满、对端暂停，
超时太短（原来 3 秒）会导致 7.7MB 只写 163KB 就 `short`。

### 4.6 大缓冲必须 `static`

线程栈只有 4KB（`THREAD_STACK_SIZE`），16KB 的 `char buf[16384]` 放局部变量
会溢出。放静态区：

```c
static char buf[16384];     /* 不能是 char buf[16384] */
```

缓冲从 1KB 提到 16KB 后，推 7.7MB 的速度从 8KB/s 提升到 335KB/s。

### 4.7 FatFs 的 `FIL` 不能跨进程共享

两个进程同时 open 同一个文件，第二个的 `f_open` 会让第一个的 `FIL`
变成 `FR_INVALID_OBJECT`（`error code 9`）。

内核 `fatfs.c` 已加 `open_count` 引用计数（第二次 open 只加计数、
close 归零才真关），但**写日志时仍建议一个进程一个文件**。

---

## 5. 和 sshd 配合的用法

**不要**用 `devd` 的 `/run` 启动 `sshd -ddd &` —— 即使做了 4.1 的隔离，
实测仍会把 devd 一起带崩（`9005 CLOSED`，但内核没死、看门狗不介入）。

用 **sshd 自己的 daemon 模式**（不带 `-d`，它自己 fork 脱离）：

```sh
curl -X POST --data-binary "/bin/sshd -E /etc/sshd.log" http://<ip>:9004/run
nc -z <ip> 22 && echo "22 OPEN"
```

---

## 6. 已知限制

| 限制 | 说明 |
|---|---|
| 单连接串行 | 一个请求处理完才 accept 下一个；大文件传输期间其他请求会排队 |
| 无鉴权 | 局域网工具，不要暴露到公网 |
| 只支持 FAT 路径 | 路径直接传给 `vfs_open`，不经过 shell 展开（`~`、`*` 都无效） |
| `POST /run` 无 `&` 之外的 shell 特性 | 这个 `sh` 很精简，没有 `echo` 内建、不支持管道重定向组合 |

---

