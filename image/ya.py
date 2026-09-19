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
    elif arch_type=='arm64':
        # 树莓派 arm64：-a/-e 必须与 boot/arm64/init.h 的 KERNEL_BASE 一致，
        # 否则内核会被加载到错误地址（raspi5 固件就固定从 0x80000 取裸内核）。
        if plat == 'raspi5':
            origin = '0x80000'
        else:
            origin = '0x100000'
        cmd='mkimage -n YiYiYa -A arm64 -O u-boot -T kernel -C none -a '+origin+' -e '+origin+' -d '+sourcefiles[0]+' '+targetfile
        os.exec(cmd)

        # 树莓派固件是按【文件名】找内核的，内容都是同一份裸二进制（kernel.bin，无
        # 任何头，与 uImage.img 不同）。真机日志（Pi5，config.txt 未写 kernel=）：
        #   Loading 'kernel_2712.img' ...  Read kernel_2712.img bytes 465104
        #   MESS: Kernel relocated to 0x80000
        # → Pi5 固件按 kernel_2712.img 找，并重定位到 0x80000，与
        #   boot/arm64/init.h 的 KERNEL_BASE(raspi5=0x80000) 一致。
        # raspi3 走 Pi3 的 64 位固件默认名 kernel8.img。
        build_dir = targetfile.rsplit('/', 1)[0]
        sd_names = ['kernel_2712.img'] if plat == 'raspi5' else ['kernel8.img']
        for sd_name in sd_names:
            sd_kernel = build_dir + '/' + sd_name
            os.cp(sourcefiles[0], sd_kernel)
            print('generated '+sd_kernel)

        # 树莓派固件读 SD 卡 boot 分区根目录的 config.txt（与内核放同一目录）
        write_pi_config(build_dir + '/config.txt', plat)

on_build(build)


def build_esp32_img(duck_kernel,duck_kernel_bin,kernel_bin_img):
    os.shell('esptool.py --chip esp32 --trace elf2image --version 3 --min-rev 1  --flash_mode dio  --flash_freq "40m" --flash_size "16MB" -o '+duck_kernel_bin+' '+duck_kernel)

    os.exec('dd if='+duck_kernel_bin+' of='+kernel_bin_img+' bs=1 seek=4k')
    #os.exec('truncate -s 16K '+kernel_bin_img)
    os.exec('truncate -s 4M '+kernel_bin_img)

#run
def add_qemu_deps():
    add_deps("duck.img", "disk.img")
    if has_config('single-kernel'):
        add_deps("kernel.elf")

def build_disk_img(target):
    disk_img = target.targetfile()
    if os.exists(disk_img):
        return
    print('make disk.img')
    os.exec('qemu-img create ' + disk_img + ' 512m')
    if is_host('mac') or is_host('linux'):
        os.exec('mkfs.vfat -n YIYIYA ' + disk_img)
    else:
        os.exec('mformat.exe -i ' + disk_img + ' -n YIYIYA ::')

