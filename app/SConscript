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
    '../arch/libarch.a',
    '../libs/libalgorithm/libalgorithm.a',
    '../libs/libkernel/libkernel.a',
    ]
    ,LINKFLAGS= '$LINKFLAGS -Tlink.ld')

env.Objcopy('kernel','kernel.elf',OBJCOPYFLAGS='-S')
env.Objcopy('kernel.dbg','kernel.elf',OBJCOPYFLAGS='--only-keep-debug')
# env.Objcopy('kernel.bin','kernel',OBJCOPYFLAGS='-O binary')


SConscript(dirs=['libjpeg'], exports='env')
SConscript(dirs=['libgui'], exports='env')
SConscript(dirs=['libetk'], exports='env')
SConscript(dirs=['libzlib'], exports='env')
SConscript(dirs=['libpng'], exports='env')


SConscript(dirs=['hello'], exports='env')
SConscript(dirs=['gui'], exports='env')
SConscript(dirs=['microui'], exports='env')
SConscript(dirs=['jpeg'], exports='env')
SConscript(dirs=['file'], exports='env')
SConscript(dirs=['etk'], exports='env')
SConscript(dirs=['test'], exports='env')


if plt=='Darwin':
    env.Command('copyhello', 
        ['hello/hello.elf','gui/gui.elf','microui/microui.elf','file/file.elf','etk/etk.elf','test/test.elf' ],
        ['hdid  image/disk.img &&  cp ${SOURCES} /Volumes/NO\ NAME/ && hdiutil eject /Volumes/NO\ NAME/'
    ])
    pass


