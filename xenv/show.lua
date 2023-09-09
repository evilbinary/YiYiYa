local cli       = import("core.base.cli")
local tty       = import("core.base.tty")
local colors    = import("core.base.colors")
local text      = import("core.base.text")
-- show logo
function show_logo(logo, opt)

    logo = logo or [[
        _  _  _ _  _  _      
       | || || | || || |     
       | \\| |/ | \\| |/ __  __
        \\_   _/ \\_   _/  \\/ /
          | |     | |( ()  < 
          |_|     |_| \\__/\\_\\
       2021 - present Copyright by evilbinary
   ]]

   local footer = [[
        ${bright}Project${clear}: ${underline}https://github.com/evilbinary/YiYiYa${clear}
        ${bright}HomePage${clear}: ${underline}http://evilbinary.org${clear}
       ]]

    opt = opt or {}
    if tty.has_color24() or tty.has_color256() then
        local lines = {}
        local seed  = opt.seed or 236
        for _, line in ipairs(logo:split("\n")) do
            local i = 0
            local line2 = ""
            line:gsub(".", function (c)
                local code = tty.has_color24() and colors.rainbow24(i, seed) or colors.rainbow256(i, seed)
                line2 = string.format("%s${bright %s}%s", line2, code, c)
                i = i + 1
            end)
            table.insert(lines, line2)
        end
        logo = table.concat(lines, "\n")
    end

    -- show logo
    cprint(colors.translate(logo))

    -- show footer
    cprint(colors.translate(footer))

end