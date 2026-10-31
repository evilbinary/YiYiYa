# coding.utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者. evilbinary on 01/01/20
# * 邮箱. rootdebug@163.com
# ********************************************************************
from detect import find_library
# 架构相关映射（含 musl 动态链接器命名）在这个模块里；显式导入，
# 不依赖调用方命名空间里恰好有 support。
import support

rule("objcopy-file")

def build(target):
    inputfile = target.targetfile()
    outputfile = string.gsub(inputfile, r"\.elf$", "")
    objcopy = target.tool("objcopy")
    os.execv(objcopy, ["-O", "binary", inputfile, outputfile+".bin"])

after_build(build)


rule("kernel-gen")
set_extensions("",".elf")


def build (target):
    # import("core.project.config")

    sourcefiles = target.sourcefiles()

    inputfile =''

    print("kernel gen target " + target.name())
    for file in sourcefiles:
        print('file->',file)
        inputfile=file


    print('kernel gen start '+inputfile)


    file_size = os.filesize(inputfile)


    kernel_size=  math.ceil((1023+ file_size)/1024)*1024
    block_size = math.ceil(kernel_size/1024)
    block_512_size = math.ceil(kernel_size/512)

    print('file size ',file_size,'kernel size ',kernel_size,'block size ',block_size)


    target.set('KERNEL_SIZE',kernel_size)
    target.set('KERNEL_BLOCK_SIZE',block_512_size)


    arch_type =get_arch_type()
    if not arch_type:
        arch_type = target.get('arch_type')


    ## 生成头文件内容（按平台独立生成，避免 raspi3/raspi5 共用同一文件互相覆盖）
    plat = get_plat() or target.get('plat') or ''
    content = "#ifndef BOOT_"+string.upper(arch_type) +"_"+string.upper(plat)+"_CONFIG_H\n"
    content= content + "#define BOOT_"+string.upper(arch_type) + "_" + string.upper(plat)  +"_CONFIG_H\n"
    content= content + "#define KERNEL_BLOCK_SIZE "+ str(block_512_size)+"\n"
    content= content + "#define KERNEL_SIZE "+str(kernel_size)+"\n"
    content= content + "#endif"


    script_dir = path.directory(os.scriptdir())
    header_file = path.join(script_dir,"boot/"+str(arch_type)+"/")+ "/config-"+str(plat)+".h"

    file = io.open(header_file, "w")
    file.write(content)
    file.close()

    print("generated header file success. " + header_file)

after_build( build)




rule("make-image")

set_extensions("",".bin")

def build(target):
    # import("core.project.config")

    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    objectfiles = target.objectfiles()

    file_size = os.filesize(sourcefiles[1])

    kernel_size=  math.ceil((1023+ file_size)/1024.0)*1024
    block_size = math.ceil(kernel_size/1024)
    block_512_size = math.ceil(kernel_size/512)

    print('make image {} kernel size {} block size {}'.format( targetfile,kernel_size,block_size))


    for file in sourcefiles:
        print('-->'+file)
    
    plat =target.get('plat')
    arch_type = target.get('arch_type')
    
    if arch_type=='x86' :
        os.exec('dd if=/dev/zero bs=512 count=2880 conv=notrunc of='+targetfile)
        os.exec('dd if='+sourcefiles[0]+' bs=512 count=11 seek=0 conv=notrunc of='+targetfile)
        os.exec('dd if='+sourcefiles[1]+' bs=512 count='+str(block_512_size)+' seek=12 conv=notrunc of='+targetfile)
        print('make image finished ')
        pass
    elif arch_type=='arm':
        if plat=='v3s' :
            os.exec('dd if=/dev/zero bs=512 count=2880 conv=notrunc of='+targetfile)
            os.exec('tools/mksunxi/mksunxiboot '+sourcefiles[0]+' boot/arm/init-spl.bin')
            os.exec('dd if=boot/arm/init-spl.bin bs=512 count=11 seek=0 conv=notrunc of='+targetfile)
            os.exec('dd if=${SOURCES[1]} bs=512 count='+str(block_512_size)+' seek=12 conv=notrunc of='+targetfile)
        elif plat=='stm32f4xx' :
            pass
        else:
            os.exec('dd if=/dev/zero bs=512 count=2880 conv=notrunc of='+targetfile)

            os.exec('dd if='+sourcefiles[0]+' bs=512 count=11 seek=0 conv=notrunc of='+targetfile)
            os.exec('dd if='+sourcefiles[1]+' bs=512 count='+str(block_512_size)+' seek=12 conv=notrunc of='+targetfile)
            print('make image finished ')

on_build(build)
        
        
rule("arch")

