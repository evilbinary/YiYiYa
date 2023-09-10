includes(
    "support.lua"
)

function append(array, ...)
    local arg = {...}
    if #arg == 1 and type(arg[1]) == "table" then
        for i = 1, #arg[1] do
            array[#array + 1] = arg[1][i]
        end
    else
        for i = 1, #arg do
            array[#array + 1] = arg[i]
        end
    end
end

function print_array(array)
    for i = 1, #array do
        print(array[i])
    end
end


plat = get_config('plat')

arch =''
arch_type ='' 
archs = ''
def_arch = ''
def_arch_type=''

if plat and plat ~='' then
    arch =get_arch(plat)
    arch_type =get_arch_type(arch)
    archs= get_archs(arch_type)

    if arch then
        def_arch=string.upper(string.gsub(arch, "-", "_"))
        def_arch_type=string.upper(string.gsub(arch_type, "-", "_"))
        set_config("arch", arch )
        set_config("arch_type", arch_type )
        set_config("def_arch",def_arch)
        set_config("def_arch_type",def_arch_type)
        set('arch',arch)
    end
end


rule("arch")
    before_run(function (target)
        import("support")
        import("core.project.config")
        import('core.base.global')
        import("show")

        arch=target:arch()
        plat=target:plat()
        arch_type= support.get_arch_type(arch)

        archs= support.get_archs(arch_type)

        global.set("arch",arch)
        global.set("arch_type",arch_type)
        global.set("plat",plat)

        config.set("arch",arch)
        config.set("arch_type",arch_type)
        -- config.set("plat",plat)

        target:set("arch_type", arch_type)
        target:set("arch", arch)
        target:set("plat", plat)


        show.show_logo(logo, {seed = 580})

        cprint('${magenta}welcome to yiyiya os build${clear}')
        cprint('${blink}Copyright (C) 2021-present evilbinary evilbinary.org')
        cprint('your select platform: '..plat..'${clear} arch: '..arch..' build env: '..os.host() )
        
    end)
    on_load(function (target)
        import("support")
        import("core.project.config")
        import('core.base.global')

        arch=target:arch()
        plat=target:plat()
        arch_type= support.get_arch_type(arch)

        support.set('arch',arch)

        global.set("arch",arch)
        global.set("arch_type",arch_type)
        global.set("plat",plat)

        config.set("arch",arch)
        config.set("arch_type",arch_type)
        -- config.set("plat",plat)

        target:set("arch_type", arch_type)
        target:set("arch", arch)
        target:set("plat", plat)
        
        import('core.base.global')
        import("lib.detect.find_library")

        local library = find_library("gcc", 
            {"/opt/local/lib/gcc/arm-none-eabi/*/",
            "/usr/lib/gcc/arm-none-eabi/*/"    
        },
            {kind = "static"}
        )
        -- print('libgcc',library.linkdir)
        target:add('linkdirs',library.linkdir)

    end)

rule("objcopy-file")
    after_build(function (target)
        local inputfile = target:targetfile()
        local outputfile = string.gsub(inputfile, "%.elf$", "")
        objcopy = target:tool("objcopy")
        os.execv(objcopy, {"-O", "binary", inputfile, outputfile..".bin"})
    end)

rule("kernel-objcopy")
    after_build(function (target)
        local inputfile = target:targetfile()
        local outputfile = string.gsub(inputfile, "%.elf$", "")
        objcopy = target:tool("objcopy")
        os.execv(objcopy,{"-S",inputfile, outputfile})
        os.execv(objcopy, {"-O", "binary", "--only-keep-debug", inputfile, outputfile..".dbg"})
        os.execv(objcopy, {"-O", "binary", "-S", inputfile, outputfile..".bin"})
        
    end)


rule("kernel-gen")
    set_extensions("",".elf")

    after_build(function (target)
        import("core.project.config")

        local sourcefiles = target:sourcefiles()

        local inputfile =''

        print("kernel gen target " .. target:name())
        for _, file in ipairs(sourcefiles) do
            print('file->',file)
            inputfile=file
        end

        print('kernel gen start '..inputfile)


        local file_size = os.filesize(inputfile)
        

        local kernel_size=  math.ceil((1023+ file_size)/1024)*1024
        local block_size = math.ceil(kernel_size/1024)

        print('kernel size: ',file_size,' size: ',kernel_size,'block size: ',block_size)


        target:set('KERNEL_SIZE',kernel_size)
        target:set('KERNEL_BLOCK_SIZE',block_size)

        
        arch_type =config.get('arch_type')
        if arch_type=='' then
            arch_type = target:get('arch_type')
        end
        
         -- 生成头文件内容
         local content = "#ifndef BOOT_"..string.upper(arch_type) .."_CONFIG_H\n"
         content= content .. "#define BOOT_"..string.upper(arch_type)  .."_CONFIG_H\n"
         content= content .. "#define KERNEL_BLOCK_SIZE ".. block_size.."\n"
         content= content .. "#define KERNEL_SIZE "..kernel_size.."\n"
         content= content .. "#endif"
 
 
         local script_dir = path.directory(os.scriptdir())
         local header_file = path.join(script_dir,"boot/"..arch_type.."/", "config.h")

         local file = io.open(header_file, "w")
         file:write(content)
         file:close()
 
         print("generated header file success: " .. header_file)

    end)

    on_build(function (target, rule)
       
    end)

rule("make-image")
    set_extensions("",".bin")
    on_build(function (target)
        import("core.project.config")

        local targetfile = target:targetfile()
        local sourcefiles = target:sourcefiles()
        local objectfiles = target:objectfiles()
        
        local file_size = os.filesize(targetfile)

        local kernel_size=  math.ceil((1023+ file_size)/1024.0)*1024
        local block_size = math.ceil(kernel_size/1024)
 
        print('make image '.. targetfile..' '..block_size)


        for _, file in ipairs(sourcefiles) do
            print('-->'..file)
        end

        arch_type =config.get('arch_type')
        if arch_type=='' then
            arch_type = target:get('arch_type')
        end

        if arch_type=='x86' then

        elseif arch_type=='arm' then
            if plat=='v3s' then
                os.exec('dd if=/dev/zero bs=512 count=2880 conv=notrunc of='..targetfile)
                os.exec('tools/mksunxi/mksunxiboot '..sourcefiles[1]..' boot/arm/init-spl.bin')
                os.exec('dd if=boot/arm/init-spl.bin bs=512 count=11 seek=0 conv=notrunc of='..targetfile)
                os.exec('dd if=${SOURCES[1]} bs=512 count='..block_size..' seek=12 conv=notrunc of='..targetfile)
            elseif plat=='stm32f4xx' then

            else
                os.exec('dd if=/dev/zero bs=512 count=2880 conv=notrunc of='..targetfile)

                os.exec('dd if='..sourcefiles[1]..' bs=512 count=11 seek=0 conv=notrunc of='..targetfile)
                os.exec('dd if='..sourcefiles[2]..' bs=512 count='..block_size..' seek=12 conv=notrunc of='..targetfile)
                print('make image finished ')
            end
        end
        
        
        

    end)

rule_end()


local root_res_dir =  os.projectdir().."/app/resource/"

function install_dir(path)
    before_build(function (target)
        if not os.exists(root_res_dir..path) then
            os.run("mkdir -p %s", root_res_dir..path)
        end
        -- if os.exists(target:scriptdir().."") then
        --     os.cp(target:scriptdir().."", rootfs_dir..path)
        -- end
    end)


    after_link(function (target)
        os.cp(target:targetfile(), root_res_dir..path)
    end)

    after_clean(function(target)
        os.rm(root_res_dir..path.."/"..target:name())
    end)
end

function set_type(type)
    if type == "lib" then
        set_kind("static")
    elseif type == "app" then
        set_kind("binary")
        install_dir("app")
    elseif type == "cli" then
        set_kind("binary")
        install_dir("bin")
    else
        print('not support')
    end


    if default_libc=='musl' then
        -- add_files(os.projectdir()..'/eggs/libmusl/lib/crt1.o')

        
        add_cflags(
            '-DDUCK -DDLIBC_POSIX',
             ' -D__LIB_MUSL__ ',
             '-static',
             '-nostdlib'
            )
        add_cxxflags(
            '-DDUCK -DDLIBC_POSIX',
                ' -D__LIB_MUSL__ ',
                '-static',
                '-nostdlib',
                ' -nostdinc'
            )
        add_ldflags(
            '-static'
        )

    end


    add_packages(default_libc)

    on_config(function (target)
        import('core.base.global')
        import("lib.detect.find_library")

        local library = find_library("gcc", 
            {"/opt/local/lib/gcc/arm-none-eabi/*/","/usr/lib/gcc/*/",
            "/usr/lib/gcc/arm-none-eabi/*/"},
            {kind = "static"}
        )
 
        -- print('libgcc',library.linkdir)
        target:add("ldflags",{
            '-L'..library.linkdir,
            "-lgcc"
        })

        if type=='cli' or type=='app' then
            target:add('ldflags','-Tapp/xlinker/user-'.. target:plat()..'.ld', {force=true})
        end

    end)

    on_run(function (target)end)

end