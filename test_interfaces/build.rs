fn main() {
    use cmake::Config;
    let all = Config::new("../")
        .define("DANDELION_PLATFORM", "test")
        .define("FREESTANDING", "ON")
        .define("DANDELION_FS", "ON")
        .build_target("all")
        .build();
    // add the system library
    println!(
        "cargo:rustc-link-search=native={}/build/system",
        all.display()
    );
    println!("cargo:rustc-link-lib=static=dandelion_system");
    // add the runtime library
    println!(
        "cargo:rustc-link-search=native={}/build/runtime",
        all.display()
    );
    println!("cargo:rustc-link-lib=static=dandelion_runtime");
    // add file system library
    println!(
        "cargo:rustc-link-search=native={}/build/file_system",
        all.display()
    );
    println!("cargo:rustc-link-lib=static=dandelion_file_system");
}