def build (target):
    import globa,show
    
    arch=target.arch()
    plat=target.plat()
    arch_type= support.get_arch_type(arch)

    archs= support.get_archs(arch_type)

    globa.set("arch",arch)
    globa.set("arch_type",arch_type)
    globa.set("plat",plat)

    config.set("arch",arch)
    config.set("arch_type",arch_type)
    #config.set("plat",plat)

    target.set("arch_type", arch_type)
    target.set("arch", arch)
    target.set("plat", plat)


    show.show_logo(logo, {seed : 580})

    # cprint('${magenta}welcome to yiyiya os build${clear}')
    # cprint('${blink}Copyright (C) 2021-present evilbinary evilbinary.org')
    # cprint('your select platform: '+plat+'${clear} arch: '+arch+' build env: '+os.host() 

before_run(build)


def load (target):
    import globa
    import xenv.support as support    

    arch=target.arch()
    plat=target.plat()
    arch_type= support.get_arch_type(arch)

    target.set("arch_type", arch_type)
    target.set("arch", arch)
    target.set("plat", plat)

    library = find_library("gcc", 
        {"/opt/local/lib/gcc/arm-none-eabi/*/",
        "/usr/lib/gcc/arm-none-eabi/*/"    
    },
        kind= "static"
    )
    if library:
        target.add('linkdirs',library.linkdir)
        target.add("ldflags",[
            '-L'+library.linkdir,
            "-l"+library.link
        ],after=True)

        print('ldflags=>',target.get('ldflags') )
        target.add("includedir",library.linkdir)

# before_build(load)



rule("kernel-objcopy")

def build (target):
    inputfile = target.targetfile()
    outputfile = string.gsub(inputfile, r"\.elf$", "")
    objcopy = target.tool("objcopy")
    os.execv(objcopy,["-S",inputfile, outputfile])
    os.execv(objcopy, ["--only-keep-debug", inputfile, outputfile+".dbg"])
    os.execv(objcopy, ["-O", "binary", "-S", inputfile, outputfile+".bin"])
    
after_build(build)
 


# rule_end()

root_res_dir =  os.projectdir()+"/app/resource/"

def install_dir(path,target_name=False):
    install_path=root_res_dir+path
    

    def build(target):
        app_path=install_path
        if target_name:
            app_path+='/'+target.get('name')
        if not os.exists(app_path):            
            os.shell('mkdir',['-p',app_path])

    before_build(build)

    def link(target):
        app_path=install_path

        if target_name:
            app_path+='/'+target.get('name')+'/'
        os.cp(target.targetfile(), app_path)

    after_link(link)

    def clean(target):
        app_path=install_path

        if target_name:
            app_path+='/'+target.get('name')
        os.rm(app_path+"/"+target.name())

    after_clean(clean)


def install_res(path,file_name,target_name=True):
    install_path=root_res_dir+path

    def build(target):
        app_path=install_path
        if target_name:
            app_path+='/'+target.get('name')
        if not os.exists(app_path):            
            os.shell('mkdir',['-p',app_path])

        os.cp(target.get('file-path')+"/"+file_name, app_path)

    before_build(build)

    def link(target):
        app_path=install_path

        if target_name:
            app_path+='/'+target.get('name')+'/'
        os.cp(target.get('file-path')+"/"+file_name, app_path)

    # after_link(link)

    def clean(target):
        app_path=install_path

        if target_name:
            app_path+='/'+target.get('name')
        os.rm(app_path+"/"+target.name()+"/"+file_name)

    after_clean(clean)

# 各架构的裸机交叉编译器：编译器自带的头文件（arm_neon.h / arm_fp16.h /
# stddef.h / stdarg.h / float.h ...）与 libgcc 都随这套工具链分发。
gcc_cmds = {
    'arm': 'arm-none-eabi-gcc',
    'riscv': 'riscv64-unknown-elf-gcc',
    'xtensa': 'xtensa-esp32-elf-gcc',
    'arm64': 'aarch64-none-elf-gcc',
}

builtin_include_cache={}

def compiler_builtin_includedir(arch_type=None):
    '''返回交叉编译器自带 include 目录 <toolchain>/lib/gcc/<triple>/<ver>/include。

    只用于补偿 set_type() 给目标加的 -nostdinc：GCC 会把这个目录从搜索序列里
    摘掉，于是只在这儿的头（arm_neon.h、arm_acle.h 等）就找不到了。
    '''
    if not arch_type:
        arch_type = get_arch_type()
    if arch_type in builtin_include_cache:
        return builtin_include_cache.get(arch_type)
    import glob as _glob
    import os as _os
    import subprocess as _sub
    cands=[]
    gcc = gcc_cmds.get(arch_type)
    if gcc:
        try:
            out = _sub.check_output([gcc, '-print-file-name=include'],
                stderr=_sub.DEVNULL)
            cands.append(out.decode().strip())
        except:
            pass
    # 工具链不在 PATH 里时按常见安装位置兜底（macOS 的 Arm GNU Toolchain）
    for pattern in (
        '/Applications/ArmGNUToolchain/*/*/lib/gcc/*/*/include',
        '/opt/local/lib/gcc/*/*/include',
        '/usr/local/lib/gcc/*/*/include',
        '/usr/lib/gcc/*/*/include',
    ):
        cands += _glob.glob(pattern)
    found=''
    for d in cands:
        if d and _os.path.isdir(d):
            found=d
            break
    builtin_include_cache[arch_type]=found
    return found


