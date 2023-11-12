all: raspi2


raspi2:
	ya -m debug -r qemu

v3s:
	ya -m debug -r v3s -p v3s


i386-pc:
	ya -m debug -r qemu -p i386-pc

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

pull:
	git submodule update --init --recursive

clean:
	rm -rf build