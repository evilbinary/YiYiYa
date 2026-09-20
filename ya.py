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
    '/Users/evil/dev/c/ymake/ymake/',
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
elif arch_type =='arm64':
    set_toolchains('aarch64-none-elf')
elif arch_type =='xtensa':
    set_toolchains('xtensa-esp32-elf')
else:
    set_toolchains("arm-none-eabi")

#默认libc
default_libc = 'musl'  # musl c newlib

#应用
apps = [              
    'init',#内核应用
    'cmd','gui','hello', 'test','unitest',
    'microui', 'etk',  'lvgl', 'track',
    'sdl2', 'infones', 'launcher', 'mgba', 
    'lua', 'scheme', 'qjs', 'gnuboy',
    'watch',
    #'sdlmine',
    'sdl',
    'doom',
    'doom2',
    'gpsp',
    'yui',

    #lvglapp
    # 'lvglapp'
]

em_apps=[]

#构建foot
foot=[
    'driver','service'
]

if arch in ['armv5']:
    apps=['init','cmd','gui','hello','test','unitest',
        'microui', 'etk',  'lvgl', 'track',
        'sdl2', 'infones', 'launcher', #'mgba', 
        'lua',  'qjs', 'gnuboy',
        'watch',
        'sdl',
        # yui（ymain/ymario/yqjs 目标在 app/yui/ya.py）：armv5 之前被整体排除，
        # 导致 `ya -p versatilepb -b ymain` 报 "not found target ymain"
        'yui'
    ]
elif arch in ['riscv']:
    apps=[
        'cmd','gui','hello','test','unitest',
        'microui', 'etk',  'lvgl', 'track',
        'sdl2', 'infones', 'launcher', #'mgba',
    ]
elif arch in ['lx6']:
    apps=[
    ]
    default_libc='c'
    foot=[]

if plat in ['stm32f4xx','esp32']:
    apps=[
        'emapp'
    ]
    default_libc='c'
    foot=[]
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
    'sysconf',
    'posix',
    'test',
    'gpu',
    'mouse',
    'sdhci',
    'rtc',
    'gpio',
    'pty',
    'ipc',
    'musl',
    'gaga',
    'trace',
    'perf',
    'backtrace',
    'irq',
    'keyboard',
    'log',
    'hello',
    'sound',
    'dma',
    # 'fat',
    'fatfs',
    'ipc',
    'power',
    # 'trace',
    #'ewok',
    'net',
    'xwin',
    'usb',
]

if plat in['t113-s3','stm32f4xx','v3s']:
    modules+=['lcd']
elif arch in['x86']:
    modules+=['ahci','pci','vga']

if plat == 'raspi5':
    # raspi5 移植初期先不构建 yui（其依赖 mquickjs/jsmodule 尚未适配 aarch64）
    apps=[a for a in apps if a != 'yui']

if plat == 'raspi5':
    # raspi5 移植初期：无 DWC2 USB / BCM2837 网络（Pi5 用 xHCI/PCIe），
    # 暂不构建这两个模块
    modules=[m for m in modules if m not in ('net','usb')]




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
if len(apps)==0:
    set_default(false)
    set_showmenu(false)
else:
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
    print('not found arch_type, please config',plat,'arch type in support.py')
    exit(-1)

def_arch_type=arch_type.replace( "-", "_").upper()

add_defines(def_arch)
add_defines(def_arch_type)

# 哪些平台有【真正可用】的网卡驱动（与 duck/modules/net/ya.py 的 plat_source 对应）：
# 只有这些平台才在 app/init/module.c 里注册 net 模块。net 模块是静态库，
# 一旦引用 net_module 就会拉入 net.o 并要求 net_init_device 有实现。
#
# 【2026-09-19 收窄为只有 v3s】原先还列了 raspi2/raspi3（走 bcm2837.c）、
# qemu/dmulator/versatilepb（走 e1000.c），但那几个都是骨架驱动：
#   · bcm2837.c（248 行、3 处 "In a real implementation"）一进 net_init() 就去读
#     未映射的 USB OTG 窗口 0x3F980000 ⇒ 实测 raspi2 在 QEMU 里直接
#     "module run net → kernel memory fault at 3f980010 / pte 3f980000 -> 0" 断掉，
#     启动再也走不下去；
#   · e1000.c 只有 85 行，同样是骨架。
# 注意：module.c 里"今天之前"根本没有 net 注册，所以把平台列进来本身就是"新开功能"，
# 会给该平台引入启动风险。等某个驱动真正实现（至少先 page_map 自己的寄存器窗口，
# 并做到 init 失败可返回）再把它的平台加回来。
net_driver_plats = ['v3s']
if plat in net_driver_plats:
    add_defines('NET_DRIVER')

arch_cflags=support.get_arch_cflags(arch)
plat_cflags=support.get_platform_cflags(plat)
arch_ldflags=support.get_arch_linkflags(arch)



if arch_cflags:
    add_cflags(arch_cflags)
    add_cxxflags(arch_cflags)
    
if plat_cflags:
    add_cflags(plat_cflags)

add_ldflags(arch_ldflags)

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

add_subs('./app/ya.py')
add_subs('./foot/ya.py')