def set_type(type, dynamic=False):
    if type == "lib":
        set_kind("static")
    elif type == "app":
        set_kind("binary")
        install_dir("app",True)
    elif type == "cli":
        set_kind("binary")
        install_dir("bin")
    elif type == "libexec":
        # 辅助进程（shell 拉起的 helper 客户端，如 weston-desktop-shell /
        # weston-keyboard）：调用方是按 LIBEXECDIR（/usr/libexec）的绝对路径
        # fork 它们的，放 /bin 里只会在运行时报 "Couldn't launch client"。
        # 这种目标只装到 usr/libexec，不再往 bin 里放一份。
        set_kind("binary")
        install_dir("usr/libexec")
    else:
        print('not support')

    default_libc=get_config('default_libc')
    # 【通用原子实现】armv5 等无 LDREX/LDREXD 的架构，GCC 会把 __atomic_xxx /
    # __sync_xxx 编成库调用，而 arm-none-eabi 的 libgcc 不提供 ⇒ SDL2、miniaudio
    # 这类库会链接失败。统一由 eggs/libatomic 提供（非 ARMv5 平台该文件编译为空，
    # 不会重复定义）。目标仅在有 app 时存在，故此处判断一下。
    if has_config('app'):
        add_deps('atomic')
    if default_libc=='musl':
        add_deps('gcc')
        # dynamic=True：链接成动态可执行文件。musl 的 libc.so（= 动态链接器
        # /lib/ld-musl-<arch>.so.1，由 eggs/libmusl 的 --enable-shared 产出）
        # 负责重定位与 dlopen，-export-dynamic 把本程序的符号导出给 dlopen 的
        # .so 用。解释器路径按架构取（见 support.musl_ldso_path，arm/riscv/x86
        # 都有对应名字，不写死 aarch64）。
        add_cflags(
            '-DDUCK -DDLIBC_POSIX',
             ' -D__LIB_MUSL__ ',
             *( [] if dynamic else ['-static'] ),
             '-nostdlib',
             '-nostdinc'
            )
        add_cxxflags(
            '-DDUCK -DDLIBC_POSIX',
                ' -D__LIB_MUSL__ ',
                *( [] if dynamic else ['-static'] ),
                '-nostdlib',
                '-nostdinc'
            )
        # 【解释器路径统一在这里给，不要散落到各 app 的 ya.py】
        # musl 是 --enable-shared 构建，cli/app 目标即使没声明 dynamic=True，
        # 链接时也会解析到 libc.so ⇒ 产物实际是动态可执行文件。这种情况下若
        # 不给 -dynamic-linker，ld 会写它自己的裸机默认值 /lib/ld.so.1（镜像里
        # 只有 /lib/ld-musl-<arch>.so.1），内核加载时直接 "load interp failed
        # /lib/ld.so.1"，程序连用户态都进不去。
        # 所以所有二进制目标（lib 是静态库，不会有 PT_INTERP）都显式给解释器
        # 路径；真的链成静态时该选项不产生 PT_INTERP，无副作用。
        if type != "lib":
            add_ldflags(
                '-dynamic-linker=' + support.musl_ldso_path(get_arch_type())
            )
        if dynamic:
            # 直接调 ld（不是 gcc 驱动），所以用 ld 自己的选项形式：
            #   -export-dynamic  → 符号进 .dynsym，dlopen 的 .so 才能解析本程序符号
            add_ldflags(
                '-export-dynamic'
            )
        else:
            add_ldflags(
                '-static'
            )
    elif default_libc=='newlib':
        add_deps('gcc')
        add_cflags(
            '-DDUCK -DDLIBC_POSIX',
             ' -D__LIB_NEWLIB__ ',
             '-static',
             '-nostdlib',
             '-nostdinc'
            )
        add_cxxflags(
            '-DDUCK -DDLIBC_POSIX',
                ' -D__LIB_NEWLIB__ ',
                '-static',
                '-nostdlib',
                '-nostdinc'
            )
        add_ldflags(
            '-static'
        )
    elif default_libc=='c':
        add_deps('gcc')
        
        add_cflags(
            '-DDUCK -DDLIBC_POSIX',
             ' -D__LIB_C__ ',
             '-static',
             '-nostdlib',
             '-nostdinc',
            )
        add_cxxflags(
            '-DDUCK -DDLIBC_POSIX',
                ' -D__LIB_C__ ',
                '-static',
                '-nostdlib',
                '-nostdinc'
            )
        add_ldflags(
            '-static'
        )
        
        add_ldflags('-L'+get_build_obj_dir()+'/eggs/libc/crt/')

    add_packages(default_libc)

    def config (target):
        # 【libexec 也必须用用户链接脚本】user-<plat>.ld 把 .text 放到
        # K_VIR_ADDR（如 0x71000000，落在本内核的用户 VA 窗口内）。漏掉
        # libexec 时（weston-desktop-shell / weston-keyboard /
        # ivi-shell-user-interface）会走 ld 默认基址 0x400000，叠加 64K
        # 的 p_align 后头段落在 0x3f0000 —— 正好压在内核映像/内核堆的
        # 物理恒等映射上，加载器照搬 p_vaddr 就会把内核内存清零。
        if type in ('cli', 'app', 'libexec'):
            target.add('ldflags','-Tapp/xlinker/user-'+ target.plat()+'.ld', force=true)

    on_config(config)

