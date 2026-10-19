fn main() {
    // A build of a branch names its run (lib.rs, LAUNCHER_BUILD).
    println!("cargo:rerun-if-env-changed=OPENRAC_LAUNCHER_BUILD");
    tauri_build::build()
}