def write_pi_config(cfg, plat):
    """生成树莓派 SD 卡用的 config.txt（幂等，不需要先删文件）。

    - 内容与模板一致           → 什么都不做；
    - 内容不同但首行是本脚本标记 → 直接改写（调整模板后无需删文件）；
    - 首行不是本脚本标记         → 认为是你自己的 config.txt，完全不碰。
    """
    if plat == 'raspi5':
        cfg_lines = [
            '# YiYiYa - Raspberry Pi 5 (BCM2712, AArch64)',
            '# 拷贝到 SD 卡 boot 分区根目录（与 kernel_2712.img 同目录）',
            '# Pi5 固件默认就找 kernel_2712.img（真机日志证实），这里显式写出',
            'arm_64bit=1',
            'kernel=kernel_2712.img',
            '# 固件会把内核重定位到 0x80000，与 boot/arm64/init.h 的',
            '# KERNEL_BASE(raspi5=0x80000) 一致；显式写出便于核对',
            'kernel_address=0x80000',
            '# 让固件使能 RP1 的 UART0 并配好 GPIO14/15（内核 uart_send 直接写',
            '# RP1 窗口内的 UART0，引脚复用依赖固件完成）',
            'enable_uart=1',
        ]
    else:
        cfg_lines = [
            '# YiYiYa - Raspberry Pi 3 (BCM2837, AArch64)',
            '# 拷贝到 SD 卡 boot 分区根目录（与 kernel8.img 同目录）',
            'arm_64bit=1',
            'kernel=kernel8.img',
            '# 必须与 boot/arm64/init.h 的 KERNEL_BASE 一致：raspi3=0x100000',
            '# （树莓派 64 位固件默认 0x80000，不写这行会加载到错误地址）',
            'kernel_address=0x100000',
            '# 让固件使能 UART0 并固定 UART 时钟：驱动保留固件设好的分频',
            '# （见 duck/platform/raspi3/init.c "typical 48MHz UARTCLK"）',
            'enable_uart=1',
        ]
    content = '\n'.join(cfg_lines) + '\n'
    if os.exists(cfg):
        f = open(cfg, 'r')
        old = f.read()
        f.close()
        if old == content:
            print('config.txt up to date: '+cfg)
            return
        if not old.startswith('# YiYiYa - '):
            print('config.txt kept (looks hand-written): '+cfg)
            return
        f = open(cfg, 'w')
        f.write(content)
        f.close()
        print('config.txt updated: '+cfg)
        return
    f = open(cfg, 'w')
    f.write(content)
    f.close()
    print('generated '+cfg)


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
        kernel_bin="build/"+plat+"/"+arch+"/"+mode+"/duck.img"
        disk_img="image/disk.img"
        
        kernel_elf="build/"+plat+"/"+arch+"/"+mode+"/kernel.elf"
        if os.exists(kernel_elf):
            os.cp(kernel_elf, 'app/resource/kernel.elf')
        
        if is_host('mac'):
            os.exec('hdiutil attach '+disk_img)
            os.cp('app/resource/*', '/Volumes/YIYIYA/')
            os.exec('hdiutil eject /Volumes/YIYIYA')
        # 【不要加 -n（no-overwrite）】这里必须覆盖：raspi2/raspi3/raspi5 的引导阶段
        # 是"从 SD 镜像里读 kernel.elf 再跳过去执行"，而镜像里同名文件一旦存在：
        #   · 带 -n ⇒ mcopy 静默跳过 ⇒ 镜像里永远是【第一次】拷进去的那份内核；
        #   · 于是"改代码 → 重新 build → 跑 QEMU"实际仍在跑旧内核，
        #     表现为"某个提交之后就一直挂"（实测：所有提交都停在完全相同的 637 字节，
        #     换提交、换代码都毫无变化 —— 就是同一个旧内核在被反复启动）。
        # app/resource/* 都是构建产物，覆盖是正确语义。
        elif is_host('linux') :
            os.shell('mcopy -smo  -i image/disk.img app/resource/* ::')
        else:
            os.exec('mcopy.exe -smo  -i image/disk.img app/resource/* ::')
        

        run_qemu_cmd=''
        debug_qemu_cmd=''
  
        if has_config('single-kernel') :
            kernel_image= "build/"+plat+"/"+arch+"/"+mode+"/kernel.elf"
            kernel_bin= "build/"+plat+"/"+arch+"/"+mode+"/kernel.bin"

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
                run_qemu_cmd='qemu-system-aarch64 -name YiYiYa -M raspi3b  -rtc base=localtime -kernel '+kernel_image+'  -serial stdio -device usb-mouse   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' ' # -d in_asm -d cpu_reset -d in_asm,int,mmu
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
                run_qemu_cmd='qemu-system-arm -name YiYiYa -M raspi2b  -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' '##-d in_asm -d cpu_reset -d in_asm,int,mmu 
                ## run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                ## run_qemu_cmd =run_qemu_cmd+' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd +' -S -s'
            
        elif arch_type =='arm64' :

            if target.plat() == 'raspi3' :
                run_qemu_cmd='qemu-system-aarch64 -name YiYiYa -M raspi3b -device usb-mouse -rtc base=localtime -kernel '+kernel_image+'  -serial stdio   -D ./qemu.log -drive if=sd,id=sd0,format=raw,file='+disk_img+' ' # -d in_asm -d cpu_reset -d in_asm,int,mmu
                #run_qemu_cmd =run_qemu_cmd+' -monitor tcp:127.0.0.1:55555,server,nowait'
                # run_qemu_cmd =run_qemu_cmd+' -chardev socket,id=monitor,path=monitor.sock,server,nowait -monitor chardev:monitor'
                debug_qemu_cmd = run_qemu_cmd +' -S -s'
            elif target.plat() == 'raspi5' :
                # QEMU does not support BCM2712 yet, boot from real SD card.
                # Generate kernel_2712.img for the Pi5 firmware.
                kernel_bin = "build/"+plat+"/"+arch+"/"+mode+"/kernel.bin"
                sd_kernel = "build/"+plat+"/"+arch+"/"+mode+"/kernel_2712.img"
                if os.exists(kernel_bin):
                    os.cp(kernel_bin, sd_kernel)
                    print('raspi5: generated '+sd_kernel)
                # 同目录一并产出 config.txt（已存在则不覆盖）
                write_pi_config(sd_kernel.rsplit('/', 1)[0] + '/config.txt', 'raspi5')
                cprint('${green}raspi5: QEMU has no BCM2712 machine, boot on real hardware:${clear}')
                cprint('  1. format an SD card with FAT32')
                cprint('  2. copy '+sd_kernel+' as kernel_2712.img to the SD card')
                cprint('  3. copy config.txt (same dir) to the SD card boot partition')
                cprint('  4. serial console on GPIO14/15 (UART0) at 115200 8N1')
                return
            else:
                print('no support arm64 platform:', target.plat())
            
        elif arch_type=='xtensa' :
            kernel_bin_img=kernel_bin+'.img'

            build_esp32_img(kernel_image,kernel_bin,kernel_bin_img)

            run_qemu_cmd='~/dev/qemu-esp32/build/qemu-system-xtensa -M esp32  --trace "*mtd*" -serial stdio -monitor telnet:localhost:1235,server,nowait -drive file='+kernel_bin_img+',if=mtd,format=raw -m 4M ' #-d in_asm -d cpu_reset -d in_asm,int,mmu
            run_qemu_cmd+=' -d cpu_reset -d in_asm,int,mmu -D ./qemu.log'

            debug_qemu_cmd = run_qemu_cmd +'  -S -s'

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
            os.shell(debug_qemu_cmd)
        else:
            cprint('${green}run qemu ${clear} %s'%run_qemu_cmd)
            os.shell(run_qemu_cmd)
        

    # print('qmeu=>',os.arch() ,plat )
    
    on_run(run)




