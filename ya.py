# coding:utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import sys
module_path=['..',
    '.',
    '../../',
    '../../..',
    'xenv',
    '/Users/evil/dev/c/YiYiYa/xenv/',
    '/Users/evil/dev/c/ymake/']

sys.path.extend(module_path)

from ymake.yaya import *
import xenv.support as support

project("YiYiYa",
    version='1.3.0',
    desc='YiYiYa is an os , project page: https://github.com/evilbinary/YiYiYa',
    targets=[
        'kernel'
    ]
)

includes(
    "xenv/env.py"
)

plat="raspi2"

set_defaultplat(plat)
set_toolchain('arm-none-eabi')


arch=support.get_arch(plat)
arch_type=support.get_arch_type(arch)


set_arch(arch)
set_arch_type(arch_type)


#构建应用
if is_plat("raspi2","raspi3"):
    set_toolchains("arm-none-eabi")
else:
    set_toolchains("arm-none-eabi")

#默认libc
default_libc = 'musl'  # musl c newlib

#应用
apps = [              
    'cmd','gui','hello', 'test','unitest',
    'microui', 'etk',  'lvgl', 'track',
    'sdl2', 'infones', 'launcher', 'mgba', 
    'lua', 'scheme', 'quickjs', 'gnuboy',
    'watch',
    #'sdlmine',
    'sdl',
]

# cpp应用
cpp_apps=[
    'xtrack','gmenu2x',
    'monogui','testcpp'
]

wayland_apps={
    'weston'
}

#构建的module
modules=[
    'posix','loader','musl','ipc','gaga','log','pty' 
    #'musl','gaga','ipc','pty','log','debug','ewok'
]

set_config('modules',modules)
set_config('default_libc',default_libc)

# #c++ 应用 输入才编译
option("cpp-apps")
set_default(false)
set_showmenu(true)
set_description("Enable cpp support")
option_end()

#wayland 应用 输入才编译
option("wayland")
set_default(false)
set_showmenu(true)
set_description("Enable wayland support")
option_end()

option("app")
set_default(true)
set_showmenu(true)
set_description("Enable app build")
option_end()

option("tests")
set_default(false)
set_showmenu(true)
set_description("Enable tests build")
option_end()

arch=get_arch()


add_defines(plat.upper())


arch=get_arch()
def_arch=arch.replace("-", "_").upper()

arch_type=get_arch_type()
def_arch_type=arch_type.replace( "-", "_").upper()

add_defines(def_arch)
add_defines(def_arch_type)



arch_cflags=support.get_arch_cflags(arch)
plat_cflags=support.get_platform_cflags(plat)


add_cflags(arch_cflags)
add_cflags(plat_cflags)

add_cxxflags(arch_cflags)

set_config('cflags',arch_cflags,plat_cflags)
set_config('cpp_apps',cpp_apps)
set_config('apps',apps)


#apply debug and release modes
add_rules("mode.debug", "mode.release","arch")

includes('./boot/ya.py')
includes('./duck/ya.py')
add_subs('./image/ya.py')
add_subs('./eggs/ya.py')
add_subs('./app/ya.py')

process()