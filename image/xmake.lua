target("duck.img")
    

    add_deps(
        'boot-init.elf',
        'kernel.elf'
    )
    
    add_files(
        "$(buildir)/$(plat)/$(arch)/$(mode)/boot-init.bin",
        "$(buildir)/$(plat)/$(arch)/$(mode)/kernel"
    )

    add_rules("make-image")
    




target("duck.fit")

    add_deps(
        'boot-init.elf',
        'kernel.elf'
    )
    add_files(
        '$(buildir)/$(plat)/$(arch)/$(mode)/kernel-$(plat).its',
        "$(buildir)/$(plat)/$(arch)/$(mode)/boot-init.bin",
        "$(buildir)/$(plat)/$(arch)/$(mode)/kernel"
    )
    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    set_configvar("BOOT", "boot-init.bin" )
    set_configvar("KERNEL", "kernel" )
    add_configfiles('kernel-$(plat).its')


    on_build(function (target)
        local targetfile = target:targetfile()
        local sourcefiles = target:sourcefiles()
        arch_type= target:get('arch_type')

        os.exec('mkimage -f '..sourcefiles[1]..' '..targetfile)
    end)



target("uImage.img")

    add_deps(
        'boot-init.elf',
        'kernel.elf'
    )
    add_files(
        "$(buildir)/$(plat)/$(arch)/$(mode)/boot-init.bin",
        "$(buildir)/$(plat)/$(arch)/$(mode)/kernel"
    )

    on_build(function (target)
        local targetfile = target:targetfile()
        local sourcefiles = target:sourcefiles()

        arch_type= target:get('arch_type')

        print('build uimage '..arch_type..' '..targetfile)
        if arch_type=='x86' then 
            os.exec('mkimage -n YiYiYa -A x86 -O u-boot -T kernel -C none -a 0x30008000 -e 0x30008000 -d '..sourcefiles[1]..' '..targetfile)
        elseif arch_type=='arm' then
            os.exec('mkimage -n YiYiYa -A arm -O u-boot -T kernel -C none -a 0x41000000 -e 0x1000000 -d '..sourcefiles[1]..' '..targetfile)
        end
    end)