target("debug")

add_qemu_deps()
add_rules("arch")

plat=get_plat()
if not plat:
    plat='raspi2'


run_qemu(plat,True)



target("qemu")

add_qemu_deps()
add_rules("arch")

plat=get_plat()
if not plat:
    plat='raspi2'

run_qemu(plat)


target("raspi2")

add_qemu_deps()
add_rules("arch")
run_qemu('raspi2')


target("raspi3")

add_qemu_deps()
add_deps("uImage.img")
add_rules("arch")
run_qemu('raspi3')


target("raspi3-debug")

add_qemu_deps()
add_deps("uImage.img")
add_rules("arch")

plat=get_plat()
if not plat:
    plat='raspi3'


run_qemu(plat,True)


target("raspi5")

add_qemu_deps()
add_deps("uImage.img")
add_rules("arch")
run_qemu('raspi5')


target("raspi5-debug")

add_qemu_deps()
add_deps("uImage.img")
add_rules("arch")

plat=get_plat()
if not plat:
    plat='raspi5'


run_qemu(plat,True)


target("disk.img", **{'build-dir': '.'})
set_filename('image/disk.img')

on_build(build_disk_img)


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
add_deps("kernel.img")

def run(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch=target.get_arch()
    arch_type= target.get_arch_type()
    mode =''+str(target.get_config('mode'))
    plat=target.plat()

    # duck_fit="build/"+plat+"/"+arch+"/"+mode+"/duck.fit"
    duck_kernel="build/"+plat+"/"+arch+"/"+mode+"/kernel"

    kernel_bin_img="build/"+plat+"/"+arch+"/"+mode+"/kernel.bin.img"

    duck_kernel_bin="build/"+plat+"/"+arch+"/"+mode+"/kernel.bin"



    print('run '+plat+' fel',duck_kernel)

    build_esp32_img(duck_kernel,duck_kernel_bin,kernel_bin_img)


    os.shell('esptool.py --chip esp32 --port /dev/cu.wchusbserial141400 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000  '+duck_kernel_bin+'  ')


on_run(run)


#stm32f407 运行
target("stm32f4xx")
add_deps("kernel.elf")

def run(target):
    targetfile = target.targetfile()
    sourcefiles = target.sourcefiles()
    arch=target.get_arch()
    arch_type= target.get_arch_type()
    mode =target.get_config('mode')
    plat=target.plat()

    
    duck_kernel="build/"+plat+"/"+arch+"/"+mode+"/kernel"
    duck_kernel_bin="build/"+plat+"/"+arch+"/"+mode+"/kernel.bin"




    os.shell(' arm-none-eabi-objcopy -O binary '+duck_kernel+'  '+duck_kernel_bin+' ')

    print('run '+plat+' fel',duck_kernel_bin)


    os.shell('dfu-util -a 0 -s 0x08000000:leave  -D '+duck_kernel_bin+'  ')


on_run(run)