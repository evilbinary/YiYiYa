# coding:utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
if get_config('single-kernel'): 
        set_kind("object")

else:
    target("boot-init.elf")
    ## set_extensions(".h",".o")
    add_deps('boot-config')

    add_files(
        'boot-{arch}.s',
        'init.c'
    )
    add_includedirs(
        '.',
        '../../duck',
        '../../duck/platform/{plat}'
    )


    add_ldflags("-T"+path.join(os.scriptdir(), "../arm/link-{plat}.ld"), force = true)

    add_rules('objcopy-file')
        


   