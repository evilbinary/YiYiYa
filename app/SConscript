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
    
    SConscript(dirs=['test'], exports='env')

    SConscript(dirs=['file'], exports='env')
    
    SConscript(dirs=['etk'], exports='env')

    SConscript(dirs=['cmd'], exports='env')
    SConscript(dirs=['lvgl'], exports='env')

    SConscript(dirs=['track'], exports='env')

    SConscript(dirs=['liblvqrcode'], exports='env')

    SConscript(dirs=['launcher'], exports='env')


    SConscript(dirs=['liblz4'], exports='env')

    SConscript(dirs=['infones'], exports='env')


    apps=['hello/hello',
            'gui/gui',
            'microui/microui',
            'file/file',
            'etk/etk',
            'test/test',
            #'rust/test/test-rs',
            'cmd/ls',
            'cmd/echo',
            'lvgl/lvgl',
            'track/track',
            'launcher/launcher',
            'infones/infones'
            ]
    apps+=Glob('resource/*')
    if env.get('DEFAULT_LIBC')=='libmusl':
        SConscript(dirs=['libuuid'], exports='env')
        # SConscript(dirs=['libncurses'], exports='env')
        SConscript(dirs=['libchez'], exports='env')
        SConscript(dirs=['scheme'], exports='env')
        SConscript(dirs=['liblua'], exports='env')
        SConscript(dirs=['lua'], exports='env')

        apps+=[
            'scheme/scheme',
            'scheme/petite.boot',
            'scheme/scheme.boot',
            'test/test-musl',
            'lua/lua',
            'lua/luat',
            'lua/hello.lua',
            ]

    if plt=='Darwin':
        env.Command('copyapp', 
            apps,
            ['hdid  image/disk.img &&  cp ${SOURCES} /Volumes/NO\ NAME/ && hdiutil eject /Volumes/NO\ NAME/'
        ])
        pass
    elif plt=='Linux':
        env.Command('copyapp', 
            apps,
            ['sudo losetup /dev/loop10 image/disk.img && sudo mount /dev/loop10 /mnt && sudo cp  ${SOURCES} /mnt && sudo umount /mnt && sudo losetup -d /dev/loop10'
        ])
    elif plt=='Windows':
        try:
            ret=env.Command('copyapp', 
            apps,
            [
                'cp ${SOURCES} app/bin/ & mcopy.exe -nmov  -i image/disk.img app/bin/* ::'
            ])
        except:
            print('please manual copy %s files to image/disk.img'%(apps))
        pass
else:
    pass


