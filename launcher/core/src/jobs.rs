//! Running actions. A job is a child process whose output lines go to a
//! sink as [`Event`]s (the app forwards them to the UI as `job-output` and
//! `job-exit`); a detached launch (the game, Godot) is started and left
//! alone.

use std::collections::{HashMap, HashSet};
use std::io::{BufRead, BufReader, Read};
use std::process::{Child, Command, Stdio};
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use serde::Serialize;

use crate::actions::Plan;

#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase", tag = "type")]
pub enum Event {
    Output { id: u32, stream: Stream, line: String },
    Exit { id: u32, code: Option<i32>, cancelled: bool },
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "camelCase")]
pub enum Stream {
    Stdout,
    Stderr,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Started {
    /// 0 for a detached launch, which has no job.
    pub id: u32,
    pub title: String,
    /// The command line, for the Tasks page and bug reports.
    pub command: String,
    pub detached: bool,
}

/// The running jobs. Cheap to clone; clones share the same jobs.
#[derive(Clone, Default)]
pub struct Jobs {
    inner: Arc<Inner>,
}

#[derive(Default)]
struct Inner {
    next: AtomicU32,
    running: Mutex<HashMap<u32, Arc<Mutex<Child>>>>,
    cancelled: Mutex<HashSet<u32>>,
}

/// What every job gets in its environment: unbuffered, UTF-8 Python output,
/// and the checkout, for tools that want it.
fn environment(cmd: &mut Command, root: Option<&std::path::Path>) {
    cmd.env("PYTHONUNBUFFERED", "1").env("PYTHONIOENCODING", "utf-8");
    if let Some(root) = root {
        cmd.env("OPENRAC_ROOT", root);
    }
    // AppImageKit's AppRun unconditionally exports PYTHONHOME=$APPDIR/usr/ and
    // PYTHONPATH=$APPDIR/usr/share/pyshared/:, which poisons any Python process
    // started from the AppImage (both the carried python in resources/openrac/python
    // and any system python). Clear them before running any tool.
    if let Ok(appdir) = std::env::var("APPDIR") {
        if std::env::var("PYTHONHOME").is_ok_and(|h| h.starts_with(&appdir)) {
            cmd.env_remove("PYTHONHOME");
        }
        if std::env::var("PYTHONPATH").is_ok_and(|p| p.starts_with(&appdir)) {
            cmd.env_remove("PYTHONPATH");
        }
    }
}

impl Jobs {
    /// Starts `plan` (a detached one through [`launch`]). `sink` gets every
    /// output line and, last, the exit; it is called from other threads.
    pub fn start(
        &self,
        plan: &Plan,
        root: Option<&std::path::Path>,
        sink: impl Fn(Event) + Send + Sync + 'static,
    ) -> Result<Started, String> {
        if plan.detached {
            return launch(plan, root);
        }
        let mut cmd = crate::detect::quiet(&plan.program);
        cmd.args(&plan.args).current_dir(&plan.cwd).stdin(Stdio::null()).stdout(Stdio::piped()).stderr(Stdio::piped());
        environment(&mut cmd, root);
        cmd.envs(plan.env.iter().map(|(k, v)| (k, v)));
        own_group(&mut cmd);
        let mut child = cmd.spawn().map_err(|e| format!("cannot start {}: {e}", plan.program.display()))?;

        let id = self.inner.next.fetch_add(1, Ordering::Relaxed) + 1;
        let sink = Arc::new(sink);
        let mut pumps = Vec::new();
        if let Some(out) = child.stdout.take() {
            pumps.push(pump(id, Stream::Stdout, out, sink.clone()));
        }
        if let Some(err) = child.stderr.take() {
            pumps.push(pump(id, Stream::Stderr, err, sink.clone()));
        }
        let child = Arc::new(Mutex::new(child));
        self.inner.running.lock().unwrap().insert(id, child.clone());

        let inner = self.inner.clone();
        std::thread::spawn(move || {
            // Poll rather than wait(), so cancel() can take the lock and kill.
            let status = loop {
                match child.lock().unwrap().try_wait() {
                    Ok(Some(status)) => break Some(status),
                    Ok(None) => {}
                    Err(_) => break None,
                }
                std::thread::sleep(Duration::from_millis(100));
            };
            for p in pumps {
                let _ = p.join();
            }
            inner.running.lock().unwrap().remove(&id);
            let cancelled = inner.cancelled.lock().unwrap().remove(&id);
            sink(Event::Exit { id, code: status.and_then(|s| s.code()), cancelled });
        });
        Ok(Started { id, title: plan.title.clone(), command: command_line(plan), detached: false })
    }

