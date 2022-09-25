# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import math
import platform 
import sys
from SCons.Environment import *
from SCons.Builder  import *

from SCons.Script import *

plt = platform.system()

from xenv.config import *


supports_archs={
    'arm': ['armv5','armv6','armv7','armv7-a','armv8-a'],
    'x86': ['x86','x86_64','x86-duck'],
    'xtensa':  ['lx6'],
    'riscv':['riscv'],
    'dummy':['dummy']
}

support_platform={
    'stm32f4xx':'armv7',
    'pc':'x86',
    'raspi2':'armv7-a',
    'esp32':'lx6',
    'v3s':'armv7-a',
    'raspi3':'armv8-a',
    'rk3128':'armv7-a',
    'cubieboard2': 'armv7-a',
    'dmulator':'x86-duck',
    'dummy':'dummy',
    'rk3288':'armv7-a',
    'orangepi-pc':'armv7-a'
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
        ARFLAGS=ARFLAGS,
        LDFLAGS=LDFLAGS,
        CFLAGS='%s -DDUCK -D%s -D%s -I. -I./include -Ilibs/include -g -nostdlib -nostdinc  -fno-builtin  -c -std=c99 -std=gnu99 -w -D%s'%(CFLAGS,arch_type.upper(),arch.upper().replace('-','_'),platform.upper()),
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
        CC_LIB_PATH=CC_LIB_PATH
        )
if plt=='Linux':
    if arch =='x86':
        env['LINKFLAGS']= env['LINKFLAGS']+' -m32'
    env['CFLAGS']= env['CFLAGS']+' -no-pie -fno-pic -Llibgcc.a '
    if CC_LIB_PATH!=None:
        env['MYLIB']='libgcc.a'
    pass
elif plt=='Windows':
    env['LINKFLAGS']='-nostdlib  ',
    env['CFLAGS']= env['CFLAGS']+' -fno-stack-protector -mno-stack-arg-probe ' #-ffreestanding -nostdlib  
    env['USER']='--entry main -Tapp/xlinker/user.ld   '
    env['MYLIB']='libgcc.a'
    env['PROGSUFFIX'] = ''
elif plt=='Darwin':
    env['MYLIB']='libgcc.a'

if env.get('DEFAULT_LIBC') == 'libmusl':
    env['CFLAGS']+=' -DLIBC_POSIX '

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
        env['CFLAGS']= env['CFLAGS']+ ' -nostdlib -nostdinc --specs=nosys.specs -mcpu=cortex-a7  -mtune=cortex-a7 -mfpu=vfpv4  -mfloat-abi=softfp '
        pass
    elif platform =='stm32f4xx':
        env['APP']=False
        env['CFLAGS']= env['CFLAGS']+ ' -specs=nosys.specs -mcpu=cortex-m4 -nolibc -nostdlib -nostdinc -fno-builtin -DUSE_HAL_DRIVER   -mthumb -mthumb-interwork  -mfloat-abi=soft -mfpu=vfpv4-d16 -DSTM32F401xC'# -DSTM32F401xC  -Wl,--gc-sections -fdata-sections -ffunction-sections    -mfpu=fpv4-sp-d16  -mcpu=cortex-m4  -ffreestanding -nostdlib  -mfloat-abi=hard -mfpu=fpv4-sp-d16   # -mfloat-abi=hard -mfloat-abi=softfp  -mfloat-abi=softfp
        env['LINKLD']='xlinker/link-'+platform+'.ld'
        arch='armv7e-m'
    else:
        env['USER']=' -Tapp/xlinker/user-'+platform+'.ld'
        env['CFLAGS']= env['CFLAGS']+ ' -mcpu=cortex-a7  -mtune=cortex-a7 -mfpu=vfpv4  -mfloat-abi=softfp '
        env['LINKLD']='xlinker/link-'+platform+'.ld'
    #env['CFLAGS']= ' -march='+arch

elif arch_type=='xtensa':
    env['APP']=False
    env['LINKLD']='xlinker/link-'+platform+'.ld'
    pass


def generate_bin(source, target, env, for_signature):
    return '$OBJCOPY %s %s %s'%(env['OBJCOPYFLAGS'],source[0], target[0])
    
env.Append(BUILDERS={
    'Objcopy': Builder(
               generator=generate_bin,
               suffix='',)})

def get_kernel_size(env,target,source):
    if source:
        file_name=str(source[0])
        file_out=str(target[0])
        if os.path.exists(file_name):
            size=os.stat(file_name).st_size
            env['KERNEL_SIZE'] = str(size)
            env['KERNEL_BLOCK_SIZE'] = str(int(math.ceil(size/1024.0)))
            print('kernel size:',env.get('KERNEL_SIZE'))
        else :
            print('file %s not exist'%(file_name))
    else:
      env['KERNEL_SIZE'] = str(1024)
      env['KERNEL_BLOCK_SIZE'] = str(int(math.ceil(1024/1024.0)))
    return ''

def generate_kernel_size(env,target,source):
    file_out=str(target[0])
    get_kernel_size(env,target,source)
    kernel_header=file_out.upper().rsplit('/',1)[-1].replace('.','_')
    kernel_header=file_out.upper().rsplit('\\',1)[-1].replace('.','_').replace('/','_')
    f = open(file_out, 'w')
    content="""#ifndef %s
#define %s
#define KERNEL_BLOCK_SIZE %s
#define KERNEL_SIZE %s
#endif
"""%(kernel_header,
    kernel_header,
    env.get('KERNEL_BLOCK_SIZE'),
    env.get('KERNEL_SIZE'))
    f.write(content)
    f.close()
    return ''

env.Append(BUILDERS={
    'GenerateKernelSize': Builder(
               action=generate_kernel_size
               ),
    'KernelSize': get_kernel_size
               })


Export('env')