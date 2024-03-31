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
   ]
sys.path.extend(module_path)
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

plat=get_plat()
if not plat:
    plat="raspi2"

set_defaultplat(plat)
set_toolchain('arm-none-eabi')


arch=support.get_arch(plat)
arch_type=support.get_arch_type(arch)


set_arch(arch)
set_arch_type(arch_type)


#构建应用
if arch_type=='arm':
    set_toolchains("arm-none-eabi")
elif arch_type=='x86':
    set_toolchains("i386-elf")
elif arch_type=='riscv':
    set_toolchains("riscv64-unknown-elf")
elif arch_type in ['dummy','general'] :
    set_toolchains("gcc")
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
    'doom'
]

if arch in ['armv5']:
    apps=['cmd','gui','hello','test','unitest',
        'microui', 'etk',  'lvgl', 'track',
        'sdl2', 'infones', 'launcher', #'mgba', 
        'lua',  'quickjs', 'gnuboy',
        'watch',
    ]
elif arch in ['riscv']:
    apps=[
        'cmd','gui','hello','test','unitest',
        'microui', 'etk',  'lvgl', 'track',
        'sdl2', 'infones', 'launcher', #'mgba',
    ]

# cpp应用
cpp_apps=[
    'xtrack','gmenu2x',
    'monogui','testcpp',
    'commander'
]

wayland_apps={
    'weston'
}

#构建的module
modules=[
    'devfs',
    'serial',
    'i2c',
    'spi',
    'loader',
    'posix',
    'test',
    'fat',
    'gpu',
    'mouse',
    'sdhci',
    'rtc',
    'gpio',
    'pty',
    'ipc',
    'musl',
    'gaga',
    'debug',
    'keyboard',
    'log',
    'hello',
    'sound'
    # 'debug',
    # 'fatfs',
    #'ewok'
]

if plat in['t113-s3','stm32f4xx','v3s']:
    modules+=['lcd']
elif arch in['x86']:
    modules+=['ahci','pci','vga']

#构建foot
foot=[
    'driver','service'
]
set_config('foot',foot)

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

option("single-kernel")
if arch_type=='x86':
    set_default(false)
else:
    set_default(true)
set_showmenu(true)
set_description("Enable kernel one file build")
option_end()


add_defines(plat.replace("-", "_").upper())


arch=get_arch()
if not arch:
    print('not found arch, please config',plat,'arch in support.py')
    exit(-1)
def_arch=arch.replace("-", "_").upper()

arch_type=get_arch_type()

if not arch_type:
    print('not found arch_type, please config',plat,'arch in support.py')
    exit(-1)

def_arch_type=arch_type.replace( "-", "_").upper()

add_defines(def_arch)
add_defines(def_arch_type)



arch_cflags=support.get_arch_cflags(arch)
plat_cflags=support.get_platform_cflags(plat)
arch_ldflags=support.get_arch_linkflags(arch)

if not arch_cflags:
    print('not found plat_cflags, please config',plat,'arch in support.py')
    exit(-1)

add_cflags(arch_cflags)
add_cflags(plat_cflags)

add_ldflags(arch_ldflags)

add_cxxflags(arch_cflags)

set_config('cflags',arch_cflags,plat_cflags)
set_config('cpp-apps',cpp_apps)
set_config('apps',apps)


#apply debug and release modes
add_rules("mode.debug", "mode.release","arch")

includes('./boot/ya.py')
includes('./duck/ya.py')
add_subs('./image/ya.py')
add_subs('./eggs/ya.py')
add_subs('./app/ya.py')
add_subs('./foot/ya.py')
