use std::process::Command;
use std::env;
use std::path::Path;

fn main() {
    let out_dir = env::var("OUT_DIR").unwrap();

    // Command::new("gcc").args(&["src/hello.c", "-c", "-fPIC", "-o"])
    //                    .arg(&format!("{}/hello.o", out_dir))
    //                    .status().unwrap();
    // Command::new("ar").args(&["crus", "libhello.a", "hello.o"])
    //                   .current_dir(&Path::new(&out_dir))
    //                   .status().unwrap();
    println!("out dir {}",out_dir);

    // println!("cargo:rustc-link-search=native=../../../libs/libc/");
    // println!("cargo:rustc-link-lib=static=c");

    // println!("cargo:rustc-link-search=native=../../libmusl/lib/");
    // println!("cargo:rustc-link-lib=static=c");
}