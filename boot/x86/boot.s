;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Copyright 2016_2080 evilbinary_
;作者:evilbinary on 12/24/16_
;邮箱:rootdebug@163_com
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
org 7c00h
_start:
%define init_base 0x0500
bits 16
	mov ax, cs
	mov ds, ax
	mov ss, ax
	; mov sp, 0
    cli
    call cls

    mov dx,0x0000
    call set_cursor

    mov si,boot
    call print_string

    mov bx,init_base
    call load_init

    mov si,load_init_success
    call print_string

    ;;跳转loader地址
    jmp init_base
    
    jmp $

;;磁盘读取到内存 es:bx 地址
load_init:
    mov ah,0x02 ;读取功能
    mov al,0x0e ;读取几个扇区
    mov cl,0x02 ;0x01 boot sector, 0x02 is first sector
    mov ch,0x00
    mov dh,0x00
    mov dl,0x00 ;软盘0
    int 0x13
    jc disk_error
    jmp dend
disk_error:
    mov si,disk_erro
    call print_string
dend:
    ret

%include "boot/x86/util.s"

boot db "boot duck",0x0a,0x0d,0
disk_erro db "read boot disk erro",0x0a,0x0d,0
load_init_success db "load init success",0x0a,0x0d,0

times 510-($-$$)  db 0
dw 0xaa55
