target("duck.img")
    

    add_deps(
        'boot-init.elf',
        'kernel.elf'
    )
    
    add_files(
        "$(buildir)/$(plat)/$(arch)/$(mode)/boot-init.bin",
        "$(buildir)/$(plat)/$(arch)/$(mode)/kernel"
    )

    set_values("arch_type",arch_type)
    add_rules("make-image")
    


