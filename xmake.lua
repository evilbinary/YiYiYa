-- 设置工程名
set_project("yiyiya")

-- 设置工程版本
set_version("1.3.0")

includes(
    "xenv/env.lua",
    "xenv/toolchains.lua"
)

set_defaultplat("raspi2")
set_config("arch", "armv7-a" )
set_config("arch_type","")

set_policy("check.auto_ignore_flags", false)


add_platformdirs("./xenv/platform")
add_moduledirs('./xenv/module')

--构建应用
set_config("apps", false )

if is_plat("raspi2") then
    set_toolchains("arm-none-eabi")

elseif is_plat("raspi3") then
    set_toolchains("arm-none-eabi")
else 
    set_toolchains("arm-none-eabi")
end

--默认libc
default_libc = 'musl'  -- musl c newlib

--应用
apps = {              
    'cmd','gui','hello', 'test','unitest',
    'microui', 'etk',  'lvgl', 'track',
    'sdl2', 'infones', 'launcher', 'mgba', 
    'lua', 'scheme', 'quickjs', 'gnuboy',
    'watch',
    --'sdlmine',
    'sdl',
}

-- cpp应用
cpp_apps={ 
    'xtrack','gmenu2x',
    'monogui','testcpp'
}

--构建的module
modules={
    'posix','loader','musl','ipc','gaga','log','pty' 
    --'musl','gaga','ipc','pty','log','debug','ewok'
}
if not plat then 

return 

end

if plat then
    -- print('plat',plat,'--->',get_config('plat'))
    arch=get_arch(plat)

    add_defines(string.upper(plat))
    add_defines(def_arch)
    add_defines(def_arch_type)
    
    arch_cflags=get_arch_cflags(arch)
    plat_cflags=get_platform_cflags(plat)
    add_cflags(arch_cflags,plat_cflags)
    add_cxxflags(arch_cflags)
    set_config('cflags',arch_cflags,plat_cflags)
end

-- print('get cflags ',get_config('cflags'))

--c++ 应用 输入才编译
set_config("cpp-apps",false)

-- apply debug and release modes
add_rules("mode.debug", "mode.release","arch")

includes("image/xmake.lua")
includes("boot/xmake.lua")
includes("duck/xmake.lua")
includes("eggs/xmake.lua")
includes("app/xmake.lua")