add_buildin('set_type',set_type)
add_buildin('install_res',install_res)


target("gcc")
set_kind('lib')
def config (target):
    arch_type=target.arch_type()
    library=None
    # gcc_cmds 提到模块级了（compiler_builtin_includedir 也要用同一份映射）
    gcc = gcc_cmds.get(arch_type)
    # 【编译器自带 include 目录】arm_neon.h / arm_fp16.h / arm_acle.h 这些
    # 只在交叉编译器里（<toolchain>/lib/gcc/<triple>/<ver>/include），而
    # set_type() 给目标带的 -nostdinc 会把它从搜索序列里摘掉：
    #   platform.c / matvec.c / vision/*.c 的 #if defined(__aarch64__) 分支
    #   ⇒ fatal error: arm_neon.h: No such file or directory
    # 在这里作为 gcc 依赖的 includedirs 下发，所有 add_deps('gcc') 的目标
    # （set_type 的 musl / newlib / c 三个分支都会加）都能拿到这个 -I。
    # 注意是 includedirs（复数）：ymake 只读这个键，上面几处 add("includedir")
    # 是单数，实际没进编译命令行。
    builtin_incdir = compiler_builtin_includedir(arch_type)
    if builtin_incdir:
        target.add("includedirs", [builtin_incdir])
    if gcc:
        try:
            import subprocess
            lib = subprocess.check_output(
                [gcc, '-print-libgcc-file-name'],
                stderr=subprocess.DEVNULL,
            ).decode().strip()
            if lib and os.exists(lib):
                linkdir = path.directory(lib)
                target.add("ldflags", ['-L'+linkdir, '-lgcc'])
                target.add("includedir", linkdir)
                return
        except:
            pass
    if arch_type in['arm']:
        library = find_library("gcc", 
                    [
                        "/opt/local/lib/gcc/arm-none-eabi/*/",
                        "/usr/lib/gcc/arm-none-eabi/*/",
                        "/usr/lib/gcc/*/",
                        "/usr/local/lib/gcc/*/"
                    ],
                    kind = "static"
                )
    elif arch_type in ['riscv']:
        library = find_library("gcc", 
            ["/usr/local/lib/gcc/riscv64-unknown-elf/*/",
            "/usr/lib/gcc/riscv64-unknown-elf/*/",
            "/opt/local/lib/gcc/riscv64-unknown-elf/*/",
            "/usr/lib/gcc/*/",
            ],
            kind = "static"
        )
    elif arch_type in ['xtensa']:
        library = find_library("gcc", 
            ["//Users/evil/.espressif/tools/xtensa-esp32-elf/esp-12.2.0_20230208/xtensa-esp32-elf/lib/gcc/xtensa-esp32-elf/12.2.0/",
            ],
            kind = "static"
        )
    elif arch_type in['arm64']:
        library = find_library("gcc", 
            [
                "/opt/local/lib/gcc/aarch64-none-elf/*/",
                "/usr/lib/gcc/aarch64-none-elf/*/",
                "/usr/lib/gcc/*/",
                "/usr/local/lib/gcc/*/",
                "/Applications/ArmGNUToolchain/12.2.rel1/aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/12.2.1/"
            ],
            kind = "static"
            )
    if library:
        target.add("ldflags",[
            '-L'+library.linkdir,
            "-l"+library.link
        ])
        target.add("includedir",library.linkdir)
    else:
        print('not found gcc lib')

on_config(config)