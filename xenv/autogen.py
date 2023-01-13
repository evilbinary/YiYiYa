# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import subprocess
import math

from SCons.Script import *

def generate_bin(source, target, env, for_signature):
    return '$OBJCOPY %s %s %s'%(env['OBJCOPYFLAGS'],source[0], target[0])
    
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




def generate(env, **kwargs):
   env.Append(BUILDERS={
    'GenerateKernelSize': Builder(
               action=generate_kernel_size
               ),
    'KernelSize': get_kernel_size
               })
   env.Append(BUILDERS={
    'Objcopy': Builder(
               generator=generate_bin,
               suffix='',)})
