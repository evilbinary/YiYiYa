all: raspi2

lib:
	scons eggs/libmusl/ -j8
	scons eggs/libffmpeg -j8

raspi2: lib
	scons platform=raspi2 -j8

v3s: lib
	scons platform=v3s -j8 
	scons run=q


i386-pc: lib
	scons platform=i386-pc -j8
	scons run=q


clean:
	scons -c