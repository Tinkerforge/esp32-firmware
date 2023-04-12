fn main() {
    cxx_build::bridge("src/lib.rs")
        .file("src/test.cpp")
        .compile("rust_example");
}