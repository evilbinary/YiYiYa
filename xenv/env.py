# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import platform 
import sys
from SCons.Environment import *
from SCons.Builder  import *

from SCons.Script import *

base_path= os.getcwd()
sys.path.append(base_path)
sys.path.append(base_path+'/xenv')

plt = platform.system()

from xenv.config import *

import autoconfig,make,autogen

supports_archs={
    'arm': ['armv5','armv6','armv7','armv7-a','armv8-a'],
    'x86': ['x86','x86_64','x86-duck'],
    'xtensa':  ['lx6'],
    'riscv':['riscv'],
    'dummy':['dummy'],
    'general': ['general'],
}

support_platform={
    'stm32f4xx':'armv7',
    'i386-pc':'x86',
    'raspi2':'armv7-a',
    'esp32':'lx6',
    'v3s':'armv7-a',
    'raspi3':'armv8-a',
    'rk3128':'armv7-a',
    'cubieboard2': 'armv7-a',
    'dmulator':'general',
    'dummy':'dummy',
    'rk3288':'armv7-a',
    'orangepi-pc':'armv7-a',
    'riscv-virt':'riscv'
}


project_path=os.path.abspath(os.curdir)

libc=['libc.a']
libcflags=''

def get_arch(arch):
    archs={}
    for k,v in supports_archs.items():
        for i in v:
            archs[i]=k
    return archs.get(arch)   

arch=support_platform[platform]
arch_type=get_arch(arch)
archs=supports_archs[arch_type]

print('welcome to yiyiya os build')
print('your select platform: %s arch: %s  support archs: %s build env:%s'%(platform,arch, archs, plt))

if plt=='Linux':
    if len(CC_PATH)==0:
        CC_PATH='/usr/bin/'
elif plt=='Darwin':
    if len(CC_PATH)==0:
        CC_PATH='/opt/local/bin/'
    if CC_PREFIX==None:
        CC_PREFIX='i386-elf-'
    pass
elif plt=='Windows':
    pass
else:
    print('no support ^_^! '+plt)

if arch_type == 'arm':
    CC_PREFIX='arm-none-eabi-'
elif arch_type=='xtensa':
    CC_PREFIX='xtensa-esp32-elf-'
    CC_PATH='~/.espressif/tools/xtensa-esp32-elf/esp-2021r1-8.4.0/xtensa-esp32-elf/bin/'


def macro_fmt(a):
    return a.upper().replace('-','_')

env = Environment(
        ENV=os.environ,
        APP=default_apps,
        CC_PREFIX=CC_PREFIX,
        CC = CC,
        LD= LD,
        CXX=CXX,
        AR= AR,
        STRIP=STRIP,
        RANLIB=RANLIB,
        OBJCOPY=OBJCOPY,
        AS= AS,
        ASFLAGS= ASFLAGS,
        ARFLAGS=ARFLAGS,
        LDFLAGS=LDFLAGS,
        CFLAGS='%s -DDUCK -D%s -D%s -I. -I./include -Ilibs/include -g -nostdlib -nostdinc  -fno-builtin  -c -std=c99 -std=gnu99 -w -D%s'%(CFLAGS,arch_type.upper(),macro_fmt(arch),macro_fmt(platform)),
        #PATH= os.environ['PATH'],
        LIBPATH=['.','../arch/','../modules','../libs/'],
        LINKFLAGS=LINKFLAGS,
        OBJCOPYFLAGS=OBJCOPYFLAGS,
        ARCH=arch,
        PLATFORM=platform,
        LINKLD=LINKLD,
        MYLIB=None,
        LIBC=libc,
        LIBCFLAGS=libcflags,
        USER=' -Tapp/xlinker/user.ld',
        ARCHS=archs,
        ARCHTYPE=arch_type,
        CPPPATH=[],
        LIBS=[],
        DEFAULT_LIBC=default_libc,
        CC_LIB_PATH=CC_LIB_PATH,
        MODULES=modules,
        SINGLE_KERNEL= SINGLE_KERNEL,
        )

autogen.generate(env)
autoconfig.generate(env)
make.generate(env)

if plt=='Linux':
    if arch =='x86':
        env['LINKFLAGS']= env['LINKFLAGS']+' -m32'
    env['CFLAGS']= env['CFLAGS']+' -no-pie -fno-pic '
    if CC_LIB_PATH:
        env['MYLIB']='libgcc.a'
        env['CFLAGS']=  env['CFLAGS']+' -Llibgcc.a '
    pass
elif plt=='Windows':
    if arch=='x86':
        env['CFLAGS']= env['CFLAGS']+' -fno-stack-protector -mno-stack-arg-probe ' #-ffreestanding -nostdlib  
    env['USER']='--entry main -Tapp/xlinker/user.ld   '
    env['MYLIB']='libgcc.a'
    env['PROGSUFFIX'] = ''
elif plt=='Darwin':
    if CC_LIB_PATH:
        env['MYLIB']='libgcc.a'

if env.get('DEFAULT_LIBC') == 'libmusl':
    env['CFLAGS']+=' -DLIBC_POSIX '

if env.get('SINGLE_KERNEL'):
    env['CFLAGS']+=' -DSINGLE_KERNEL '

if arch_type == 'x86':
    if platform=='x86_duck':
        env['CFLAGS']+=' -m32 -march=native '
    else:
        env['CFLAGS']+=' -march=i486 '
    pass
