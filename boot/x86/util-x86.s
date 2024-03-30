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
