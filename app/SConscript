# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import platform 
plt = platform.system()

Import('env')



env.Program('kernel.elf', [
    'main.c',
    'shell.c',
    'serial.c',
    '../drivers/libdriver.a',
    '../kernel/libyiyiya.a',
    '../libs/libkernel/libkernel.a',
    '../arch/libarch.a',
    ]
    ,LINKFLAGS= '$LINKFLAGS -Tlink.ld')

env.Objcopy('kernel','kernel.elf',OBJCOPYFLAGS='-S')
env.Objcopy('kernel.dbg','kernel.elf',OBJCOPYFLAGS='--only-keep-debug')
env.Objcopy('kernel.bin','kernel',OBJCOPYFLAGS='-O binary')

SConscript(dirs=['hello'], exports='env')
SConscript(dirs=['gui'], exports='env')
SConscript(dirs=['microui'], exports='env')



if plt=='Darwin':
    env.Command('copyhello', 
        ['hello/hello.elf' ],
        ['hdid  image/disk.img &&  cp ${SOURCES[0]} /Volumes/NO\ NAME/ && hdiutil eject /Volumes/NO\ NAME/'
    ])
    env.Command('copygui', 
        ['gui/gui.elf' ],
        ['hdid  image/disk.img &&  cp ${SOURCES[0]} /Volumes/NO\ NAME/ && hdiutil eject /Volumes/NO\ NAME/'
    ])
    #if env.get('w')=='hello':
    env.Command('copymicroui', 
        ['microui/microui.elf' ],
        ['hdid  image/disk.img &&  cp ${SOURCES[0]} /Volumes/NO\ NAME/ && hdiutil eject /Volumes/NO\ NAME/'
    ])
    pass


