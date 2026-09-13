all: raspi2


raspi2:
	ya -m debug -r qemu

raspi2-d:
	arm-none-eabi-objdump -D build/raspi2/armv7-a/debug/kernel.elf >kernel.txt.c
	arm-none-eabi-objdump -D build/raspi2/armv7-a/debug/ls >ls.txt.c
	arm-none-eabi-objdump -D build/raspi2/armv7-a/debug/config >config.txt.c
v3s:
	ya -m debug -r v3s -p v3s

raspi3:
	ya -m debug -p raspi3  -r raspi3

i386-pc:
	ya -m debug -r qemu -p i386-pc

raspi3-d:
	aarch64-none-elf-objdump -D build/raspi3/armv8-a/debug/kernel.elf >kernel.txt.c
	aarch64-none-elf-objdump -D build/raspi3/armv8-a/debug/ls >ls.txt.c

raspi5:
	ya -m debug -p raspi5 -r raspi5

raspi5-d:
	aarch64-none-elf-objdump -D build/raspi5/armv8-a/debug/kernel.elf >kernel.txt.c

raspi3-ls-d:
	aarch64-none-elf-objdump -D build/raspi3/armv8-a/debug/ls  >ls.txt.c
esp32:
	ya -m debug -p esp32 -r esp32

esp32-c:
	rm -rf build/esp32/lx6/debug/objs/duck

esp32-d:
	xtensa-esp32-elf-objdump -D build/esp32/lx6/debug/kernel.elf >kernel.txt.c

stm32:
	ya -m debug -p stm32f4xx -r stm32f4xx -app
stm32-c:
	rm -rf build/stm32f4xx/armv7/debug/objs/
stm32-d:
	arm-none-eabi-objdump -D build/stm32f4xx/armv7/debug/kernel.elf >kernel.txt.c


riscv-virt:
	ya -m debug -p riscv-virt -r qemu

t113-s3:
	ya -m debug -p t113-s3 -b t113-s3
	
img:
	qemu-img create  image/disk.img 512m
	mkfs.vfat -n YIYIYA image/disk.img 

duck.img:
	ya -m debug -b duck.img

qemu: 
	ya -m debug -b qemu

run: duck.img
	ya -m debug -r qemu


com:
	minicom -D /dev/tty.usbserial 


push:
	cd app && git push
	cd eggs && git push
	cd duck && git push
	cd foot && git push
	git push
	
pullm:
	cd app &&git checkout main && git pull
	cd eggs &&git checkout main && git pull
	cd duck &&git checkout main && git pull
	cd foot &&git checkout main && git pull
	git pull
pull:
	git submodule update --init --recursive

clean:
	rm -rf build
