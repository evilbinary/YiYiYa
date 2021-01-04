;;设置光标位置  DH=列，DL=行
set_cursor:
    mov ah,0x02 ;光标位置初始化
    mov bh,0
    int 0x10
    ret
reset:
    mov ah,0x00
    mov al,0x01
    int 0x10
    ret

;;清除屏幕 
cls:
    mov ah,0x06   ;清除屏幕					
    mov al,0
    mov cx,0   
    mov dx,0xffff  
    mov bh,0x0f ;属性为白字
    int 0x10
    ret

;;打印一个字符 al=ascii值
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