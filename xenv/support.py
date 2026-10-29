import sys

support_archs = {
    'arm': ['armv5', 'armv6', 'armv7', 'armv7e-m', 'armv7-a'],
    'arm64': ['armv8-a'],
    'x86': ['x86', 'x86_64', 'x86-duck'],
    'xtensa':  ['lx6'],
    'riscv': ['riscv'],
    'dummy': ['dummy'],
    'general': ['general'],
}

support_platform = {
    'stm32f4xx': 'armv7',
    'stm32f401': 'armv7',
    'i386-pc': 'x86',
    'raspi2': 'armv7-a',
    'esp32': 'lx6',
    'v3s': 'armv7-a',
    'raspi3': 'armv8-a',
    'raspi5': 'armv8-a',
    'rk3128': 'armv7-a',
    'cubieboard2': 'armv7-a',
    'dmulator': 'general',
    'dummy': 'dummy',
    'rk3288': 'armv7-a',
    'orangepi-pc': 'armv7-a',
    'riscv-virt': 'riscv',
    'miyoo': 'armv7-a',
    'versatilepb': 'armv5',
    'f1c100s': 'armv5',
    't113-s3': 'armv7-a',
    'f1c200s': 'armv5',
}

support_arch_cflags ={
    'x86': [],
    'armv5':['-march=armv5te','-mfloat-abi=soft'], #, '-mthumb-interwork', '-mcpu=arm926ej-s','-mtune=arm926ej-s','-marm',
    'armv7': ['-mthumb', '-mthumb-interwork',  '-mfloat-abi=soft', '-mfpu=vfpv4-d16', '-mcpu=cortex-m4',],
    'armv7-a': ['-mcpu=cortex-a7',
                '-mtune=cortex-a7',
                '-mfpu=vfpv4',
                '-mfloat-abi=softfp'
            ],
    'armv8-a': ['-mcpu=cortex-a53', '-mtune=cortex-a53',
            # linux-gnu 交叉编译器默认 PIE/-fPIE + outline atomics：
            # -fPIE 会为全局变量生成 GOT 间接寻址（内核链接脚本没有 .got 处理），
            # outline atomics 依赖 libatomic（裸机没有），都必须关掉。
            # 注意：这里不能再写 -fno-pie —— 架构级 cflags 在 ymake 里排在目标级
            # 之后，会覆盖目标自己加的 -fPIC（共享库/libweston 需要），导致
            # "recompile with -fPIC" 链接失败。所用的 aarch64-none-elf 交叉工具链
            # 默认就是非 PIE（--with-default-pie 只在 linux-gnu 目标里开），
            # 因此省掉这个开关对内核与静态程序没有影响。
            '-mno-outline-atomics',
            '-Wno-error=implicit-function-declaration','-Wno-error=incompatible-pointer-types','-Wno-error=int-conversion'],#'-mfpu=vfpv4', '-mfloat-abi=softfp'
    'dummy':[],
    'riscv':['-fstack-protector',
            '-nostdlib', '-nostdinc',
            '-fno-builtin', '-mcmodel=medany',  
            '-march=rv32imaczicsr','-mabi=ilp32','-DSBI' ],#
    'general':[],
    'lx6': ['-mlongcalls','-mtext-section-literals','-mabi=call0','-mlongcalls'],
}

general_link = []

if sys.platform=='linux':
    general_link=['-m32']

support_arch_linkflags ={
    'x86': [],
    # 'armv5':['-mcpu=arm926ej-s','-mfloat-abi=soft','-mfpu=vfp'],
    'armv5': [],
    'armv7': [],
    'armv7-a': [],
    'armv8-a': [],
    'dummy': [],
    'riscv':['-melf32lriscv','-nostdlib',], #'-m elf32lriscv' '-march=rv32imaczicsr', '-mabi=ilp32','-mcmodel=medany'
    'general': general_link,
    'lx6': [],
}

