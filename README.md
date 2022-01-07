# YiYiYa
YiYiYa is an os

https://github.com/evilbinary/YiYiYa

## 使用

### 编译

提前安装 scons、qemu [安装文档](https://github.com/evilbinary/YiYiYa/tree/main/docs/install)

编译

```
scons
```

### 运行

```
scons -Q init && scons --run=qemu
```


## 开发

具体见文档：[开发文档](https://github.com/evilbinary/YiYiYa/tree/main/docs/develop)
## FAQ

1、在linux下如果运行黑屏失败，修改根目录Sconstruct `env['CC']='/usr/bin/gcc-9'` gcc 改成9版本。

2、如果在linux下运行出现boot duck停住了，则`env['CFLAGS']= env['CFLAGS']+' -no-pie -fno-pic '`修改修改成这个。

3、如果运行出现 ../app/kernel not exist，则image下的disk.img被删除了，需要找回来。





### 截图

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/etk-gui.jpeg?raw=true" width="800px" />

https://user-images.githubusercontent.com/5143386/133922735-972ddceb-eab6-4102-b9a2-2e14e7d86397.mp4


https://user-images.githubusercontent.com/5143386/121841338-acf76f80-cd10-11eb-98fe-f96cb78ba221.mp4





<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/lvgl-gui.png?raw=true" width="800px" />
      
<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/lvgl2.png?raw=true" width="800px" />

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/etk.png?raw=true" width="400px" /><img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/microui.jpg?raw=true" width="400px" />

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/yiyiya.png?raw=true" width="400px" /><img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/shell.png?raw=true" width="400px" />


## 开发

qq群 26893532 可以加入一起讨论学习，如果找不到该群，使用扫码加入。
<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/demo/group.jpeg?raw=true" width="200px" />


https://github.com/evilbinary/YiYiYa


## 参考资料
https://wiki.osdev.org

https://zh.wikipedia.org/wiki/%E8%BD%AF%E7%9B%98

64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf
