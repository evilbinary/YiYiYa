#!/usr/bin/env python3
"""LTP 逐测试墙钟超时 runner（替代被移除的内核 alarm 看门狗）。

用法: python3 /tmp/ltp_batch.py [--timeout 90] test1 test2 ...
- 每个测试串口下发 /bin/ltp <name>（设备端按子串匹配 wrapper）
- 墙钟超时或串口静默超时 => 判卡死，杀 QEMU 重启续跑下一个
- 结果追加 /tmp/ltp_results.txt，日志 /tmp/ltp_batch.log
"""
import socket, subprocess, sys, time, os

HOST, PORT = "127.0.0.1", 4448
KERNEL = "/Users/evil/dev/c/YiYiYa/build/raspi3/armv8-a/debug/kernel.elf"
RESULTS = "/tmp/ltp_results.txt"
LOG = "/tmp/ltp_batch.log"

QEMU_CMD = [
    "qemu-system-aarch64", "-name", "YiYiYa", "-M", "raspi3b",
    "-rtc", "base=localtime",
    "-kernel", KERNEL,
    "-serial", f"tcp:127.0.0.1:{PORT},server,nowait",
    "-drive", "if=sd,id=sd0,format=raw,file=/Users/evil/dev/c/YiYiYa/image/disk.img",
    "-display", "none",
]


def qemu_alive():
    r = subprocess.run(["pgrep", "-f", "qemu-system-aarch64"],
                       capture_output=True)
    return r.returncode == 0


def kill_qemu():
    subprocess.run(["pkill", "-9", "-f", "qemu-system-aarch64"],
                   capture_output=True)
    time.sleep(1.5)


STATE = "/tmp/ltp_state.txt"  # 记录当前 QEMU 启动时的 kernel.elf mtime


def qemu_kernel_stale():
    """QEMU 里跑的内核比磁盘上的 kernel.elf 旧（改过代码）⇒ 必须重启"""
    try:
        last = open(STATE).read().strip()
    except OSError:
        return True
    cur = str(int(os.path.getmtime(KERNEL)))
    return last != cur


def boot_wait(s):
    """等 shell 提示符（yiyiya$），替代固定 sleep"""
    deadline = time.time() + 30
    buf = bytearray()
    while time.time() < deadline:
        try:
            d = s.recv(4096)
            if d:
                buf.extend(d)
                if b"yiyiya$" in bytes(buf)[-64:]:
                    return True
        except socket.timeout:
            pass
        s.sendall(b"\n")
        time.sleep(1.0)
    return False


def start_qemu():
    kill_qemu()
    subprocess.Popen(QEMU_CMD, stdout=open("/tmp/qemu.out", "wb"),
                     stderr=subprocess.STDOUT, start_new_session=True)
    time.sleep(4)
    s = connect()
    boot_wait(s)
    with open(STATE, "w") as f:
        f.write(str(int(os.path.getmtime(KERNEL))))
    return s


def ensure_session():
    """QEMU 活着且内核没改 ⇒ 直接复用；否则重启一次"""
    if qemu_alive() and not qemu_kernel_stale():
        try:
            s = connect()
            boot_wait(s)
            print("reuse running qemu", flush=True)
            return s
        except OSError:
            pass
    return start_qemu()


def connect():
    s = socket.create_connection((HOST, PORT), timeout=5)
    s.settimeout(0.5)
    return s


def pump(s, logf, buf):
    try:
        d = s.recv(65536)
        if d:
            buf.extend(d)
            logf.write(d)
            logf.flush()
            return len(d)
    except socket.timeout:
        pass
    return 0


def run_one(s, name, timeout, logf):
    s.sendall(b"\n")
    time.sleep(1.0)
    pump(s, logf, bytearray())
    s.sendall(f"/bin/ltp {name}\n".encode())
    start = time.time()
    last_data = time.time()
    tail = bytearray()
    while time.time() - start < timeout:
        n = pump(s, logf, tail)
        if n:
            last_data = time.time()
        text = bytes(tail).decode("utf-8", "replace")
        if "Summary:" in text:
            end = time.time() + 2.5
            while time.time() < end:
                pump(s, logf, tail)
                time.sleep(0.2)
            text = bytes(tail).decode("utf-8", "replace")
            return classify(name, text, time.time() - start, "done")
        if time.time() - last_data > 60:
            return classify(name, bytes(tail).decode("utf-8", "replace"),
                            time.time() - start, "silent")
    return classify(name, bytes(tail).decode("utf-8", "replace"),
                    time.time() - start, "timeout")


def classify(name, text, elapsed, status):
    tp = text.count("TPASS")
    tf = text.count("TFAIL")
    tb = text.count("TBROK")
    tc = text.count("TCONF")
    return (name, status, elapsed, tp, tf, tb, tc)


def done_tests():
    """已测过的测试名（断点续跑用）。
    结果文件比 kernel.elf 旧 ⇒ 全部作废（旧内核的结果对新内核无效）"""
    try:
        if os.path.getmtime(KERNEL) > os.path.getmtime(RESULTS):
            return set()
    except OSError:
        return set()
    done = set()
    try:
        for line in open(RESULTS):
            parts = line.split("\t")
            if len(parts) >= 2 and parts[1] == "done":
                done.add(parts[0])
    except OSError:
        pass
    return done


def main():
    args = sys.argv[1:]
    timeout = 90.0
    force = False
    if args and args[0] == "--timeout":
        timeout = float(args[1])
        args = args[2:]
    if args and args[0] == "--force":
        force = True
        args = args[1:]
    skip = set() if force else done_tests()
    tests = [t for t in args if t not in skip]
    logf = open(LOG, "ab")
    rf = open(RESULTS, "a")
    if skip:
        print(f"skip already done: {sorted(skip)}", flush=True)
    logf.write(f"\n===== batch start {time.ctime()} tests={tests} =====\n".encode())

    if not tests:
        return
    s = ensure_session()

    for name in tests:
        try:
            res = run_one(s, name, timeout, logf)
        except (BrokenPipeError, ConnectionResetError, OSError):
            res = (name, "qemu-dead", 0, 0, 0, 0, 0)
        print(f"{res[0]:<28} {res[1]:<8} {res[2]:6.1f}s  "
              f"P={res[3]} F={res[4]} B={res[5]} C={res[6]}", flush=True)
        rf.write("\t".join(str(x) for x in res) + "\n")
        rf.flush()
        if res[1] in ("timeout", "silent", "qemu-dead"):
            s.close()
            s = start_qemu()

    s.close()
    logf.close()
    rf.close()


if __name__ == "__main__":
    main()
