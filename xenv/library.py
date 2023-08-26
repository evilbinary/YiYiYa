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


def use_library(env, libs=[], cpppath=[], cflags=[], linkflags=[]):
    """
    add use dependency lib in env
    """
    lib_list=[]
    for lib in libs:
        p=lib.split('/')
        lib_path= ''
        lib_name = p[-1].replace('.a','')
        lib_origin_name=p[-1]

        if lib_name[:3] != "lib":
            lib_name = 'lib'+lib_name
        if len(p)>1:
            lib_path= '/'.join(p[:-1])
            lib_path+='/'

        # if check_exist(env, lib_name):
        #     continue

        # print('---->', env, lib)
        # search path
        env.Append(CPPPATH=['#eggs/' +lib_path+ lib_name,
                            '#eggs/' +lib_path+ lib_name+'/include'
                            ])
        # search lib path
        env.Prepend(LIBPATH=['#eggs/'+lib_path + lib_name,
                            '#eggs/'+lib_path + lib_name+'/lib'
                            ])
        lib_list.append(lib_origin_name)
    
    # add to link
    env.PrependUnique(LIBS=lib_list)

    for cpath in cpppath:
        env.Append(CPPPATH=cpath)

    for cflag in cflags:
        env.Append(CFLAGS=' '+cflag)
    for linkflag in linkflags:
        env.Append(LINKFLAGS=' '+linkflag)

    # env.Append(LINKFLAGS=[])

    # print('env cpppath', env['CPPPATH'])
    # print('env libs   ', env['LIBS'])

    # print(env.Dump())


def add_include(env, include=[]):
    env.Append(CPPPATH=include)


def add_path(env, include=[]):
    env.Append(CPPPATH=include)


def add_libpath(env, libpath=[]):
    env.Append(LIBPATH=libpath)


def add_cflags(env, cflags=[]):
    if isinstance(cflags,str):
        cflags=[cflags]
    for cflag in cflags:
        env.Append(CFLAGS=cflag+' ')


def add_cxxflags(env, cxxflags=[]):
    if isinstance(cxxflags,str):
        cxxflags=[cxxflags]
    for cflag in cxxflags:
        env.Append(CXXFLAGS=' '+cflag.replace('#/','')+' ')


def check_exist(e, name):
    if not e.get('ALL_LIBS'):
        e['ALL_LIBS'] = {}
    if e.get('ALL_LIBS').get(name):
        return True
    e.get('ALL_LIBS')[name] = True
    return False


def use_libc(e):
    if check_exist(e, 'libc'):
        return
    arch = e['ARCH']
    if e.get('DEFAULT_LIBC') == 'libmusl':
        # print('>>>>>>',e)
        e.useLibGcc()

        e.UseLibrary(
            ['musl', 'm'],
            [
                '.',
                '#eggs/include/',
                '#eggs/libmusl',
                '#eggs/libmusl/include',
                '#eggs/libmusl/obj/include/',
                '#eggs/libmusl/arch/generic/',
                '#eggs/libmusl/arch/generic/bits'
            ],
            ['-DDUCK',
             ' -D__LIB_MUSL__ -Wl,-dynamic-linker,/lib/ld-musl-%s.so ' % (
                 arch)
             ],
            ['eggs/libmusl/lib/crt1.o ']
        )

        if e['ARCHTYPE'] == 'x86':
            e.AddInclude(
                [
                    '#eggs/libmusl/arch/i386/',
                    '#eggs/libmusl/arch/i386/bits']
            )
        elif e['ARCHTYPE'] == 'arm':
            e.AddInclude([
                '#eggs/libmusl/arch/arm/',
                '#eggs/libmusl/arch/arm/bits']
            )
        else:
            print('no support libmusl type %s' % (arch))

    elif e.get('DEFAULT_LIBC') == 'libnewlib':
        e.UseLibrary(['m', 'm', 'cygmon'],
                     [
            '#/eggs/libnewlib/lib/',
            '#/eggs/libnewlib',
            '#/eggs/include/c',
            '#/eggs/include/',

        ], ['-D__LIB_NEWLIB__ -D_LIBC  -static'])

        e['USER'] = '--entry main -Tapp/xlinker/cygmon.ld   '
        if e['ARCHTYPE'] == 'x86':
            e.AddInclude([
                '#/eggs/libnewlib/lib/i386-elf/include',
                '#/eggs/libnewlib/lib/i386-elf/lib',
            ])
        elif e['ARCHTYPE'] == 'arm':
            e.AddInclude([
                '#/eggs/libnewlib/lib/arm-eabi/include', ])
        else:
            print('no support libmusl type %s' % (e['ARCHTYPE']))
    else:
        e.UseLibrary(
            ['c'],
            [
                '#eggs/libc',
                '#/eggs/include/c',
                '#/eggs/include/',
                '.'
            ],
            [
                '-DLIBYC ',
            ],
            ['  eggs/libc/crt/crt.o ']
        )


