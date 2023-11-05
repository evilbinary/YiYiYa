# coding:utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
target("duck.img")
add_deps(
    'boot-init.elf',
    'kernel.elf'
)

add_files(
    "{buildir}/boot-init.bin",
    "{buildir}/kernel"
)

add_rules("make-image")
    

rule("its")
set_extensions(".bin", ".its","")




target("duck.fit")

add_deps(
    'boot-init.elf',
    'kernel.elf'
)
add_rules("its")

add_files('{buildir}/kernel-{plat}.its',
    rules = "its")

add_files(
    "{buildir}/boot-init.bin",
    rules = "its"
)

add_files("{buildir}/kernel",rules = "its")

set_configdir("{buildir}")
set_configvar("BOOT", "boot-init.bin" )
set_configvar("KERNEL", "kernel" )
add_configfiles('./kernel-{plat}.its')


def build(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch_type= target.get('arch_type')
    os.exec('mkimage -f '+sourcefiles[0]+' '+targetfile)

on_build(build)



target("uImage.img")

add_deps(
    'boot-init.elf',
    'kernel.elf'
)
add_rules("its")
add_files(
    "{buildir}/boot-init.bin",
    "{buildir}/kernel"
    ,rules = "its"
)


def build(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()

    arch_type= target.get('arch_type')

    print('build uimage '+arch_type+' '+targetfile)
    if arch_type=='x86': 
        os.exec('mkimage -n YiYiYa -A x86 -O u-boot -T kernel -C none -a 0x30008000 -e 0x30008000 -d '+sourcefiles[0]+' '+targetfile)
    elif arch_type=='arm':
        os.exec('mkimage -n YiYiYa -A arm -O u-boot -T kernel -C none -a 0x41000000 -e 0x1000000 -d '+sourcefiles[0]+' '+targetfile)

on_build(build)


#run
def run_qemu(plat,debug=False):

    def run(target):
        mode=target.get_config('mode')

        # import('core.base.global')        
        script_dir = path.directory(os.scriptdir())
        env_dir = path.join(script_dir,"xenv/")
        # import('support', { rootdir = env_dir })
        
        arch=target.get_arch()
        arch_type= target.get_arch_type()

        kernel_image="build/"+plat+"/"+arch+"/"+mode+"/duck.img"
        disk_img="image/disk.img"
        
        if is_host('mac'):
            os.exec('hdiutil attach '+disk_img)
            os.cp('app/resource/*', '/Volumes/YIYIYA/')
            os.exec('hdiutil eject /Volumes/YIYIYA')
        elif is_host('linux') :
            os.shell('mcopy -nmov  -i image/disk.img app/resource/* ::')
        else:
            os.exec('mcopy.exe -nmov  -i image/disk.img app/resource/* ::')
        

        run_qemu_cmd=''
        debug_qemu_cmd=''
  
        if has_config('single-kernel') :
            kernel_image= "build/"+plat+"/"+arch+"/"+mode+"/kernel.elf"

        if arch_type=='x86' :
            if target.get('arch')=='x86-duck' :
                run_qemu_cmd='boot/x86-duck/init.elf'
                debug_qemu_cmd='ggdb '+ run_qemu_cmd
            else:
                run_qemu_cmd='qemu-system-i386 -smp 1,sockets=1 -m 512M -name YiYiYa -rtc base=localtime,clock=host -boot a  -fda $SOURCE -serial stdio -D ./qemu.log  -drive id=disk,file='+disk_img+',format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -device sb16 -net nic,model=e1000 ' # -d in_asm -d cpu_reset -d in_asm,int,mmu -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor 
                run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                
                debug_qemu_cmd = 'qemu-system-i386 -smp 2,sockets=1 -m 512M  -name YiYiYa -rtc base=localtime -boot a -S -s -fda $SOURCE -serial stdio  -D ./qemu.log  -drive id=disk,file='+disk_img+',format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -device sb16  -net nic,model=e1000 '  #-d in_asm -d cpu_reset -d in_asm,int,mmu
                debug_qemu_cmd =debug_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
            
        elif arch_type =='arm' :

            if target.get('arch') =='armv7' :
                run_qemu_cmd='qemu-system-gnuarmeclipse -name YiYiYa #verbose #verbose #board STM32F429I-Discovery #mcu STM32F429ZITx   -rtc base=localtime -kernel duck/init/kernel.elf  -serial stdio  -D ./qemu.log -d unimp,guest_errors #semihosting-config enable -d in_asm,int,mmu,cpu_reset' #-d in_asm -d cpu_reset -d in_asm,int,mmu -d   -drive if=sd,id=sd0,format=raw,file='+disk_img+' #-d in_asm -d cpu_reset -d in_asm,int,mmu
                debug_qemu_cmd = run_qemu_cmd+ ' -S -s'

            elif string.find(target.get('plat') , 'orangepi-pc,cubieboard2') :
                run_qemu_cmd = 'qemu-system-arm -name YiYiYa -M orangepi-pc -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' -d in_asm,int,mmu,cpu_reset,guest_errors,strace'##-d in_asm -d cpu_reset -d in_asm,int,mmu,cpu_reset
                debug_qemu_cmd =run_qemu_cmd+' -S -s -monitor tcp:127.0.0.1:55555,server,nowait'

            elif string.find(target.get('plat'), 'raspi3' ) :
                run_qemu_cmd='qemu-system-aarch64 -name YiYiYa -M raspi3b  -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' ' # -d in_asm -d cpu_reset -d in_asm,int,mmu
                #run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                # run_qemu_cmd =run_qemu_cmd+' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd +' -S -s'
            else:
                run_qemu_cmd='qemu-system-arm -name YiYiYa -M raspi2b  -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' '##-d in_asm -d cpu_reset -d in_asm,int,mmu
                ## run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                ## run_qemu_cmd =run_qemu_cmd+' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd +' -S -s'
            
        elif arch_type=='xtensa' :
            run_qemu_cmd='~/dev/qemu-esp32/build/qemu-system-xtensa -nographic -M esp32 -drive file='+disk_img+',if=mtd,format=raw -s -serial stdio -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor  -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' -d in_asm -d cpu_reset -d in_asm,int,mmu' #-d in_asm -d cpu_reset -d in_asm,int,mmu
            debug_qemu_cmd = run_qemu_cmd +' -S '

        elif arch_type=='general' :
            run_qemu_cmd ='duck/init/kernel.elf'
            debug_qemu_cmd = 'lldb '+run_qemu_cmd

        elif arch_type=='riscv' :
            bios='none'
            if has_config('single-kernel') :
                bios='default'
            

            run_qemu_cmd='qemu-system-riscv32 -smp 1 -name YiYiYa -M virt -bios '+bios+' -rtc base=localtime -kernel '+kernel_image+' -drive id=disk,file='+disk_img+' -serial stdio -D ./qemu.log   ' #-serial stdio -d in_asm -d cpu_reset -d in_asm,int,mmu
            debug_qemu_cmd =run_qemu_cmd+' -S -s -monitor tcp:127.0.0.1:55555,server,nowait'
        else:
            print('no support run')
        

        if debug : 
            cprint('${green}run qemu debug ${clear} %s'%debug_qemu_cmd)
            os.exec(debug_qemu_cmd)
        else:
            cprint('${green}run qemu ${clear} %s'%run_qemu_cmd)
            os.exec(run_qemu_cmd)
        

    # print('qmeu=>',os.arch() ,plat )
    
    on_run(run)




target("debug")

add_deps("duck.img","disk.img")
add_rules("arch")
run_qemu('raspi2',True)



target("qemu")

add_deps("duck.img","disk.img")
add_rules("arch")
run_qemu('raspi2')


target("raspi2")

add_deps("duck.img","disk.img")
add_rules("arch")
run_qemu('raspi2')



target("disk.img")

# def build(target):
#     os.exec('qemu-img create image/disk.img 512m')
#     os.exec('mkfs.vfat -n disk image/disk.img')
#     pass

# on_build(build)

def run(target):
    print('make disk.img')
    os.exec('qemu-img create image/disk.img 512m')
    os.exec('mkfs.vfat -n YIYIYA image/disk.img')

on_run(run)


#v3s 运行

target("v3s")
add_deps("duck.fit")

def run(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch=target.get_arch()
    arch_type= target.get_arch_type()
    mode =target.get_config('mode')
    plat=target.plat()

    duck_fit="build/"+plat+"/"+arch+"/"+mode+"/duck.fit"

    print('run v3s fel ',duck_fit)

    os.shell('~/dev/c/sunxi-tools/sunxi-fel version uboot ~/dev/c/u-boot-v3s/u-boot-sunxi-with-spl.bin  write 0x41000000 '+duck_fit)

on_run(run)