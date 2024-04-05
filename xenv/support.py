support_archs = {
    'arm': ['armv5', 'armv6', 'armv7', 'armv7e-m', 'armv7-a', 'armv8-a'],
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
    'armv8-a': ['-mcpu=cortex-a53', '-mtune=cortex-a53','-mfpu=vfpv4', '-mfloat-abi=softfp'],
    'dummy':[],
    'riscv':['-fstack-protector',
            '-nostdlib', '-nostdinc',
            '-fno-builtin', '-mcmodel=medany',  
            '-march=rv32imaczicsr','-mabi=ilp32','-DSBI' ],#
    'general':[]
}


support_arch_linkflags ={
    'x86': [],
    # 'armv5':['-mcpu=arm926ej-s','-mfloat-abi=soft','-mfpu=vfp'],
    'armv5': [],
    'armv7': [],
    'armv7-a': [],
    'armv8-a': [],
    'dummy': [],
    'riscv':['-melf32lriscv','-nostdlib',], #'-m elf32lriscv' '-march=rv32imaczicsr', '-mabi=ilp32','-mcmodel=medany'
    'general':['-lSystem', '-e _main']
}

support_platform_cflags ={
    'raspi2': ['-nostdlib','-nostdinc'],# #specs=nosys.specs
    'raspi3': ['-mcpu=cortex-a53','-mtune=cortex-a53'],
    'stm32f4xx': ['-specs=nosys.specs',  '-nostdlib', '-nostdinc' ,'-fno-builtin', '-DUSE_HAL_DRIVER','-DSTM32F401xC'],#'-nolibc',
    'android' : [],
    'v3s':['-nostdlib','-nostdinc'],
    'orangepi-pc':['-nostdlib','-nostdinc'],
    'rk3128': ['-nostdlib','-nostdinc'],
    'i386-pc': ['-nostdlib','-nostdinc'],
    'cubieboard2': ['-nostdlib','-nostdinc'],
    'dmulator':['-Wno-error=implicit-function-declaration'],
    'dummy':['-nostdlib','-nostdinc','-Wno-error,-Wimplicit-function-declaration'],
    'miyoo':['-nostdlib','-nostdinc'],
    'versatilepb': ['-nostdlib','-nostdinc'],
    'f1c100s': ['-nostdlib','-nostdinc'],
    'f1c200s': ['-nostdlib','-nostdinc'],
    't113-s3': ['-nostdlib','-nostdinc'],
    'riscv-virt':[],#-g -Wall
}


def get_arch_type(arch):
    archs = {}
    for k, v in support_archs.items():
        for i in v:
            archs[i] = k
    return archs.get(arch)


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

