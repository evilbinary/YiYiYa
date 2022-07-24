#ifndef REFRESH_H
#define REFRESH_H
#include "defs.h"

void refresh_1(byte *dest, byte *src, byte *pal, int cnt);
void refresh_2(byte *dest, byte *src, un16 *pal, int cnt);
void refresh_3(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_1_2x(byte *dest, byte *src, byte *pal, int cnt);
void refresh_2_2x(byte *dest, byte *src, un16 *pal, int cnt);
void refresh_3_2x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4_2x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_2_3x(byte *dest, byte *src, un16 *pal, int cnt);
void refresh_3_3x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4_3x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_3_4x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4_4x(byte *dest, byte *src, un32 *pal, int cnt);

#endif