def use_libcxx(e):
    # if check_exist(e, 'libcxx'):
    #     return
    arch = e['ARCH']
    e.UseLibC()
    e.UseLibrary(
        ['cxx', 'cxxabi'],
        [
            '#/eggs/libcxx',
            '#/eggs/libcxx/include',
            '#/eggs/libcxxabi',
            '#/eggs/libcxxabi/include'
        ],
        ['-DDUCK',
         ' -D__LIB_MUSL__ -Wl,-dynamic-linker,/lib/ld-musl-%s.so.1 ' % (
             arch)
         ],
        ['  ']
    )
    e.AddCxxFlags([' -g -fno-use-cxa-atexit -fno-threadsafe-statics',
                   ' -D_LIBCPP_HAS_NO_THREADS',
                   '-D_LIBCPP_HAS_NO_MONOTONIC_CLOCK',
                   '-D_LIBCPP_HAS_MUSL_LIBC',
                   '-D_LIBCPP_HAS_NO_LIBRARY_ALIGNED_ALLOCATION',
                   '-D_LIBCPP_BUILDING_LIBRARY',
                   '-D_POSIX_C_SOURCE -D_LIBCXXABI_HAS_NO_THREADS -D_GNU_SOURCE  ',
                   '-D_LIBCPP_AVAILABILITY_FILESYSTEM'
                   ])


def use_libgcc(env):
    # if check_exist(env, 'libgcc'):
    #     return
    if env.get('CC_LIB_PATH'):
        env.AddLibPath([env.get('CC_LIB_PATH')])
        env.UseLibrary(['gcc'])


def use_libapp(env):
    env.UseLibc()
    env.UseLibrary(['gui', 'jpeg', 'zlib', 'png',
                   'etk', 'cmocka', 'lz4', 'uuid'])

    app_lib=[
        '#/eggs/',
        '.',
        '../libs/include/',
        '../include/',
        '#/duck/libs/include',
        '#/eggs/libgui',
        '#/eggs/libjpeg',
        '#/eggs/libzlib',
        '#/eggs/libpng',
        '#/eggs/libetk',
        '#/eggs/libcmocka/include',
        '#/eggs/libgui',
        '#/eggs/libimage',
        '#/eggs/libjpeg',
        '#/eggs/libpng',
        '#/eggs/libzlib',
        '#/eggs/libetk',
        '#/eggs/libcmocka',
        '#/eggs/liblz4',
        '#/eggs/libuuid',
    ]
    env.AddPath(app_lib)

def generate(env, **kwargs):
    env.AddMethod(add_include, "AddInclude")
    env.AddMethod(use_library, "UseLibrary")
    env.AddMethod(use_library, "UseLib")
    env.AddMethod(use_libc, "UseLibC")
    env.AddMethod(use_libc, "UseLibc")
    env.AddMethod(use_libcxx, "UseLibcxx")
    env.AddMethod(use_libcxx, "UseLibCxx")
    env.AddMethod(add_cxxflags, "AddCxxFlags")
    env.AddMethod(add_cflags, "AddCflags")
    env.AddMethod(add_cflags, "AddCFlags")
    
    env.AddMethod(add_path, "AddPath")
    env.AddMethod(use_libapp, "UseLibApp")
    env.AddMethod(use_libgcc, "useLibGcc")
    env.AddMethod(add_libpath, "AddLibPath")
