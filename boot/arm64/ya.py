# coding:utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************

if has_config('single-kernel'):
    # In single-kernel mode, boot code is linked with kernel
    # Just compile the assembly, don't create separate elf
    target("boot-init.elf")
    set_kind("object")
    add_files('boot-{arch}.s')
else:
    target("boot-init.elf")
    add_deps('boot-config')

    add_files(
        'boot-{arch}.s',
        'init-{arch}.c'
    )
    add_includedirs(
        '.',
        '../../duck',
        '../../duck/platform/{plat}'
    )

    # Use arm64 directory's own link script
    add_ldflags("-T"+path.join(os.scriptdir(), "link-{plat}.ld"), force = true)

    add_rules('objcopy-file')
