fn main() {
    // A build of a branch names its run (lib.rs, LAUNCHER_BUILD).
    println!("cargo:rerun-if-env-changed=OPENRAC_LAUNCHER_BUILD");
    // tauri-build copies the bundle's resources next to the program in target/, for `tauri dev`;
    // an installer's resources are the whole copy of OpenRAC (resources/openrac: the games, a Python
    // and the tools, tools/bundle_standalone.py), which the bundler takes from resources/ itself.
    // A release build copied them all again, and watched every one of them for changes, each time
    // it built: they are left out of what tauri-build sees there.
    if std::env::var("PROFILE").as_deref() == Ok("release") {
        let mut config = std::env::var("TAURI_CONFIG")
            .ok()
            .and_then(|patch| serde_json::from_str(&patch).ok())
            .unwrap_or_else(|| serde_json::json!({}));
        if let Some(fields) = config.as_object_mut() {
            let bundle = fields.entry("bundle").or_insert_with(|| serde_json::json!({}));
            if let Some(bundle) = bundle.as_object_mut() {
                // null takes the key away in the merge (RFC 7396): no resources at all.
                bundle.insert("resources".into(), serde_json::Value::Null);
            }
        }
        std::env::set_var("TAURI_CONFIG", config.to_string());
    }
    tauri_build::build()
}
