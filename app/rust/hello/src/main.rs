#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[no_mangle]
pub unsafe fn _start() {
    main();
}


extern "C" {
    fn printf(c: *const i8, ...) -> i32;
}



fn print(str:& str){
    unsafe {
        printf(str.as_ptr() as *const i8);
    }
}

fn main() {
    print("Hello, world! Rust!");
}
