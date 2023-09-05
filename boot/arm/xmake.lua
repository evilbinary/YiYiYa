if get_config('SINGLE_KERNEL') then 
        set_kind("object")

else

    target("boot-init.elf")
        -- set_extensions(".h",".o")
        add_deps('boot-config')

        add_files(
            'boot-$(arch).s',
            'init.c'
        )
        add_includedirs(
            '.',
            '../../duck',
            '../../duck/platform/$(plat)'
        )

        add_ldflags("-T "..path.join(os.scriptdir(), "/../arm/link-$(plat).ld"),  {force = true})

        add_rules('objcopy-file')
        
end


   