--run
function run_qemu(plat,mode,debug) 
    on_run(function (target)
        import('core.base.global')
        
        local script_dir = path.directory(os.scriptdir())
        local env_dir = path.join(script_dir,"xenv/")
        import('support', { rootdir = env_dir })
        
        arch=support.get_arch(plat)
        arch_type= support.get_arch_type(arch)

        kernel_image="build/"..plat.."/"..arch.."/"..mode.."/duck.img"
        disk_img="image/disk.img"
        
        run_qemu_cmd=''
        debug_qemu_cmd=''
  

        if target:get('SINGLE_KERNEL') then
            kernel_image= 'duck/init/kernel.elf'
        end

        if is_config('arch_type','x86') then
            if target:get('arch')=='x86-duck' then
                run_qemu_cmd='boot/x86-duck/init.elf'
                debug_qemu_cmd='ggdb '.. run_qemu_cmd
            else
                run_qemu_cmd='qemu-system-i386 -smp 1,sockets=1 -m 512M -name YiYiYa -rtc base=localtime,clock=host -boot a  -fda $SOURCE -serial stdio -D ./qemu.log  -drive id=disk,file='..disk_img..',format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -device sb16 -net nic,model=e1000 ' -- -d in_asm -d cpu_reset -d in_asm,int,mmu -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor 
                run_qemu_cmd =run_qemu_cmd..' -monitor tcp:127.0.0.1:55555,server,nowait'
                
                debug_qemu_cmd = 'qemu-system-i386 -smp 2,sockets=1 -m 512M  -name YiYiYa -rtc base=localtime -boot a -S -s -fda $SOURCE -serial stdio  -D ./qemu.log  -drive id=disk,file='..disk_img..',format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -device sb16  -net nic,model=e1000 '  ---d in_asm -d cpu_reset -d in_asm,int,mmu
                debug_qemu_cmd =debug_qemu_cmd..' -monitor tcp:127.0.0.1:55555,server,nowait'
            end
        elseif arch_type =='arm' then

            if target:get('arch') =='armv7' then
                run_qemu_cmd='qemu-system-gnuarmeclipse -name YiYiYa --verbose --verbose --board STM32F429I-Discovery --mcu STM32F429ZITx   -rtc base=localtime -kernel duck/init/kernel.elf  -serial stdio  -D ./qemu.log -d unimp,guest_errors --semihosting-config enable -d in_asm,int,mmu,cpu_reset' ---d in_asm -d cpu_reset -d in_asm,int,mmu -d   -drive if=sd,id=sd0,format=raw,file='..disk_img..' #-d in_asm -d cpu_reset -d in_asm,int,mmu
                debug_qemu_cmd = run_qemu_cmd.. ' -S -s'

            elseif string.find(target:get('plat') , 'orangepi-pc,cubieboard2') then
                run_qemu_cmd = 'qemu-system-arm -name YiYiYa -M orangepi-pc -rtc base=localtime -kernel '..kernel_image..'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='..disk_img..' -d in_asm,int,mmu,cpu_reset,guest_errors,strace'--#-d in_asm -d cpu_reset -d in_asm,int,mmu,cpu_reset
                debug_qemu_cmd =run_qemu_cmd..' -S -s -monitor tcp:127.0.0.1:55555,server,nowait'

            elseif string.find(target:get('plat'), 'raspi3' ) then
                run_qemu_cmd='qemu-system-aarch64 -name YiYiYa -M raspi3b  -rtc base=localtime -kernel '..kernel_image..'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='..disk_img..' ' -- -d in_asm -d cpu_reset -d in_asm,int,mmu
                --run_qemu_cmd =run_qemu_cmd..' -monitor tcp:127.0.0.1:55555,server,nowait'
                -- run_qemu_cmd =run_qemu_cmd..' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd ..' -S -s'
            else
                run_qemu_cmd='qemu-system-arm -name YiYiYa -M raspi2b  -rtc base=localtime -kernel '..kernel_image..'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='..disk_img..' -d cpu_reset -d in_asm,int,mmu'--#-d in_asm -d cpu_reset -d in_asm,int,mmu
                --# run_qemu_cmd =run_qemu_cmd..' -monitor tcp:127.0.0.1:55555,server,nowait'
                --# run_qemu_cmd =run_qemu_cmd..' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd ..' -S -s'
            end
        elseif arch_type=='xtensa' then
            run_qemu_cmd='~/dev/qemu-esp32/build/qemu-system-xtensa -nographic -M esp32 -drive file='..disk_img..',if=mtd,format=raw -s -serial stdio -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor  -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='..disk_img..' -d in_asm -d cpu_reset -d in_asm,int,mmu' ---d in_asm -d cpu_reset -d in_asm,int,mmu
            debug_qemu_cmd = run_qemu_cmd ..' -S '

        elseif arch_type=='general' then
            run_qemu_cmd ='duck/init/kernel.elf'
            debug_qemu_cmd = 'lldb '+run_qemu_cmd

        elseif arch_type=='riscv' then
            bios='none'
            if target:get('SINGLE_KERNEL') then
                bios='default'
            end

            run_qemu_cmd='qemu-system-riscv32 -smp 1 -name YiYiYa -M virt -bios '..bios..' -rtc base=localtime -kernel '..kernel_image..' -drive id=disk,file='..disk_img..' -serial stdio -D ./qemu.log   ' ---serial stdio -d in_asm -d cpu_reset -d in_asm,int,mmu
            debug_qemu_cmd =run_qemu_cmd..' -S -s -monitor tcp:127.0.0.1:55555,server,nowait'
        else
            print('no support run')
        end

        if debug then 
            cprint('${green}run qemu debug ${clear} %s',debug_qemu_cmd)
            os.exec(debug_qemu_cmd)
        else
            cprint('${green}run qemu ${clear} %s',run_qemu_cmd)
            os.exec(run_qemu_cmd)
        end

    -- print('qmeu=>',os.arch() ,plat )
    end)
    on_build(function (target) 

    end)
end

target("debug")
    
    add_deps("duck.img","disk.img")

    add_rules("arch")


    run_qemu('raspi2',get_config('mode'),true)

target("qemu")
    
    add_deps("duck.img","disk.img")

    add_rules("arch")


    run_qemu('raspi2',get_config('mode'))


target("raspi2")
    
    add_deps("duck.img","disk.img")

    add_rules("arch")
    run_qemu('raspi2',get_config('mode'))



target("disk.img")

    on_build(function (target)
        os.exec('qemu-img create image/disk.img 256m')
        os.exec('mkfs.vfat image/disk.img')
    end)

    on_run(function (target)
        print('make disk.img')

        os.exec('qemu-img create image/disk.img 256m')
        os.exec('mkfs.vfat image/disk.img')
    end)