support_platform_cflags ={
    'raspi2': ['-nostdlib','-nostdinc','-mapcs-frame','-DBACKTRACE'],# #specs=nosys.specs
    'raspi3': ['-mcpu=cortex-a53','-mtune=cortex-a53'],
    'raspi5': ['-mcpu=cortex-a76','-mtune=cortex-a76'],
    'stm32f4xx': ['-specs=nosys.specs',  '-nostdlib', '-nostdinc' ,'-fno-builtin', '-DUSE_HAL_DRIVER','-DSTM32F401xC'],#'-nolibc',
    'android' : [],
    'v3s':['-nostdlib','-nostdinc'],
    'orangepi-pc':['-nostdlib','-nostdinc'],
    'rk3128': ['-nostdlib','-nostdinc'],
    'i386-pc': ['-nostdlib','-nostdinc'],
    'cubieboard2': ['-nostdlib','-nostdinc'],
    'dmulator':['-m32','-Wno-error=implicit-function-declaration'],
    'dummy':['-nostdlib','-nostdinc','-Wno-error,-Wimplicit-function-declaration'],
    'miyoo':['-nostdlib','-nostdinc'],
    'versatilepb': ['-nostdlib','-nostdinc'],
    'f1c100s': ['-nostdlib','-nostdinc'],
    'f1c200s': ['-nostdlib','-nostdinc'],
    't113-s3': ['-nostdlib','-nostdinc','-mapcs-frame'],
    'riscv-virt':[],#-g -Wall
    'esp32': ['-mlongcalls','-mtext-section-literals','-mabi=call0','-mlongcalls'  #禁用密度指令
    ]
}


def get_arch_type(arch):
    archs = {}
    for k, v in support_archs.items():
        for i in v:
            archs[i] = k
    return archs.get(arch)


# ---- musl 动态链接器（ldso）命名 ----
#
# 动态链接的可执行文件在 PT_INTERP 里写的是解释器的绝对路径，musl 装出来的
# 名字是 <syslibdir>/ld-musl-<ARCH><SUBARCH>.so.1，其中 ARCH 是 musl configure
# 时的架构名（不是本项目的 arch_type）。这里做单点映射，构建脚本（env.py 的
# set_type、eggs/libmusl/ya.py 的部署）都从这里取，保证跨平台一致：
#   armv8-a/armv7-a/armv5 → aarch64 / arm / arm
#   riscv                 → riscv64
#   x86                   → i386
support_musl_arch = {
    # get_arch_type() 的返回值（架构大类）
    'arm': 'arm',
    'arm64': 'aarch64',
    'riscv': 'riscv64',
    'x86': 'i386',
    'xtensa': 'xtensa',
    # 具体 arch 名（有些地方传的是 arch 而不是 arch_type）
    'armv5': 'arm',
    'armv6': 'arm',
    'armv7': 'arm',
    'armv7e-m': 'arm',
    'armv7-a': 'arm',
    'armv8-a': 'aarch64',
    'x86_64': 'x86_64',
    'x86-duck': 'i386',
}


def musl_ldso_arch(arch_type):
    '''项目 arch_type → musl 的 ARCH（未知架构按原名透传）'''
    return support_musl_arch.get(arch_type, arch_type)


def musl_ldso_name(arch_type):
    '''动态链接器文件名，例如 armv8-a → ld-musl-aarch64.so.1'''
    return 'ld-musl-%s.so.1' % musl_ldso_arch(arch_type)


def musl_ldso_path(arch_type):
    '''写进 ELF PT_INTERP 的解释器绝对路径（镜像里放在 /lib 下）'''
    return '/lib/' + musl_ldso_name(arch_type)


def get_arch(p):
    return support_platform.get(p)

def get_archs(arch_type):
    return support_archs.get(arch_type)


def get_arch_cflags(key):
    return support_arch_cflags.get(key)
 

def get_platform_cflags(plat):
    return support_platform_cflags.get(plat)
 

def get_arch_linkflags(key):
    return support_arch_linkflags.get(key)

