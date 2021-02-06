# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os

Import('env')



env.Program('kernel.elf', [
    'main.c',
    'screen.c',
    '../drivers/libdriver.a',
    '../kernel/libyiyiya.a',
    '../libs/libkernel/libkernel.a',
    '../arch/libarch.a',
    ]
    ,LINKFLAGS= '$LINKFLAGS -Tlink.ld')

env.Objcopy('kernel','kernel.elf',OBJCOPYFLAGS='-S')
env.Objcopy('kernel.dbg','kernel.elf',OBJCOPYFLAGS='--only-keep-debug')
env.Objcopy('kernel.bin','kernel',OBJCOPYFLAGS='-O binary')


