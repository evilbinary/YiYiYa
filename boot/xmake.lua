if arch_type~='general' then 
    if arch_type and arch_type~='' then
        includes("./"..arch_type.."/xmake.lua")
    end
end
