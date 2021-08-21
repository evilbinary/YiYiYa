#ifndef _BMP_H_
#define _BMP_H_

/******************************************************
* bmp 格式处理文件                                    *
* 现只支持 16位色(5:6:5) 非压缩 Windows 标准 bmp 格式 *
******************************************************/

/******************************************************
* 下面是说明部分                                      *
******************************************************/

struct bmp_bmp_info_head_struct{
  int the_struct_size ;
  int width ;
  int height ;
  // 下面还有数据，但目前 pyos 只处理 16 位真彩位图，因此下面的数据不要了
} ;

struct bmp_bmp_head_struct{
  short type ; // 类型
  int size_file ; // 大小
  short reserved_0 ;
  short reserved_1 ;
  int offset ; // 位图阵列的起始位置
  struct bmp_bmp_info_head_struct info_head ;
} ;

#endif