elif arch_type == 'arm':
    env['USER']=' -Tapp/xlinker/user-arm.ld '
    # -nostdlib -nostdinc -fno-builtin -DMALLOC_TRACE -mcpu=cortex-a7  -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=hard -mfloat-abi=softfp

    #env['CFLAGS']= env['CFLAGS']+ ' -fno-omit-fram e-pointer -mapcs -mno-sched-prolog ' #for debug backtrace
    if platform=='raspi2': 
        env['CFLAGS']= env['CFLAGS']+ ' -nostdlib -nostdinc --specs=nosys.specs -mcpu=cortex-a7  -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=softfp '
        env['LINKFLAGS'] += ' -mcpu=cortex-a7  -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=softfp '
        pass
    elif platform =='stm32f4xx':
        env['CFLAGS']= env['CFLAGS']+ ' -specs=nosys.specs -mcpu=cortex-m4 -nolibc -nostdlib -nostdinc -fno-builtin -DUSE_HAL_DRIVER   -mthumb -mthumb-interwork  -mfloat-abi=soft -mfpu=vfpv4-d16 '# -DSTM32F401xC  -Wl,--gc-sections -fdata-sections -ffunction-sections    -mfpu=fpv4-sp-d16  -mcpu=cortex-m4  -ffreestanding -nostdlib  -mfloat-abi=hard -mfpu=fpv4-sp-d16   # -mfloat-abi=hard -mfloat-abi=softfp  -mfloat-abi=softfp
        arch='armv7e-m'
    else:
        env['USER']=' -Tapp/xlinker/user-'+platform+'.ld'
        env['CFLAGS']= env['CFLAGS']+ ' -mcpu=cortex-a7  -mtune=cortex-a7 -mfloat-abi=softfp   -mfpu=vfpv4 '#-mfpu=vfpv4  -mfloat-abi=softfp
    #env['CFLAGS']= ' -march='+arch
        env['LINKFLAGS'] += ' -mcpu=cortex-a7  -mtune=cortex-a7 -mfloat-abi=softfp   -mfpu=vfpv4'

elif arch_type=='xtensa':
    env['APP']=False
    env['LINKLD']='link-'+platform+'.ld'
    pass


if env.get('MODULES'):
    for module in env.get('MODULES'):
        env['CFLAGS'] = env['CFLAGS']+' -D'+module.upper()+'_MODULE '


def add_libc(e):
    if e.get('HAS_LIBC'):
        return
    e['HAS_LIBC']=True
    if e.get('DEFAULT_LIBC') == 'libmusl':
        e['CFLAGS'] += ' -D__LIB_MUSL__  -static '
        e['LIBPATH'] += ['../../eggs/libmusl/lib/']
        e['CPPPATH'] += [
            '#/eggs/libmusl',
            '#/eggs/libmusl/include',
            '#/eggs/libmusl/obj/include/',
            '#/eggs/libmusl/arch/generic/',
            '#/eggs/ibmusl/arch/generic/bits'
        ]
        e['LIBC'] = ['libm.a','libmusl.a']
        e['LINKFLAGS']+='  eggs/libmusl/lib/crt1.o '

        if e['ARCHTYPE'] == 'x86':
            e['CPPPATH'] += [
                '#/eggs/libmusl/arch/i386/',
                '#/eggs/libmusl/arch/i386/bits']
        elif e['ARCHTYPE'] == 'arm':
            e['CPPPATH'] += [
                '#/eggs/libmusl/arch/arm/',
                '#/eggs/libmusl/arch/arm/bits']
        else:
            print('no support libmusl type %s' % (arch))
    elif e.get('DEFAULT_LIBC') == 'libnewlib':
        e['CFLAGS'] += ' -D__LIB_NEWLIB__ -D_LIBC  -static '
        e['LIBPATH'] += ['#/eggs/libnewlib/lib/']
        e['USER']='--entry main -Tapp/xlinker/cygmon.ld   '
        e['CPPPATH'] += [
            '#/eggs/libnewlib',
            '#/eggs/include/c',
            '#/eggs/include/',
        ]
        e['LIBC'] = ['libm.a', 'libc.a','libcygmon.a']

        if e['ARCHTYPE'] == 'x86':
            e['CPPPATH'] += [
                '#/eggs/libnewlib/lib/i386-elf/include',
                '#/eggs/libnewlib/lib/i386-elf/lib',
            ]
            e['LIBPATH'] += [ '#/eggs/libnewlib/lib/i386-elf/lib',]
        elif e['ARCHTYPE'] == 'arm':
            e['CPPPATH'] += [
                '#/eggs/libnewlib/lib/arm-eabi/include',]
        else:
            print('no support libmusl type %s' % (e['ARCHTYPE']))
    else:
        e['LIBPATH'] += ['../../eggs/libc/']
        e['CPPPATH'] += [
            '#/eggs/include/c',
            '#/eggs/include/',
            '.'
        ]
        e['CFLAGS'] += '  -DLIBYC '
        e['LINKFLAGS']+='  eggs/libc/crt/crt.o '
        e['LIBC'] = ['libc.a']


bootEnv = env.Clone()
appEnv = env.Clone()

Export('bootEnv')
Export('appEnv')
Export('env')