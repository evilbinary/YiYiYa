#ifndef _BMP_H_
#define _BMP_H_

/******************************************************
* bmp ��ʽ�����ļ�                                    *
* ��ֻ֧�� 16λɫ(5:6:5) ��ѹ�� Windows ��׼ bmp ��ʽ *
******************************************************/

/******************************************************
* ������˵������                                      *
******************************************************/

struct bmp_bmp_info_head_struct{
  int the_struct_size ;
  int width ;
  int height ;
  // ���滹�����ݣ���Ŀǰ pyos ֻ���� 16 λ���λͼ�������������ݲ�Ҫ��
} ;

struct bmp_bmp_head_struct{
  short type ; // ����
  int size_file ; // ��С
  short reserved_0 ;
  short reserved_1 ;
  int offset ; // λͼ���е���ʼλ��
  struct bmp_bmp_info_head_struct info_head ;
} ;

#endif
