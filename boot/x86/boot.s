;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Copyright 2016_2080 evilbinary_
;作者:evilbinary on 12/24/16_
;邮箱:rootdebug@163_com
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
org 7c00h
_start:
%define init_base 0x0500
bits 16
    ; 寄存器复位
    mov ax, cs
    mov ds, ax
    mov ss, ax
    ; mov sp, 0
    
    ; 屏蔽所有外部中断
    cli
    
    ; 清除屏幕
    call cls
    
    ; 初始化光标
    mov dx,0x0000
    call set_cursor
    
    ; 打印boot字符串
    mov si,boot
    call print_string
    
    ; 读取磁盘
    mov bx,init_base
    call load_init
    
    ; 读取成功
    mov si,load_init_success
    call print_string

    ; 跳转到loader
    jmp init_base
    
    jmp $

; int 13h ah=02h 读取磁盘扇区 al=读入的扇区数 ch=磁道号的低8位 cl=扇区号1~63(bit 0~5)磁道号,(柱面号)的高2位(bit 6~7,只对硬盘有效) dh=磁头号 dl=驱动器号 es:bx=>数据缓冲区
; 根目录占用空间 = 14(详细请查看fat12相关信息)
load_init:
    mov ah,0x02
    mov al,0x0e
    mov cl,0x02
    mov ch,0x00
    mov dh,0x00
    mov dl,0x00
    int 0x13
    jc disk_error
    jmp dend
disk_error:
    mov si,disk_erro
    call print_string
dend:
    ret

%include "boot/x86/util.s"

; 字符串定义(0x0a,0x0d,0表结束符)
boot db "boot duck",0x0a,0x0d,0
disk_erro db "read boot disk erro",0x0a,0x0d,0
load_init_success db "load init success",0x0a,0x0d,0

; 补齐至512
times 510-($-$$)  db 0

; boot结束标志
dw 0xaa55
