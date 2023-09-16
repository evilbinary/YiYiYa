local support_archs = {
    arm = {'armv5', 'armv6', 'armv7', 'armv7e-m', 'armv7-a', 'armv8-a'},
    x86 = {'x86', 'x86-64', 'x86-duck'},
    xtensa = {'lx6'},
    riscv = {'riscv'},
    dummy = {'dummy'},
    general = {'general'}
}

local support_platform = {
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
    ['riscv-virt'] = 'riscv',
    android ='armv7-a'
}

local support_arch_cflags = {
    x86= '',
    armv7= '',
    ['armv7e-m']= {'-mthumb', '-mthumb-interwork',  '-mfloat-abi=soft', '-mfpu=vfpv4-d16', '-mcpu=cortex-m4'},
    ['armv7-a']= {'-mcpu=cortex-a7',
                '-mtune=cortex-a7',
                '-mfpu=vfpv4',
                '-mfloat-abi=softfp'
            },
    ['armv8-a']= {'-mfpu=vfpv4', '-mfloat-abi=softfp'}
}


local support_arch_linkflags = {
    x86= '',
    armv7= '',
    ['armv7e-m']= {'-mthumb', '-mthumb-interwork',  '-mfloat-abi=soft', '-mfpu=vfpv4-d16', '-mcpu=cortex-m4'},
    ['armv7-a']= {'-mcpu=cortex-a7','-mtune=cortex-a7','-mfpu=vfpv4','-mfloat-abi=softfp'},
    ['armv8-a']={'-mcpu=cortex-a53', '-mtune=cortex-a53'}
}

local support_platform_cflags = {
    ['raspi2']= {'-nostdlib','-nostdinc'},-- --specs=nosys.specs
    ['raspi3']= {'-mcpu=cortex-a53','-mtune=cortex-a53'},
    ['stm32f4xx']= {'-specs=nosys.specs', '-nolibc', '-nostdlib', '-nostdinc' ,'-fno-builtin', '-DUSE_HAL_DRIVER'},
    ['android'] = {}
}

local support_config= {
    arch=''
}

function dump_table(table, indent)
    indent = indent or ""
    for key, value in pairs(table) do
        if type(value) == "table" then
            print(indent .. key .. ":")
            dumpTable(value, indent .. "  ")
        else
            print(indent .. key .. ": " .. tostring(value))
        end
    end
end

function get_arch_type(arch)
    local archs = {}
    for k, v in pairs(support_archs) do
        for i = 1, #v do
            archs[v[i]] = k
        end
    end
    return archs[arch]
end

function get_arch(p)
    return support_platform[p]
end


function get_archs(arch_type)
    return support_archs[arch_type]
end

function get_arch_cflags(key)
    return support_arch_cflags[key]
end 

function get_platform_cflags(key)
    return support_platform_cflags[key]
end 




function set(key,val)
    support_config[key]=val
end

function get(key)
    return support_config[key]
end
