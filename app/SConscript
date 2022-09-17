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


env['CPPPATH']+= [
    '../../eggs/include',
    '../../eggs/include/c',
    '../../eggs/libc',
    '../../eggs/libgui',
    '../../eggs/libjpeg',
    '../../eggs/libzlib',
    '../../eggs/libpng',
    '../../eggs/libetk',
    '../../eggs/libcmocka/include',
    '../../eggs/liblvgl',
    '../../eggs/libsdl2/include',

    '../../eggs/',
    '../include/c',
    '.',
    '../libs/include/',
    '../include/c',
    '../include/',
    '../../duck/libs/include'
]

env['LIBPATH']+=[
    '../../eggs/',
    '../../eggs/libc/',
    '../../eggs/libgui',
    '../../eggs/libimage',
    '../../eggs/libjpeg',
    '../../eggs/libpng',
    '../../eggs/libzlib',
    '../../eggs/libetk',
    '../../eggs/libcmocka',
    '../../eggs/liblz4',
    '../../eggs/libuuid',
    '../../eggs/liblvgl',
    '../../eggs/liblvqrcode',

]

env['LIBS']+=env['LIBC']

if env.get('MYLIB'):
    env['LIBS'].append(env.get('MYLIB'))

def check_exit(apps):
    new_list = copy.deepcopy(apps)
    for app in new_list:
        if os.path.exists(app)==False:
            print('ignore app '+app)
            apps.remove(app)

if env.get('APP'):

    SConscript(dirs=['hello'], exports='env')
    SConscript(dirs=['gui'], exports='env')
    SConscript(dirs=['microui'], exports='env')
    SConscript(dirs=['test'], exports='env')
    SConscript(dirs=['etk'], exports='env')
    SConscript(dirs=['cmd'], exports='env')
    SConscript(dirs=['lvgl'], exports='env')
    SConscript(dirs=['track'], exports='env')
    SConscript(dirs=['sdl2'], exports='env')
    SConscript(dirs=['infones'], exports='env')
    SConscript(dirs=['launcher'], exports='env')
    SConscript(dirs=['mgba'], exports='env')
    # SConscript(dirs=['lua'], exports='env')
    # SConscript(dirs=['scheme'], exports='env')

    SConscript(dirs=['quickjs'], exports='env')

    # SConscript(dirs=['gnuboy'], exports='env')


    apps=['hello/hello',
            'gui/gui',
            'microui/microui',
            'etk/etk',
            'test/test',
            'test/test-file',
            'test/test-mem',
            'test/test-uncompress',
            'test/test-string',
            'test/test-stdlib',
            'test/test-stdio',

            #'rust/test/test-rs',
            'cmd/ls',
            'cmd/echo',
            'cmd/cat',
            'cmd/hexdump',
            'cmd/touch',
            'cmd/date',

            'lvgl/lvgl',
            'track/track',
            'launcher/launcher',
            'infones/infones',
            'sdl2/sdl2',
            'sdl2/player',
            'sdl2/showimage',
            'sdl2/showfont',
            'mgba/mgba',
            'mgba/miniunz',
            # 'gnuboy/gnuboy',

            # 'lua/lua',
            # 'lua/luat',
            # 'lua/hello.lua',
            # 'scheme/scheme',
            # 'scheme/petite.boot',
            # 'scheme/scheme.boot',

            'quickjs/qjs',
            'quickjs/qjsc',
            'quickjs/repl.js',
            'quickjs/qjscalc.js',
            # 'quickjs/tests/test_builtin.js',
            'quickjs/examples/hello.js',
            
            ]
    apps+=Glob('resource/*')
    if env.get('DEFAULT_LIBC')=='libmusl':
        # SConscript(dirs=['libncurses'], exports='env')

        # SConscript(dirs=['toybox'], exports='env')

        apps+=[
            'test/test-musl',
            # 'toybox/toybox',
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


