
support_archs = {
    arm = {'armv5', 'armv6', 'armv7', 'armv7e-m', 'armv7-a', 'armv8-a'},
    x86 = {'x86', 'x86-64', 'x86-duck'},
    xtensa = {'lx6'},
    riscv = {'riscv'},
    dummy = {'dummy'},
    general = {'general'}
}

support_platform = {
    stm32f4xx = 'armv7e-m',
    ['i386-pc'] = 'x86',
    raspi2 = 'armv7-a',
    esp32 = 'lx6',
    v3s = 'armv7-a',
    raspi3 = 'armv8-a',
    rk3128 = 'armv7-a',
    cubieboard2 = 'armv7-a',
    dmulator = 'general',
    dummy = 'dummy',
    rk3288 = 'armv7-a',
    ['orangepi-pc'] = 'armv7-a',
    ['riscv-virt'] = 'riscv'
}

support_arch_cflags = {
    x86= '',
    armv7= '',
    ['armv7e-m']= '-mthumb -mthumb-interwork  -mfloat-abi=soft -mfpu=vfpv4-d16 -mcpu=cortex-m4 ',
    ['armv7-a']= '-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=softfp ',
    ['armv8-a']= ' -mfpu=vfpv4 -mfloat-abi=softfp '
}


support_arch_linkflags = {
    x86= '',
    armv7= '',
    ['armv7e-m']= '-mthumb -mthumb-interwork  -mfloat-abi=soft -mfpu=vfpv4-d16 -mcpu=cortex-m4 ',
    ['armv7-a']= '-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=softfp ',
    ['armv8-a']='-mcpu=cortex-a53 -mtune=cortex-a53'
}

support_platform_cflags = {
    ['raspi2']= '-nostdlib -nostdinc ',-- --specs=nosys.specs
    ['raspi3']= '-mcpu=cortex-a53 -mtune=cortex-a53',
    ['stm32f4xx']= '-specs=nosys.specs -nolibc -nostdlib -nostdinc -fno-builtin -DUSE_HAL_DRIVER'

}

function get_arch(arch)
    local archs = {}
    for k, v in pairs(support_archs) do
        for i = 1, #v do
            archs[v[i]] = k
        end
    end
    return archs[arch]
end

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
    arch =support_platform[plat]
    arch_type =get_arch(arch)
    archs= support_archs[arch_type]

    if arch then
        def_arch=string.upper(string.gsub(arch, "-", "_"))
        def_arch_type=string.upper(string.gsub(arch_type, "-", "_"))
        set_config("arch", arch )
        set_config("arch_type", arch_type )
        set_config("def_arch",def_arch)
        set_config("def_arch_type",def_arch_type)
    end
end


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
        os.execv(objcopy,{"-O", "binary",inputfile, outputfile})
        os.runv(objcopy, {"-O", "binary", "--only-keep-debug", inputfile, outputfile..".dbg"})
        os.runv(objcopy, {"-O", "binary", "-S", inputfile, outputfile..".bin"})
        
    end)
   

rule("kernel-gen")
    set_extensions("",".elf")

    after_build(function (target)
        import("core.project.config")

        local sourcefiles = target:sourcefiles()

        local inputfile =''

        print("target " .. target:name())
        for _, file in ipairs(sourcefiles) do
            print('file->',file)
            inputfile=file
        end

        print('kernel gen start '..inputfile)


        local file_size = os.filesize(inputfile)
        

        local kernel_size=  math.ceil((1023+ file_size)/1024.0)*1024
        local block_size = math.ceil(kernel_size/1024)

        print('kernel size: ',file_size,' size: ',kernel_size,'block size: ',block_size)


        target:set('KERNEL_SIZE',kernel_size)
        target:set('KERNEL_BLOCK_SIZE',block_size)

        
        arch_type =config.get('arch_type')
        if arch_type=='' then
            arch_type = target:values('arch_type')
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
        
        print('make image '.. targetfile..' ')

        for _, file in ipairs(sourcefiles) do
            print('-->'..file)
        end

        arch_type =config.get('arch_type')
        if arch_type=='' then
            arch_type = target:values('arch_type')
        end

        if arch_type=='x86' then

        elseif arch_type=='arm' then
            if plat=='v3s' then
                os.exec('dd if=/dev/zero bs=512 count=2880 conv=notrunc of='..targetfile)
                os.exec('tools/mksunxi/mksunxiboot '..sourcefiles[1]..' boot/arm/init-spl.bin')
                os.exec('dd if=boot/arm/init-spl.bin bs=512 count=11 seek=0 conv=notrunc of='..targetfile)
                os.exec('dd if=${SOURCES[1]} bs=512 count=11 seek=12 conv=notrunc of='..targetfile)
            elseif plat=='stm32f4xx' then

            else
                os.exec('dd if=/dev/zero bs=512 count=2880 conv=notrunc of='..targetfile)

                os.exec('dd if='..sourcefiles[1]..' bs=512 count=11 seek=0 conv=notrunc of='..targetfile)
                os.exec('dd if='..sourcefiles[2]..' bs=512 count=11 seek=12 conv=notrunc of='..targetfile)
                print('make image finished ')
            end
        end
        
        
        

    end)
