# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import platform 
import copy
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

if env.get('ARCHTYPE')=='xtensa':
    # libs.append('../app/libgui/libgui.a')
    # libs.append('../libs/libc/libc.a')
    pass

if env.get('MYLIB'):
    libs.append(env.get('MYLIB'))

def check_exit(apps):
    new_list = copy.deepcopy(apps)
    for app in new_list:
        if os.path.exists(app)==False:
            print('ignore app '+app)
            apps.remove(app)

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
            'rust/test/test-rs',
            # 'lua/lua',
            # 'lua/luat',
            'lua/hello.lua',
            # 'bin/ls',
            'lvgl/lvgl',
            'track/track.elf',
            'launcher/launcher'
            ]
    #check_exit(apps)

    if plt=='Darwin':
        env.Command('copyhello', 
            apps,
            ['hdid  image/disk.img &&  cp ${SOURCES} /Volumes/NO\ NAME/ && hdiutil eject /Volumes/NO\ NAME/'
        ])
        pass
    elif plt=='Linux':
        env.Command('copyhello', 
            apps,
            ['sudo losetup /dev/loop10 image/disk.img && sudo mount /dev/loop10 /mnt && sudo cp  ${SOURCES} /mnt && sudo umount /mnt && sudo losetup -d /dev/loop10'
        ])
    elif plt=='Windows':
        print('please manual copy %s files to image/disk.img'%(apps))
        # env.Command('copyhello', 
        #     apps,
        #     ['wsl.exe sudo losetup /dev/loop10 image/disk.img &&  mount /dev/loop10 /mnt &&  cp  ${SOURCES} /mnt &&  umount /mnt &&  losetup -d /dev/loop10'
        # ])
        pass
else:
    pass


