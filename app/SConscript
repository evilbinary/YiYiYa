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

linkflags=' $LINKFLAGS -T'+env.get('LINKLD')

libs=[
     '../drivers/libdriver.a',
    '../kernel/libyiyiya.a',
    '../arch/libarch.a',
    '../platform/libplatform.a',
    '../libs/libalgorithm/libalgorithm.a',
    '../libs/libkernel/libkernel.a'
]

if env.get('ARCH')=='xtensa':
    # libs.append('../app/libgui/libgui.a')
    # libs.append('../libs/libc/libc.a')
    pass

if env.get('MYLIB'):
    libs.append(env.get('MYLIB'))

if env.get('APP'):
    SConscript(dirs=['libjpeg'], exports='env')
    SConscript(dirs=['libgui'], exports='env')
    SConscript(dirs=['libetk'], exports='env')
    SConscript(dirs=['libzlib'], exports='env')
    SConscript(dirs=['libpng'], exports='env')
    SConscript(dirs=['liblvgl'], exports='env')

    SConscript(dirs=['hello'], exports='env')
    SConscript(dirs=['gui'], exports='env')
    SConscript(dirs=['microui'], exports='env')
    #SConscript(dirs=['jpeg'], exports='env')


    SConscript(dirs=['file'], exports='env')
    
    SConscript(dirs=['etk'], exports='env')

    SConscript(dirs=['test'], exports='env')
    SConscript(dirs=['bin'], exports='env')
    SConscript(dirs=['lvgl'], exports='env')

    SConscript(dirs=['track'], exports='env')

    SConscript(dirs=['liblvqrcode'], exports='env')

    SConscript(dirs=['launcher'], exports='env')

    # SConscript(dirs=['liblua'], exports='env')
    # SConscript(dirs=['lua'], exports='env')

    

    apps=['hello/hello',
            'gui/gui.elf',
            'microui/microui.elf',
            'file/file.elf',
            'etk/etk.elf',
            'test/test.elf',
            # 'test/test-musl.elf',
            'bin/ls',
            'lvgl/lvgl',
            'track/track.elf',
            'launcher/launcher'
            ]
    if plt=='Darwin':
        env.Command('copyhello', 
            apps,
            ['hdid  image/disk.img &&  cp ${SOURCES} /Volumes/NO\ NAME/ && hdiutil eject /Volumes/NO\ NAME/'
        ])
        pass
    elif plt=='Linux':
        env.Command('copyhello', 
            apps,
            ['sudo losetup /dev/loop10 image/disk.img && sudo mount /dev/loop10 /mnt && sudo cp ${SOURCES} /mnt && sudo umount /mnt && sudo losetup -d /dev/loop10'
        ])
else:
    pass


