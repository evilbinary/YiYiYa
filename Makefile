all: raspi2


raspi2:
	ya -m debug -r qemu

v3s:
	ya -m debug -r v3s -p v3s

raspi3:
	ya -m debug -p raspi3  -r raspi3

i386-pc:
	ya -m debug -r qemu -p i386-pc

dump-raspi3:
	aarch64-none-elf-objdump -D build/raspi3/armv8-a/debug/kernel.elf >kernel.txt.c

esp32:
	ya -m debug -p esp32 -r esp32

clean-esp32:
	rm -rf build/esp32/lx6/debug/objs/
stm32:
	ya -m debug -p stm32f4xx -r stm32f4xx -app
clean-stm32:
	rm -rf build/stm32f4xx/armv7/debug/objs/
dump-stm32:
	arm-none-eabi-objdump -D build/stm32f4xx/armv7/debug/kernel.elf >kernel.txt.c
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
