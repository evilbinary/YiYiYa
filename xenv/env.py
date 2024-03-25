# coding.utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者. evilbinary on 01/01/20
# * 邮箱. rootdebug@163.com
# ********************************************************************
from detect import find_library

rule("objcopy-file")

def build(target):
    inputfile = target.targetfile()
    outputfile = string.gsub(inputfile, "\.elf$", "")
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


    ## 生成头文件内容
    content = "#ifndef BOOT_"+string.upper(arch_type) +"_CONFIG_H\n"
    content= content + "#define BOOT_"+string.upper(arch_type)  +"_CONFIG_H\n"
    content= content + "#define KERNEL_BLOCK_SIZE "+ str(block_512_size)+"\n"
    content= content + "#define KERNEL_SIZE "+str(kernel_size)+"\n"
    content= content + "#endif"


    script_dir = path.directory(os.scriptdir())
    header_file = path.join(script_dir,"boot/"+str(arch_type)+"/")+ "/config.h"

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
    target.add('linkdirs',library.linkdir)
    if library:
        # print('target=>',target.get('name'))
        # print('libgcc==>',library)
        
        target.add("ldflags",[
            '-L'+library.linkdir,
            "-l"+library.link
        ],after=True)

        print('ldflags=>',target.get('ldflags') )
        # target.add("ldflags",[
        #     '-L'+library.linkdir+'/'+library.filename
        # ])
        target.add("includedir",library.linkdir)

# before_build(load)



rule("kernel-objcopy")

def build (target):
    inputfile = target.targetfile()
    outputfile = string.gsub(inputfile, "\.elf$", "")
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


def set_type(type):
    if type == "lib":
        set_kind("static")
    elif type == "app":
        set_kind("binary")
        install_dir("app",True)
    elif type == "cli":
        set_kind("binary")
        install_dir("bin")
    else:
        print('not support')

    default_libc=get_config('default_libc')
    if default_libc=='musl':
        add_deps('gcc')
        # add_files(os.projectdir()+'/eggs/libmusl/lib/crt1.o')
        add_cflags(
            '-DDUCK -DDLIBC_POSIX',
             ' -D__LIB_MUSL__ ',
             '-static',
             '-nostdlib',
             '-nostdinc'
            )
        add_cxxflags(
            '-DDUCK -DDLIBC_POSIX',
                ' -D__LIB_MUSL__ ',
                '-static',
                '-nostdlib',
                '-nostdinc'
            )
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
        if type=='cli' or type=='app':
            target.add('ldflags','-Tapp/xlinker/user-'+ target.plat()+'.ld', force=true)

    on_config(config)

add_buildin('set_type',set_type)



target("gcc")
set_kind('lib')
def config (target):

    library = find_library("gcc", 
                ["/opt/local/lib/gcc/arm-none-eabi/*/","/usr/lib/gcc/*/",
                "/usr/lib/gcc/arm-none-eabi/*/"],
                kind = "static"
            )
    # print('found gcc lib',library)
    if library:
        target.add("ldflags",[
            '-L'+library.linkdir,
            "-l"+library.link
        ])
        target.add("includedir",library.linkdir)
    else:
        print('not found gcc lib')

on_config(config)