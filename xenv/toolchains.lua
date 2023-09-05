


toolchain("arm-none-eabi")
    -- mark as standalone toolchain
    set_kind("standalone")
    
    -- set toolset
    set_toolset("cc", "arm-none-eabi-gcc")
    set_toolset("cxx","arm-none-eabi-c++")
    set_toolset("ld", "arm-none-eabi-ld")
    set_toolset("ar", "arm-none-eabi-ar")
    set_toolset("strip", "arm-none-eabi-strip")
    set_toolset("as", "arm-none-eabi-gcc")
    set_toolset("objcopy", "arm-none-eabi-objcopy")

    arch_cflags=support_arch_cflags[arch]
    plat_cflags=support_platform_cflags[plat]
    add_cflags(arch_cflags,plat_cflags)

toolchain_end()

toolchain("riscv64-unknown-elf")
    -- mark as standalone toolchain
    set_kind("standalone")

    -- set toolset
    set_toolset("cc", "riscv64-unknown-elf-gcc")
    set_toolset("cxx","riscv64-unknown-elf-c++")
    set_toolset("ld", "riscv64-unknown-elf-ld")
    set_toolset("ar", "riscv64-unknown-elf-ar")
    set_toolset("strip", "riscv64-unknown-elf-strip")
    set_toolset("as", "riscv64-unknown-elf-gcc")
    set_toolset("objcopy", "riscv64-unknown-elf-objcopy")
toolchain_end()


toolchain("i386-elf")
    -- mark as standalone toolchain
    set_kind("standalone")

    -- set toolset
    set_toolset("cc", "i386-elf-gcc")
    set_toolset("cxx","i386-elf-c++")
    set_toolset("ld", "i386-elf-ld")
    set_toolset("ar", "i386-elf-ar")
    set_toolset("strip", "i386-elf-strip")
    set_toolset("as", "i386-elf-gcc")
    set_toolset("objcopy", "i386-elf-objcopy")
toolchain_end()

toolchain("i686-elf")
    -- mark as standalone toolchain
    set_kind("standalone")

    -- set toolset
    set_toolset("cc", "i686-elf-gcc")
    set_toolset("cxx","i686-elf-c++")
    set_toolset("ld", "i686-elf-ld")
    set_toolset("ar", "i686-elf--ar")
    set_toolset("strip", "i686-elf-strip")
    set_toolset("as", "i686-elf-gcc")
    set_toolset("objcopy", "i686-elf-objcopy")
toolchain_end()

