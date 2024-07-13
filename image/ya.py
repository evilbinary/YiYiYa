# coding:utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
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

if has_config('single-kernel'): 
    target("duck.fit")
    target("duck.img")
else:
    target("duck.img")
    add_deps(
        'boot-init.elf',
        'kernel.elf',
        'kernel'
    )

    add_files(
        "{buildir}/boot-init.bin",
        "{buildir}/kernel",
    )

    add_rules("make-image")
        



target("uImage.img")

if has_config('single-kernel'): 
    add_deps(
        'kernel.elf'
    )
    add_rules("its")
    add_files(
        "{buildir}/kernel.bin"
        ,rules = "its"
    )
else:
    add_deps(
        'boot-init.elf',
        'kernel.elf'
    )
    add_rules("its")
    add_files(
        "{buildir}/boot-init.bin",
        "{buildir}/kernel.bin"
        ,rules = "its"
    )


def build(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()

    arch_type= target.get('arch_type')
    plat= target.get('plat')

    print('build uimage '+arch_type+' '+targetfile)
    if arch_type=='x86': 
        os.exec('mkimage -n YiYiYa -A x86 -O u-boot -T kernel -C none -a 0x30008000 -e 0x30008000 -d '+sourcefiles[0]+' '+targetfile)
    elif arch_type=='arm':
        if plat in ['f1c200s','f1c100s']:
            cmd='mkimage -n YiYiYa -A arm -O u-boot -T kernel -C none -a 0x82000000 -e 0x82000000 -d '+sourcefiles[0]+' '+targetfile
            os.exec(cmd)
        else:
            cmd='mkimage -n YiYiYa -A arm -O u-boot -T kernel -C none -a 0x42000000 -e 0x42000000 -d '+sourcefiles[0]+' '+targetfile
            os.exec(cmd)

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
                run_qemu_cmd='qemu-system-i386 -smp 1,sockets=1 -m 512M -name YiYiYa -rtc base=localtime,clock=host -boot a  -fda '+kernel_image+' -serial stdio -D ./qemu.log  -drive id=disk,file='+disk_img+',format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -device sb16 -net nic,model=e1000 -d in_asm,int,mmu' # -d in_asm -d cpu_reset -d in_asm,int,mmu -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor 
                run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                
                debug_qemu_cmd = 'qemu-system-i386 -smp 2,sockets=1 -m 512M  -name YiYiYa -rtc base=localtime -boot a -S -s -fda '+kernel_image+' -serial stdio  -D ./qemu.log  -drive id=disk,file='+disk_img+',format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -device sb16  -net nic,model=e1000 '  #-d in_asm -d cpu_reset -d in_asm,int,mmu
                debug_qemu_cmd =debug_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
            
        elif arch_type =='arm' :

            if target.get('arch') =='armv7' :
                run_qemu_cmd='qemu-system-gnuarmeclipse -name YiYiYa -verbose -board STM32F429I-Discovery -mcu STM32F429ZITx   -rtc base=localtime -kernel '+kernel_image+'  -serial stdio  -D ./qemu.log -d unimp,guest_errors -semihosting-config enable -d in_asm,int,mmu,cpu_reset' #-d in_asm -d cpu_reset -d in_asm,int,mmu -d   -drive if=sd,id=sd0,format=raw,file='+disk_img+' #-d in_asm -d cpu_reset -d in_asm,int,mmu
                debug_qemu_cmd = run_qemu_cmd+ ' -S -s'

            elif string.find(target.plat() , 'orangepi-pc,cubieboard2') :
                run_qemu_cmd = 'qemu-system-arm -name YiYiYa -M orangepi-pc -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' -d in_asm,int,mmu,cpu_reset,guest_errors,strace'##-d in_asm -d cpu_reset -d in_asm,int,mmu,cpu_reset
                debug_qemu_cmd =run_qemu_cmd+' -S -s -monitor tcp:127.0.0.1:55555,server,nowait'

            elif string.find(target.plat(), 'raspi3' ) :
                run_qemu_cmd='qemu-system-aarch64 -name YiYiYa -M raspi3b  -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' ' # -d in_asm -d cpu_reset -d in_asm,int,mmu
                #run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                # run_qemu_cmd =run_qemu_cmd+' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd +' -S -s'
            elif target.plat() in ['versatilepb']:
                run_qemu_cmd='qemu-system-arm -name YiYiYa -M versatilepb -cpu arm926 -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' '##-d in_asm -d cpu_reset -d in_asm,int,mmu
                ## run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                ## run_qemu_cmd =run_qemu_cmd+' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd +' -S -s'
            else:
                print('-->', target.plat())
                run_qemu_cmd='qemu-system-arm -name YiYiYa -M raspi2b  -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+'  '##-d in_asm -d cpu_reset -d in_asm,int,mmu 
                ## run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                ## run_qemu_cmd =run_qemu_cmd+' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd +' -S -s'
            
        elif arch_type=='xtensa' :
            run_qemu_cmd='~/dev/qemu-esp32/build/qemu-system-xtensa -cpu lx6 -nographic -M esp32 -kernel '+kernel_image+' -drive file='+disk_img+',if=mtd,format=raw -s -serial stdio -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor  -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' -d in_asm -d cpu_reset -d in_asm,int,mmu' #-d in_asm -d cpu_reset -d in_asm,int,mmu
            debug_qemu_cmd = run_qemu_cmd +' -S '

        elif arch_type=='general' :
            run_qemu_cmd ='duck/init/kernel.elf'
            debug_qemu_cmd = 'lldb '+run_qemu_cmd

        elif arch_type=='riscv' :
            bios='none'
            if has_config('single-kernel') :
                bios='default'
            

            run_qemu_cmd='qemu-system-riscv32 -machine virt -cpu rv32 -smp 1 -name YiYiYa -M virt -bios '+bios+' -rtc base=localtime -kernel '+kernel_image+' -drive id=disk,file='+disk_img+' -serial stdio -D ./qemu.log  -d in_asm,int,mmu ' #-serial stdio -d in_asm -d cpu_reset -d in_asm,int,mmu
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

plat=get_plat()
if not plat:
    plat='raspi2'


run_qemu(plat,True)



target("qemu")

add_deps("duck.img","disk.img")
add_rules("arch")

plat=get_plat()
if not plat:
    plat='raspi2'

run_qemu(plat)


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
# add_deps("duck.fit")
add_deps("uImage.img")

def run(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch=target.get_arch()
    arch_type= target.get_arch_type()
    mode =target.get_config('mode')
    plat=target.plat()

    duck_fit="build/"+plat+"/"+arch+"/"+mode+"/duck.fit"
    duck_img="build/"+plat+"/"+arch+"/"+mode+"/uImage.img"
    duck_kernel="build/"+plat+"/"+arch+"/"+mode+"/kernel"

    print('run '+plat+' fel',duck_kernel)

    # os.shell('~/dev/c/sunxi-tools/sunxi-fel version uboot ~/dev/c/u-boot-v3s/u-boot-sunxi-with-spl.bin  write 0x41000000 '+duck_kernel)
    # os.shell('~/dev/c/sunxi-tools/sunxi-fel version uboot ~/dev/c/u-boot-v3s/u-boot-sunxi-with-spl.bin  write 0x41000000 '+duck_fit)
    os.shell('~/dev/c/sunxi-tools/sunxi-fel -p version uboot ~/dev/c/u-boot-v3s/u-boot-sunxi-with-spl.bin  write 0x41000000 '+duck_img)


on_run(run)


#t113-s3 运行
target("t113-s3")
# add_deps("duck.fit")
add_deps("uImage.img")

def run(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch=target.get_arch()
    arch_type= target.get_arch_type()
    mode =target.get_config('mode')
    plat=target.plat()

    duck_fit="build/"+plat+"/"+arch+"/"+mode+"/duck.fit"
    duck_kernel="build/"+plat+"/"+arch+"/"+mode+"/kernel"
    duck_img="build/"+plat+"/"+arch+"/"+mode+"/uImage.img"


    print('run '+plat+' fel',duck_kernel)

    os.shell('~/dev/c/sunxi-tools/sunxi-fel -p version uboot ~/dev/c/uboots/t113-s3/u-boot-sunxi-with-spl.bin  write 0x41000000 '+duck_img)
    # os.shell('~/dev/c/sunxi-tools/sunxi-fel version uboot ~/dev/c/uboots/u-boot-sunxi-with-spl.bin  write 0x41000000 '+duck_fit)
    # os.shell('~/dev/c/sunxi-tools/sunxi-fel version uboot ~/dev/c/uboots/u-boot-sunxi-with-spl.bin  write 0x41000000 '+duck_kernel)

on_run(run)


#f1c200s 运行
target("f1c200s")
# add_deps("duck.fit")
add_deps("uImage.img")

def run(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch=target.get_arch()
    arch_type= target.get_arch_type()
    mode =target.get_config('mode')
    plat=target.plat()

    duck_fit="build/"+plat+"/"+arch+"/"+mode+"/duck.fit"
    duck_kernel="build/"+plat+"/"+arch+"/"+mode+"/kernel.elf"
    duck_img="build/"+plat+"/"+arch+"/"+mode+"/uImage.img"


    print('run '+plat+' fel',duck_kernel)

    os.shell('~/dev/c/sunxi-tools/sunxi-fel -p version uboot ~/dev/c/uboots/f1c200s/u-boot-sunxi-with-spl.bin  write 0x81000000 '+duck_img)
    # os.shell('~/dev/c/sunxi-tools/sunxi-fel -p version uboot ~/dev/c/uboots/f1c200s/u-boot-sunxi-with-spl.bin  write 0x81000000 '+duck_fit)
    # os.shell('~/dev/c/sunxi-tools/sunxi-fel -p version  uboot ~/dev/c/uboots/f1c200s/u-boot-sunxi-with-spl.bin  write 0x81000000 '+duck_kernel)

on_run(run)



#esp32 运行
target("esp32")
# add_deps("duck.fit")
add_deps("uImage.img")

def run(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch=target.get_arch()
    arch_type= target.get_arch_type()
    mode =target.get_config('mode')
    plat=target.plat()

    duck_fit="build/"+plat+"/"+arch+"/"+mode+"/duck.fit"
    duck_kernel="build/"+plat+"/"+arch+"/"+mode+"/kernel"
    duck_img="build/"+plat+"/"+arch+"/"+mode+"/uImage.img"

    duck_kernel_bin="build/"+plat+"/"+arch+"/"+mode+"/kernel.bin"


    print('run '+plat+' fel',duck_kernel)

    os.shell('source /Users/evil/dev/c/esp/esp-idf/export.sh && esptool.py --chip esp32 elf2image --flash_mode="dio" --flash_freq "40m" --flash_size "4MB" -o '+duck_kernel_bin+' '+duck_kernel)

    os.shell('esptool.py --chip esp32 --port /dev/cu.wchusbserial1413100 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000  '+duck_kernel_bin+'  ')


on_run(run)