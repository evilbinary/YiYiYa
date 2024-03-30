.section ".text.boot"
.org 0x7c00
_start:
.code16gcc
.equ init_base, 0x0500

    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %ss    
	// mov sp, 0
    cli
    call cls

    mov $0x0000, %dx
    call set_cursor

    mov $boot, %si
    call print_string

    mov $init_base, %bx
    call load_init

    mov $load_init_success, %si
    call print_string

    //跳转loader地址
    jmp *%bx
    
jmp_end:
    jmp jmp_end

//磁盘读取到内存 es:bx 地址
load_init:
    mov $0x02, %ah //读取功能
    mov $0x0e, %al //读取几个扇区
    mov $0x02, %cl //0x01 boot sector, 0x02 is first sector
    xor %ch, %ch
    xor %dh, %dh
    xor %dl, %dl //软盘0
    int $0x13
    jc disk_error
    jmp dend

disk_error:
    mov $disk_erro, %si
    call print_string

dend:
    ret

// Set cursor position (DH=column, DL=row)
set_cursor:
    mov $0x02, %ah      // Cursor position initialization
    xor %bh, %bh
    int $0x10
    ret

// Reset cursor
reset:
    mov $0x00, %ah
    mov $0x01, %al
    int $0x10
    ret

// Clear screen
cls:
    mov $0x06, %ah      // Clear screen
    xor %al, %al
    xor %cx, %cx
    mov $0xffff, %dx
    mov $0x0f, %bh      // Attribute for white text
    int $0x10
    ret

// Print a character (al=ASCII value)
print_char:
    mov $0x0e, %ah
    mov $0x0007, %bx
    int $0x10
    ret

// Print a string (si=address of the string)
print_string:
    ps:
    mov (%si), %al
    inc %si
    or %al, %al
    jz pend
    call print_char
    jmp ps
    pend:
    ret
    

boot:
    .ascii "boot duck\0"
    .byte 0x0a, 0x0d, 0x00

disk_erro:
    .ascii "read boot disk error\0"
    .byte 0x0a, 0x0d, 0x00

load_init_success:
    .ascii "load init success\0"
    .byte 0x0a, 0x0d, 0x00

.fill 510-(.-_start), 1, 0
.word 0xaa55