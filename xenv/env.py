# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import platform
import sys
import datetime
from SCons.Environment import *
from SCons.Builder import *

from SCons.Script import *

base_path = os.getcwd()
sys.path.append(base_path)
sys.path.append(base_path+'/xenv')

plt = platform.system()

from xenv.config import *

import autoconfig,make,autogen,library,progress

support_archs = {
    'arm': ['armv5', 'armv6', 'armv7', 'armv7e-m', 'armv7-a', 'armv8-a'],
    'x86': ['x86', 'x86_64', 'x86-duck'],
    'xtensa':  ['lx6'],
    'riscv': ['riscv'],
    'dummy': ['dummy'],
    'general': ['general'],
}

support_platform = {
    'stm32f4xx': 'armv7e-m',
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
    'riscv-virt': 'riscv'
}

support_arch_cflags = {
    'x86': '',
    'armv7': '',
    'armv7e-m': '-mthumb -mthumb-interwork  -mfloat-abi=soft -mfpu=vfpv4-d16 -mcpu=cortex-m4 ',
    'armv7-a': '-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=softfp ',
    'armv8-a': ' -mfpu=vfpv4 -mfloat-abi=softfp ',
}


support_arch_linkflags = {
    'x86': '',
    'armv7': '',
    'armv7e-m': '-mthumb -mthumb-interwork  -mfloat-abi=soft -mfpu=vfpv4-d16 -mcpu=cortex-m4 ',
    'armv7-a': '-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=softfp ',
    'armv8-a':'-mcpu=cortex-a53 -mtune=cortex-a53',
}

support_platform_cflags = {
    'raspi2': '-nostdlib -nostdinc ',#--specs=nosys.specs
    'raspi3': '-mcpu=cortex-a53 -mtune=cortex-a53',
    'stm32f4xx': '-specs=nosys.specs -nolibc -nostdlib -nostdinc -fno-builtin -DUSE_HAL_DRIVER'

}


project_path = os.path.abspath(os.curdir)

libc = ['libc.a']
libcflags = ''


def get_arch(arch):
    archs = {}
    for k, v in support_archs.items():
        for i in v:
            archs[i] = k
    return archs.get(arch)


def macro_fmt(a):
    return a.upper().replace('-', '_')


if ARGUMENTS.get('cpp', 0):
    default_apps+=cpp_apps
else:
    cpp_apps=[]

if ARGUMENTS.get('platform'):
    platform=ARGUMENTS.get('platform')
    

arch = support_platform.get(platform)
if not arch:
    print('please select right platform')
    exit(-1)
    
arch_type = get_arch(arch)
archs = support_archs[arch_type]

common_cflags = '-DDUCK -D%s -D%s -nostdlib -nostdinc -fno-builtin -std=c99 -std=gnu99 -w -D%s' % (
    arch_type.upper(), macro_fmt(arch), macro_fmt(platform))

common_cxxflags = '-DDUCK -D%s -D%s  -w -D%s' % (
    arch_type.upper(), macro_fmt(arch), macro_fmt(platform))

lib_path = ['.']


print('welcome to yiyiya os build')
print('your select platform: %s arch: %s  support archs: %s build env:%s' %
      (platform, arch, archs, plt))

if plt == 'Linux':
    if len(CC_PATH) == 0:
        CC_PATH = '/usr/bin/'
elif plt == 'Darwin':
    if len(CC_PATH) == 0:
        CC_PATH = '/opt/local/bin/'
    if CC_PREFIX == None:
        CC_PREFIX = 'i386-elf-'
    pass
elif plt == 'Windows':
    pass
else:
    print('no support ^_^! '+plt)

if arch_type == 'arm':
    CC_PREFIX = 'arm-none-eabi-'
elif arch_type == 'xtensa':
    CC_PREFIX = 'xtensa-esp32-elf-'
    CC_PATH = '~/.espressif/tools/xtensa-esp32-elf/esp-2021r1-8.4.0/xtensa-esp32-elf/bin/'


if support_platform_cflags.get(platform)==None:
    support_platform_cflags[platform]=''

# VariantDir('build', 'src', duplicate=False)

CFLAGS +=' '+common_cflags
CFLAGS +=' '+support_arch_cflags.get(arch)
CFLAGS +=' '+support_platform_cflags.get(platform)
CFLAGS +=' '

