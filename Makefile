all: raspi2


raspi2:
	xmake f -m debug -p raspi2 -v
	xmake

v3s:
	make f -m debug -p v3s -v
	xmake


i386-pc:
	xmake f -m debug -p i386-pc -v
	xmake


img:
	qemu-img create  image/disk.img 512m
	mkfs.vfat image/disk.img 

qemu:
	xmake build qemu

run:qemu
	xmake run qemu


com:
	minicom -D /dev/tty.usbserial 


push:
	cd app && git push
	cd eggs && git push
	cd duck && git push
	cd foot && git push

clean:
	rm -rf build