    /// Stops job `id` and everything it started (a build runs compilers in
    /// containers). A job that has already ended is not an error.
    pub fn cancel(&self, id: u32) -> Result<(), String> {
        let Some(child) = self.inner.running.lock().unwrap().get(&id).cloned() else { return Ok(()) };
        self.inner.cancelled.lock().unwrap().insert(id);
        let mut child = child.lock().unwrap();
        kill_tree(&mut child)
    }

    pub fn running(&self) -> Vec<u32> {
        let mut ids: Vec<u32> = self.inner.running.lock().unwrap().keys().copied().collect();
        ids.sort_unstable();
        ids
    }
}

/// Starts `plan` on its own: its output is not read and the launcher does
/// not wait for it.
pub fn launch(plan: &Plan, root: Option<&std::path::Path>) -> Result<Started, String> {
    let mut cmd = Command::new(&plan.program);
    cmd.args(&plan.args).current_dir(&plan.cwd).stdin(Stdio::null()).stdout(Stdio::null()).stderr(Stdio::null());
    environment(&mut cmd, root);
    cmd.envs(plan.env.iter().map(|(k, v)| (k, v)));
    let mut child = cmd.spawn().map_err(|e| format!("cannot start {}: {e}", plan.program.display()))?;
    // Reap it when it ends, so it never lingers as a zombie on Unix.
    std::thread::spawn(move || child.wait());
    Ok(Started { id: 0, title: plan.title.clone(), command: command_line(plan), detached: true })
}

/// The command line as a person would type it, for display only.
pub fn command_line(plan: &Plan) -> String {
    let quote = |s: &str| {
        if s.is_empty() || s.contains([' ', '"', '\'']) {
            format!("\"{}\"", s.replace('"', "\\\""))
        } else {
            s.to_string()
        }
    };
    std::iter::once(plan.program.display().to_string())
        .chain(plan.args.iter().cloned())
        .map(|s| quote(&s))
        .collect::<Vec<_>>()
        .join(" ")
}

#[cfg(unix)]
fn own_group(cmd: &mut Command) {
    use std::os::unix::process::CommandExt;
    cmd.process_group(0);
}

#[cfg(not(unix))]
fn own_group(_: &mut Command) {}

#[cfg(unix)]
fn kill_tree(child: &mut Child) -> Result<(), String> {
    // The job leads its own process group (own_group): signal all of it.
    let group = format!("-{}", child.id());
    let killed = Command::new("kill").args(["-TERM", "--", &group]).status().is_ok_and(|s| s.success());
    if killed {
        return Ok(());
    }
    child.kill().map_err(|e| e.to_string())
}

#[cfg(windows)]
fn kill_tree(child: &mut Child) -> Result<(), String> {
    let mut kill = crate::detect::quiet(std::path::Path::new("taskkill"));
    kill.args(["/T", "/F", "/PID", &child.id().to_string()]);
    if crate::detect::run(kill, Duration::from_secs(10)).is_ok() {
        return Ok(());
    }
    child.kill().map_err(|e| e.to_string())
}

/// Sends each line of `stream` to `sink`. Tools redraw progress with `\r`,
/// so that splits lines too; terminal colours are removed.
fn pump(
    id: u32,
    stream: Stream,
    reader: impl Read + Send + 'static,
    sink: Arc<impl Fn(Event) + Send + Sync + 'static>,
) -> std::thread::JoinHandle<()> {
    std::thread::spawn(move || {
        let mut reader = BufReader::new(reader);
        let mut buf = Vec::new();
        loop {
            buf.clear();
            match reader.read_until(b'\n', &mut buf) {
                Ok(0) | Err(_) => break,
                Ok(_) => {
                    let text = String::from_utf8_lossy(&buf);
                    for line in text.trim_end_matches(['\r', '\n']).split('\r') {
                        sink(Event::Output { id, stream, line: plain(line) });
                    }
                }
            }
        }
    })
}

/// `line` without ANSI escape sequences (colours, cursor moves, titles).
pub fn plain(line: &str) -> String {
    let mut out = String::with_capacity(line.len());
    let mut chars = line.chars().peekable();
    while let Some(c) = chars.next() {
        if c != '\x1b' {
            out.push(c);
            continue;
        }
        match chars.next() {
            // CSI: parameters and intermediates, ended by a byte from @ to ~.
            Some('[') => {
                for c in chars.by_ref() {
                    if ('@'..='~').contains(&c) {
                        break;
                    }
                }
            }
            // OSC: ended by BEL or by ESC \.
            Some(']') => {
                while let Some(c) = chars.next() {
                    if c == '\x07' {
                        break;
                    }
                    if c == '\x1b' {
                        chars.next();
                        break;
                    }
                }
            }
            // Two-byte and charset escapes (ESC ( B): skip intermediates and the final byte.
            Some(c) if (' '..='/').contains(&c) => {
                while chars.next_if(|c| (' '..='/').contains(c)).is_some() {}
                chars.next();
            }
            _ => {}
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;
    use std::sync::mpsc;

    #[test]
    fn plain_removes_escapes() {
        assert_eq!(plain("\x1b[1;32mok\x1b[0m done"), "ok done");
        assert_eq!(plain("\x1b]0;title\x07after \x1b(Bé"), "after é");
        assert_eq!(plain("nothing to strip"), "nothing to strip");
    }

    #[test]
    fn command_lines_quote_what_needs_it() {
        let plan = Plan {
            title: "t".into(),
            program: "python3".into(),
            args: vec!["tools/openrac.py".into(), "my disc.iso".into(), "".into()],
            cwd: ".".into(),
            detached: false,
            env: Vec::new(),
        };
        assert_eq!(command_line(&plan), "python3 tools/openrac.py \"my disc.iso\" \"\"");
    }

    #[cfg(unix)]
    fn sh(script: &str) -> Plan {
        Plan {
            title: "sh".into(),
            program: PathBuf::from("/bin/sh"),
            args: vec!["-c".into(), script.into()],
            cwd: std::env::temp_dir(),
            detached: false,
            env: Vec::new(),
        }
    }

    #[cfg(unix)]
    #[test]
    fn streams_output_then_the_exit() {
        let jobs = Jobs::default();
        let (tx, rx) = mpsc::channel();
        let tx = Mutex::new(tx);
        let started = jobs
            .start(&sh("echo one; printf 'a\\rb\\n'; echo two >&2; exit 3"), None, move |e| {
                tx.lock().unwrap().send(e).unwrap();
            })
            .unwrap();
        let mut lines = Vec::new();
        let exit = loop {
            match rx.recv_timeout(Duration::from_secs(10)).unwrap() {
                Event::Output { line, stream, .. } => lines.push((stream, line)),
                exit @ Event::Exit { .. } => break exit,
            }
        };
        assert_eq!(exit, Event::Exit { id: started.id, code: Some(3), cancelled: false });
        let stdout: Vec<_> = lines.iter().filter(|(s, _)| *s == Stream::Stdout).map(|(_, l)| l.as_str()).collect();
        assert_eq!(stdout, ["one", "a", "b"]);
        assert!(lines.contains(&(Stream::Stderr, "two".to_string())));
        assert!(jobs.running().is_empty());
    }

    #[cfg(unix)]
    #[test]
    fn cancel_stops_the_whole_job() {
        let jobs = Jobs::default();
        let (tx, rx) = mpsc::channel();
        let tx = Mutex::new(tx);
        // The child shell starts a grandchild: both must go.
        let started = jobs
            .start(&sh("echo started; sleep 30 & wait"), None, move |e| {
                tx.lock().unwrap().send(e).unwrap();
            })
            .unwrap();
        assert!(matches!(rx.recv_timeout(Duration::from_secs(10)).unwrap(), Event::Output { .. }));
        jobs.cancel(started.id).unwrap();
        let exit = rx.recv_timeout(Duration::from_secs(10)).unwrap();
        assert!(matches!(exit, Event::Exit { cancelled: true, .. }), "{exit:?}");
    }

    #[test]
    fn cleans_appimage_python_environment() {
        std::env::set_var("APPDIR", "/tmp/.mount_test");
        std::env::set_var("PYTHONHOME", "/tmp/.mount_test/usr/");
        std::env::set_var("PYTHONPATH", "/tmp/.mount_test/usr/share/pyshared/:");
        let mut cmd = Command::new("true");
        environment(&mut cmd, None);
        std::env::remove_var("APPDIR");
        std::env::remove_var("PYTHONHOME");
        std::env::remove_var("PYTHONPATH");
        let envs: HashMap<_, _> = cmd.get_envs().collect();
        assert_eq!(envs.get(std::ffi::OsStr::new("PYTHONHOME")), Some(&None));
        assert_eq!(envs.get(std::ffi::OsStr::new("PYTHONPATH")), Some(&None));
    }
}
