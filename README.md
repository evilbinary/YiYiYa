# YiYiYa

[YiYiYa](https://github.com/evilbinary/YiYiYa) is an os.

## 使用

### 编译

提前安装

1. scons
2. qemu 

编译

```shell
scons
```


### 运行镜像


```shell
scons --run=qemu
```


## FAQ

1. 在 Linux 下如果运行黑屏失败，修改根目录 Sconstruct `env['CC']='/usr/bin/gcc-9'` gcc 改成 9 版本；
2. 如果在 Linux 下运行出现 boot duck 停住了，则 `env['CFLAGS']= env['CFLAGS']+' -no-pie -fno-pic '` 修改修改成这个；
3. 如果运行出现 `../app/kernel not exist` ，则 `image` 下的 `disk.img` 被删除了，需要找回来。

## 截图

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/etk-gui.jpeg?raw=true" width="800px" />

https://user-images.githubusercontent.com/5143386/133922735-972ddceb-eab6-4102-b9a2-2e14e7d86397.mp4


https://user-images.githubusercontent.com/5143386/121841338-acf76f80-cd10-11eb-98fe-f96cb78ba221.mp4


<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/lvgl-gui.png?raw=true" width="800px" />
      
<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/lvgl2.png?raw=true" width="800px" />

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/etk.png?raw=true" width="400px" /><img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/microui.jpg?raw=true" width="400px" /><img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/yiyiya.png?raw=true" width="400px" />


## 开发

- QQ Group：26893532
- 可以加入一起讨论学习，如果找不到该群，使用扫码加入

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/group.jpeg?raw=true" width="200px" />

https://github.com/evilbinary/YiYiYa


## 致谢

- [OSDev Wiki](https://wiki.osdev.org/Main_Page)
- [软盘 - 维基百科，自由的百科全书](https://zh.wikipedia.org/wiki/%E8%BD%AF%E7%9B%98)
- [Intel® 64 and IA-32 Architectures Developer's Manual: Vol. 3A](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html)
