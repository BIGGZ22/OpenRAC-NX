//! Running actions. A job is a child process whose output lines go to a
//! sink as [`Event`]s (the app forwards them to the UI as `job-output` and
//! `job-exit`); a detached launch (the game, Godot) is started and left
//! alone.

use std::collections::{HashMap, HashSet};
use std::io::{BufRead, BufReader, Read};
use std::process::{Child, Command, Stdio};
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

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

/// How long the end of a job waits for its output to be read, after the program itself has
/// ended: long enough for the last lines, short enough that a program that left something
/// behind (a daemon, a watcher) does not hold the queue up for ever.
const OUTPUT_GRACE: Duration = Duration::from_secs(3);

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
    /// The running jobs and the ones the player cancelled, under one lock: a cancel and
    /// the end of a job are decided together, so that one cannot come apart from the other.
    jobs: Mutex<State>,
}

#[derive(Default)]
struct State {
    running: HashMap<u32, Arc<Mutex<Child>>>,
    cancelled: HashSet<u32>,
}

/// What every job gets in its environment: unbuffered, UTF-8 Python output,
/// and the checkout, for tools that want it.
fn environment(cmd: &mut Command, root: Option<&std::path::Path>) {
    cmd.env("PYTHONUNBUFFERED", "1").env("PYTHONIOENCODING", "utf-8");
    if let Some(root) = root {
        cmd.env("OPENRAC_ROOT", root);
    }
    // An AppImage's own environment (the AppRun's PYTHONHOME, its paths, its
    // libraries) would follow the job: a Python told that its home is the
    // AppImage's cannot start, and the game would find the AppImage's
    // libraries instead of the system's. The job gets the system's.
    crate::appimage::sanitize(cmd);
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
        self.inner.jobs.lock().unwrap().running.insert(id, child.clone());

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
            // The output is read by two threads, which end when the pipe is closed. A
            // program that leaves something behind (a daemon, a watcher) keeps the pipe
            // open after it has ended, so the end of the job is reported after a moment:
            // the tail of the output may be lost, but a process that is not the job's
            // never holds the queue up.
            let deadline = Instant::now() + OUTPUT_GRACE;
            for p in pumps {
                while !p.is_finished() && Instant::now() < deadline {
                    std::thread::sleep(Duration::from_millis(20));
                }
                if p.is_finished() {
                    let _ = p.join();
                }
                // Not finished: the thread ends when the pipe is closed, at the latest when
                // the program holding it ends. It is left to end on its own.
            }
            let cancelled = {
                let mut jobs = inner.jobs.lock().unwrap();
                jobs.running.remove(&id);
                jobs.cancelled.remove(&id)
            };
            sink(Event::Exit { id, code: status.and_then(|s| s.code()), cancelled });
        });
        Ok(Started { id, title: plan.title.clone(), command: command_line(plan), detached: false })
    }

    /// Stops job `id` and everything it started (a build runs compilers in
    /// containers). A job that has already ended is not an error.
    pub fn cancel(&self, id: u32) -> Result<(), String> {
        // The end of a job and the cancel of it are decided together: a cancel that
        // arrives as the job ends is either the one that stopped it (the exit says so) or
        // nothing at all (the job had already ended). They cannot come apart, and one
        // cannot leave the other behind.
        let child = {
            let mut jobs = self.inner.jobs.lock().unwrap();
            match jobs.running.get(&id).cloned() {
                Some(child) => {
                    jobs.cancelled.insert(id);
                    Some(child)
                }
                None => None,
            }
        };
        let Some(child) = child else { return Ok(()) };
        let mut child = child.lock().unwrap();
        kill_tree(&mut child)
    }

    pub fn running(&self) -> Vec<u32> {
        let mut ids: Vec<u32> = self.inner.jobs.lock().unwrap().running.keys().copied().collect();
        ids.sort_unstable();
        ids
    }
}

/// Starts `plan` on its own: its output is not read and the launcher does
/// not wait for it.
pub fn launch(plan: &Plan, root: Option<&std::path::Path>) -> Result<Started, String> {
    // Quiet, so a console program (Godot's `_console.exe`, a Scoop shim) opens no console
    // window on Windows; a window program opens its window as ever.
    let mut cmd = crate::detect::quiet(&plan.program);
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
    // For the tests that run a shell, which are Unix only.
    #[cfg(unix)]
    use std::path::PathBuf;
    #[cfg(unix)]
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
        // The cancel and the end of the job are decided together: nothing is left behind.
        assert!(jobs.inner.jobs.lock().unwrap().cancelled.is_empty());
        assert!(jobs.running().is_empty());
    }

    #[cfg(unix)]
    #[test]
    fn a_job_that_leaves_a_process_behind_still_ends() {
        let jobs = Jobs::default();
        let (tx, rx) = mpsc::channel();
        let tx = Mutex::new(tx);
        // The program ends at once, but the process it left behind keeps the output pipe
        // open: the job must still report its end, or the queue waits for ever.
        let started = jobs
            .start(&sh("sleep 10 & echo started; exit 0"), None, move |e| {
                tx.lock().unwrap().send(e).unwrap();
            })
            .unwrap();
        let mut lines = Vec::new();
        let exit = loop {
            match rx.recv_timeout(Duration::from_secs(8)).unwrap() {
                Event::Output { line, .. } => lines.push(line),
                exit @ Event::Exit { .. } => break exit,
            }
        };
        assert_eq!(exit, Event::Exit { id: started.id, code: Some(0), cancelled: false });
        assert!(lines.contains(&"started".to_string()), "{lines:?}");
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