env = Environment(
    ENV=os.environ,
    APP=default_apps,
    CPP_APP= cpp_apps,
    CC_PREFIX=CC_PREFIX,
    CC=CC,
    LD=LD,
    CXX=CXX,
    AR=AR,
    STRIP=STRIP,
    RANLIB=RANLIB,
    OBJCOPY=OBJCOPY,
    AS=AS,
    ASFLAGS=ASFLAGS,
    ARFLAGS=ARFLAGS,
    LDFLAGS=LDFLAGS,
    CFLAGS=CFLAGS,
    CXXFLAGS='%s %s ' % (CXXFLAGS, common_cxxflags,),
    #PATH= os.environ['PATH'],
    LIBPATH=lib_path,
    LINKFLAGS='%s %s'%(LINKFLAGS,support_arch_linkflags.get(arch) ),
    OBJCOPYFLAGS=OBJCOPYFLAGS,
    ARCH=arch,
    PLATFORM=platform,
    LINKLD=LINKLD,
    MYLIB=None,
    LIBC=libc,
    LIBCFLAGS=libcflags,
    USERLD=' -Tapp/xlinker/user.ld',
    ARCHS=archs,
    ARCHTYPE=arch_type,
    CPPPATH=[],
    LIBS=[],
    DEFAULT_LIBC=default_libc,
    CC_LIB_PATH=CC_LIB_PATH,
    MODULES=modules,
    SINGLE_KERNEL=SINGLE_KERNEL,
)

gcc_version = env.Detect(CC)
print("scons version: %s compiler: %s date: %s" %
      (SCons.__version__, gcc_version, datetime.datetime.now()))

autogen.generate(env)
autoconfig.generate(env)
make.generate(env)
library.generate(env)


if plt == 'Linux':
    if arch == 'x86':
        env['LINKFLAGS'] = env['LINKFLAGS']+' -m32'
    env['CFLAGS'] = env['CFLAGS']+' -no-pie -fno-pic '
    if CC_LIB_PATH:
        env['MYLIB'] = 'libgcc.a'
        env['CFLAGS'] = env['CFLAGS']+' -Llibgcc.a '
    pass
elif plt == 'Windows':
    if arch == 'x86':
        # -ffreestanding -nostdlib
        env['CFLAGS'] = env['CFLAGS'] + \
            ' -fno-stack-protector -mno-stack-arg-probe '
    env['USERLD'] = '--entry main -Tapp/xlinker/user.ld   '
    env['MYLIB'] = 'libgcc.a'
    env['PROGSUFFIX'] = ''
elif plt == 'Darwin':
    if CC_LIB_PATH:
        env['MYLIB'] = 'libgcc.a'

if env.get('SINGLE_KERNEL'):
    env['CFLAGS'] += ' -DSINGLE_KERNEL '

if arch_type == 'x86':
    if platform == 'x86_duck':
        env['CFLAGS'] += ' -m32 -march=native '
    else:
        env['CFLAGS'] += ' -march=i486 '
    pass
elif arch_type == 'arm':
    env['USERLD'] = ' -Tapp/xlinker/user-arm.ld '
    if platform == 'raspi2':
        pass
    elif platform == 'stm32f4xx':
        pass
    else:
        env['USERLD'] = ' -Tapp/xlinker/user-'+platform+'.ld'

elif arch_type == 'xtensa':
    env['APP'] = False
    env['LINKLD'] = 'link-'+platform+'.ld'
    pass

# program check

def check(env,tools=[]):
    conf = Configure(env)
    for tool in tools:
        if not conf.CheckProg(tool):
            print(tools+' is missing,please install')
            Exit(1)
    env = conf.Finish()

check(env,[
    'qemu-img',
    'mkfs.vfat',
    'dd',
    CC
])

progress.progress_settings(env, 5)

bootEnv = env.Clone()
appEnv = env.Clone()
cliEnv = env.Clone()
libEnv = env.Clone()

if env.get('MODULES'):
    for module in env.get('MODULES'):
        env.Append(CFLAGS=' -D'+module.upper()+'_MODULE ')

libEnv.UseLibc()
libEnv.UseLibApp()

cliEnv.UseLibc()
cliEnv.AddLinkFlags(cliEnv['USERLD'])

appEnv.UseLibApp()
appEnv.AddLinkFlags(appEnv['USERLD'])


cppEnv = env.Clone()
cppEnv.UseLibCxx()

cppCliEnv=cppEnv.Clone()
cppCliEnv.AddLinkFlags(cppCliEnv['USERLD'])


Export('cppEnv')
Export('bootEnv')
Export('appEnv')
Export('libEnv')
Export('cliEnv')
Export('cppCliEnv')
Export('env')
