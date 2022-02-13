; 设置光标位置
; int 10h ah=02h表设定光标位置 dh=游标的列数 dl=游标的行数 bh=页码
set_cursor:
    mov ah,0x02
    mov bh,0
    int 0x10
    ret
    
reset:
    mov ah,0x00
    mov al,0x01
    int 0x10
    ret

; 清除屏幕
; int 10h ah=06h表滚动窗口 al=滚动的列数，若为0则实现清空屏幕功能
cls:
    mov ah,0x06			
    mov al,0
    mov cx,0
    mov dx,0xffff  
    mov bh,0x0f
    int 0x10
    ret

; 打印一个字符
; int 10h ah=0eh al=打印字符 bh＝页码 bl=前景色
print_char:
    mov ah,0eh
    mov bx,0007h
    int 0x10
    ret

;;打印字符串 si=字符地址
print_string:
    ps:
    mov al,[si]
    inc si
    or al,al
    jz pend  
    call print_char      
    jmp ps
    pend:
    ret
