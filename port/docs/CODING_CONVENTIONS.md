# C++ coding conventions for the port

> **Status: decided** (2026-10-22, by the maintainer). This page is the rule for the port's own
> code. It changes through a pull request that edits this page and that a maintainer approves;
> until then, it holds as written. The decisions behind it: C++20 is the floor and the ceiling,
> errors cross module boundaries as an own `openrac::Result<T>`, and the C headers are the
> contracts between C and C++ ([section 14](#14-the-c-boundary)).

The rules for the C++ and C that OpenRAC writes under `port/`: the runtime, the platform layer,
the renderer, the asset and media readers, the host program in `game/common`, the per-game host
code and the tests. The port is on its way to being C++ throughout (the games' own C stays C), and
this page is the foundation that conversion builds on: one layout, one way to document, one way to
own a resource, report an error and cross into C.

Anyone who touches the port follows this page: people, and any agent. It builds on
[CONTRIBUTING.md](../../CONTRIBUTING.md), [AGENTS.md](../../AGENTS.md), the
[port README](../README.md) and the port's design ([DESIGN.md](../../docs/port/DESIGN.md)), and none
of them is loosened here. After a context compaction, read this page again from the top instead of
working from a summary of it.

It started from the C++ conventions of the removed `runtime/` experiment
(`git show 7954152:runtime/docs/CODING_CONVENTIONS.md`). Its layout, comment, documentation and
project rules carry over, adapted to the port. Its rules about modelling the console's hardware do
not ([DESIGN.md](../../docs/port/DESIGN.md), section 5): the port does not model hardware.

Every rule below can be counted. If you cannot count your own compliance against your diff, you
have not complied. The list at the end ([section 25](#25-self-check-before-you-report-done)) is the
count.

---

## Contents

1. [Scope: which code, which rules](#1-scope-which-code-which-rules)
2. [What the machines check](#2-what-the-machines-check)
3. [Hard rules](#3-hard-rules)
4. [Language and standard](#4-language-and-standard)
5. [Layout](#5-layout)
6. [Comments](#6-comments)
7. [Documentation](#7-documentation)
8. [Naming](#8-naming)
9. [Modules, namespaces and files](#9-modules-namespaces-and-files)
10. [Declarations and types](#10-declarations-and-types)
11. [Ownership, lifetime and state](#11-ownership-lifetime-and-state)
12. [Functions and control flow](#12-functions-and-control-flow)
13. [Errors](#13-errors)
14. [The C boundary](#14-the-c-boundary)
15. [Per-game knowledge](#15-per-game-knowledge)
16. [Configuration](#16-configuration)
17. [Logging and messages](#17-logging-and-messages)
18. [Layering](#18-layering)
19. [Build](#19-build)
20. [Tests](#20-tests)
21. [Performance-sensitive code](#21-performance-sensitive-code)
22. [Reuse](#22-reuse)
23. [The project's constraints](#23-the-projects-constraints)
24. [Migrating existing code](#24-migrating-existing-code)
25. [Self-check before you report done](#25-self-check-before-you-report-done)

---

## 1. Scope: which code, which rules

### 1.1 Which code

| Code | Path | Follows |
|---|---|---|
| The port's C++ | every `.cpp`, `.mm` and C++ `.h` under `port/` | every rule on this page |
| The port's own C | the C contracts (`port/runtime/include/openrac/`, `port/game/common/include/openrac/`), `port/game/common/lib/*.c`, `port/game/*/host/*.c`, `port/tests/*.c`, `port/android/compat/` | sections 5, 6, 7, 8 (C names), 14, 15, 23: layout, comments, documentation, the C boundary, per-game tables, licence lines. Not the C++-only rules |
| A game's hand C | `port/game/*/hand/` | not this page: it is the game's C on its way into `games/` ([LAYOUT.md](../../docs/LAYOUT.md)) and keeps the decompilation's style |
| Generated C | hostgen's output in build trees, `port/game/rac1/script_binding.c` | not this page: change the generator, never the output |
| The decompilations | `games/` | never edited from port work |
| Python | `port/tools/` | the repository's Python habits: licence lines, a module docstring with the usage, `unittest` tests beside the tool |
| The mod SDK | `sdk/include/` | its own rules ([MODS.md](../../docs/port/MODS.md)): `openrac_mod.h` is a C ABI, `mod.hpp` a C++17 layer compiled into mods |

In C, `static` is how a file-local function is spelled, and the comment forms are C's (6.1). The
rest of the C rules are the same as for C++.

### 1.2 Which rules apply to which work

| Work | What the rules cover |
|---|---|
| A new file, function, type or module | Every rule. |
| A function or file you change for another reason | Every rule on the lines you touch, and the doc comment of each function you change. Leave the rest of the file as it is. |
| A migration ([section 24](#24-migrating-existing-code)): `static` to an anonymous namespace, a file-global `g` to a state object, `getenv` to `common/env.h`, `printf` to the log, `extern "C"` into its contract file | Only as a commit of its own, or as the package that owns it, one module at a time. Never as a sweep over the tree. |
| A formatting pass over existing code | Layout and comments only, with the proof of [section 24.3](#243-proving-that-a-pass-changed-nothing). |

Commit types follow [CONTRIBUTING.md](../../CONTRIBUTING.md#commits): `style(port)` for layout
alone, `docs(port)` for comments alone, `refactor(port)` for renames and moves. A commit never mixes
layout or comments with a change of behaviour.

### 1.3 What exists today and what is arriving

Some rules name a header or a tool that a later package of the restructure writes
([docs/restructure/PROGRESS.md](../../docs/restructure/PROGRESS.md)). Until it lands, the rule says
what to do instead.

| Rule | Arriving with | Until then |
|---|---|---|
| `openrac::Result<T>` in `common/result.h` (13.3) | P2-08 | `bool` and a `std::string& error`, or `std::optional<T>` |
| The C-boundary wrapper that catches everything (13.4) | P2-08 | a `try` and two `catch` clauses written out, as 13.4 shows |
| `common/env.h`, the table of `OPENRAC_*` variables (16) | P1-03 | one read per variable, in its module, listed in DEBUGGING.md |
| `common/json.h` and `common/ps2_float.h` | P1-01 | `viewer/json.h` and `assets/ps2_float.h` |
| `host::GameProfile`, one per game version (15) | P2-03 (rac1), P3-02 (all) | per-game knowledge in `port/game/<id>/`; nothing new in shared code |
| The `scene` module and `renderer/ui` (9.1) | P2-02, P2-06 | `viewer/` and `game/common` |

---

## 2. What the machines check

| Tool | What it checks | Where it runs | Fails the build or CI |
|---|---|---|---|
| clang-format, [`port/.clang-format`](../.clang-format) | layout (section 5) | your editor; CI on new files and changed lines | new C++ files: yes. Changed lines: reported, not failing yet (24.4) |
| [`tools/conventions/check_conventions.py`](../tools/conventions/README.md) | licence lines, tabs, dashes everywhere; on new C++ files: `static` at namespace scope, raw `new`/`delete`/`malloc`/`free`, `using namespace` in headers, `extern "C"` outside contract files, console output outside tests and `main` | CI ("Port conventions"), and you before you commit | yes |
| clang-tidy, [`port/.clang-tidy`](../.clang-tidy) | naming, ownership, function size, `else` after `return`, missing `override`, nested ternaries, `goto` | you, on the C++ files you touch (it needs a configured build) | not in CI; new code has zero findings |
| The compiler, `openrac_warnings` (`port/cmake/Warnings.cmake`) | `-Wall -Wextra -Wshadow`, and for C++ `-Wpedantic -Wconversion` | every build | no new warning is accepted in review |
| [`tools/layering/layering.py`](../tools/layering/README.md) | includes against the layers and allowed edges of section 18; today's violations are listed exceptions | you before you commit (not in CI yet) | it exits 1 on a new violation; CI does not run it yet |
| ctest, the Python tests | behaviour | every build, CI | yes |

clang-format is version 21. CI installs 21.1.8 from PyPI (`pip install clang-format==21.1.8`); on
macOS, Xcode's `xcrun clang-format` (Apple clang-format 21.0.0) gives the same output on every port
file (measured on 2026-10-22). When two versions disagree, 21.1.8 decides. clang-tidy needs version
19 or newer.

From the top of OpenRAC:

```sh
# Layout: apply it to each file you change, then check it (prints nothing when clean).
clang-format -i port/renderer/texture.cpp
clang-format --dry-run -Werror port/renderer/texture.cpp

# The countable rules, on every file and on what you added since master.
python3 port/tools/conventions/check_conventions.py --since origin/master --touched

# The layering: no include of a higher layer (prints nothing but its summary when clean).
python3 port/tools/layering/layering.py

# Naming and ownership on the C++ you changed (needs port/build/release configured).
clang-tidy -p port/build/release port/renderer/texture.cpp
```

---

## 3. Hard rules

These sixteen are not negotiable. Each one is countable. Count them against your own diff before
you say the work is done.

1. **The layout is what `.clang-format` produces.** Four spaces, never a tab, at most 100 columns.
   The tool reports nothing on the files you added, nor on the lines you changed (section 5).
2. **Every body has braces.** Every `if`, `else`, `for`, `while`, `do` and `switch` body is a
   `{ }` block, with its statements on their own lines. The tool inserts them.
3. **Every declaration is documented.** Every file has a file comment; every type, function,
   member, constant and enumerator group in a header, and every function in a source file, has a
   doc comment (section 7). Every module has a README in the form of 7.7.
4. **Every branch and every unexplained number says why it is there.** Each `if`, `else if`,
   `case`, guard clause and deciding ternary has a comment. Each numeric literal other than 0 and
   1 has a name, or a comment that names what it stands for (section 6).
5. **A comment says why, in one sentence, or it is a block of at most 8 lines.** It never repeats
   the line below it (6.4).
6. **Never rewrite a comment you did not break.** If a comment is correct and follows these rules,
   leave it byte for byte as it is.
7. **No em dashes and no en dashes, anywhere.** Not in code, comments, strings, Markdown or commit
   messages. Use a comma, a colon, a full stop or parentheses. A hyphen between numbers
   (`bits 0-14`) is fine.
8. **Names follow section 8.** `snake_case` functions and variables, `PascalCase` types, `m_`
   private members, `kPascalCase` constants, `openrac::<module>` namespaces.
9. **File-local code is in an anonymous namespace**, never `static` at namespace scope (C++ only).
10. **Every resource has an owner that releases it.** No raw `new`, `delete`, `malloc` or `free` in
    C++ (section 11).
11. **No exception crosses a C boundary or leaves the frame loop.** A module's public function
    reports failure in its return type (section 13).
12. **The C contracts stay C.** `extern "C"` definitions live only in the file named for their
    contract. C++ never includes a hostgen-generated header; game C never includes a C++ header
    (section 14).
13. **Shared code knows no game.** No game id comparison, no game address, no per-game branch
    outside the per-game tables (section 15).
14. **The log and the environment have one door each.** Diagnostics go through `common/log.h`;
    `OPENRAC_*` variables through `common/env.h` once it lands (sections 16 and 17).
15. **A lower layer never includes a higher one** (section 18).
16. **A refactor or formatting pass changes no behaviour, and you say how you checked**
    (section 24).

---

## 4. Language and standard

### 4.1 C++20, and not past it

The port's C++ is C++20 (`CMAKE_CXX_STANDARD 20`, extensions off), and that is the ceiling as well
as the floor. No C++23 language or library feature (`std::expected`, `std::print`, deducing `this`,
`std::flat_map`) until the maintainers raise it: every target toolchain must have it first (Apple
Clang, Clang against MSVC's library on Windows, the Android NDK).

A C++20 library feature the port does not use yet is checked against those three before it is used:
CI builds Linux and Windows, and `--preset android-arm64` builds Android. `std::format`,
`std::span`, concepts and `std::bit_cast` are in use and safe.

### 4.2 C11, with the GNU extensions

The port's C is C11 with GNU extensions (`CMAKE_C_EXTENSIONS ON`): the guest contract uses statement
expressions and `cleanup`. Clang builds all of it, `clang-cl` on Windows; MSVC's own compiler cannot
([port/CMakeLists.txt](../CMakeLists.txt)).

### 4.3 Compiler extensions and attributes

C++ code uses no compiler extension. Where the platform needs one (`__builtin_expect`, an
`__attribute__`, an intrinsic), it sits in one named helper in `common/` with a comment that says
why. Standard attributes are used for what they say: `[[nodiscard]]` (12.4), `[[maybe_unused]]`
for a parameter only some builds read, `[[fallthrough]]` (12.8). `[[likely]]` and `[[unlikely]]`
need a measurement (section 21).

### 4.4 Platforms

The port builds on Linux, macOS, Windows and Android, 64-bit only. A POSIX-only header or function
(`unistd.h`, `execinfo.h`, `fseeko`) goes behind `_WIN32` with its Windows form beside it, as in
`runtime/`. A debugging aid that wants the host's stack calls `openrac_host_caller` (`guest.h`).

### 4.5 Libraries

The standard library first. A third-party library (SDL3, Vulkan, Luau, Dear ImGui, Embree) is
included only by the module that wraps it (section 18), and the rest of the port uses that
module's interface. A new dependency is fetched by a file in `port/cmake/`, pinned, and credited in
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md) with its licence.

---

## 5. Layout

Names, numbers and facts in the samples on this page are made up to show the form; they are not
statements about the game.

All of this section is what the tool does. It is written down so that you can predict the tool and
write code that needs no fixing. When [`.clang-format`](../.clang-format) and this page disagree
about layout, the file wins and this page gets fixed.

### 5.1 The choices

| Subject | Rule |
|---|---|
| Indentation | four spaces per level, never a tab; a continued line goes one level in |
| Line length | at most 100 columns, comments and strings included; a long string is split into two literals |
| Braces | on the line of the statement; every body braced (`InsertBraces`) |
| One line | only an empty function, an accessor defined in its class, and a short lambda passed as an argument share a line with their body |
| Namespaces | not indented; the closing line names the namespace (`}  // namespace openrac::log`) |
| Access specifiers | at the class's own column, no blank line after them |
| `case` labels | one level under `switch`; a case's statements one more |
| Wrapped calls | break after `(`; the arguments on one line at the next level, or one per line; `)` alone at the statement's indent |
| Wrapped expressions | break before the operator; a wrapped ternary starts its lines with `?` and `:` |
| Pointers | `u8* data`, `const Image& image` |
| Blank lines | never two in a row, none at the start of a block, one between definitions |
| Templates | `template <...>` on its own line |
| Includes | grouped and sorted by the tool (9.5) |

### 5.2 Braces

```cpp
// The texture is gone when the level changes, so it is made again.
if (!m_texture) {
    m_texture = upload(image);
}
```

Never:

```cpp
if (!m_texture) m_texture = upload(image);
```

A body of one line becomes a body of two the first time somebody adds a log line, and then the
second line runs on every path while the indentation says it does not.

### 5.3 Wrapped calls

Write a call on one line when it fits. When it does not, the tool breaks after `(`. If a call wraps
only because one argument is a long expression, give the expression a name first:

```cpp
const Entry& entry = entries[n];
const std::uint32_t at = table + static_cast<std::uint32_t>(n) * kEntryBytes;

put_entry(at, entry.name, entry.directory, entry.size);
```

If an expression needs more than three lines, it is doing too much. Give its parts names.

### 5.4 Blank lines in a body

A body is made of groups: a few lines that do one step. One blank line before each group that
starts with a comment, after the closing brace of a block (unless `}` or `else` follows), after a
run of declarations, after each guard clause, and before the final `return` when more than three
lines come before it. None directly inside `{` or before `}`.

```cpp
std::size_t Mixer::fill(std::span<float> out) {
    // Nothing plays until a bank is loaded.
    if (!m_bank) {
        std::ranges::fill(out, 0.0f);
        return 0;
    }

    std::size_t written = 0;

    // A voice that ends inside this block frees its slot for the next one.
    for (Voice& voice : m_voices) {
        written = std::max(written, voice.mix_into(out));
    }

    return written;
}
```

### 5.5 Hand-laid tables

A table the tool would spoil (rows of a format laid out so that the columns mean something) is
fenced with the tool's directives and a reason:

```cpp
// clang-format off: one vertex per row, so the fields line up with the format.
constexpr float kQuad[] = {
    -1.0f, -1.0f,   0.0f, 0.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
};
// clang-format on
```

This is the only way to turn the tool off. The reason is mandatory, and the directive never covers
more than one table or any function body.

---

## 6. Comments

### 6.1 The forms

| Form | Used for |
|---|---|
| `//` | Everything in C++: doc comments (section 7), a one-sentence comment inside a body, a block of up to 8 lines, a trailing label, a section heading. |
| `/* ... */` | C files, in the form the file already uses. In C++, only around the name of a parameter a function does not use: `void on_key(int /*scancode*/)`. |

Nothing else: no `/** */`, no `///`, no `/*!`, no Doxygen tags, no `#if 0`, no commented-out code
(git keeps it). The port has always documented with `//`, and this keeps one form.

A comment starts with `// ` (two slashes and a space). Above a line it is a complete sentence:
capital letter, full stop. Trailing labels are the exception (6.8).

### 6.2 Where a comment is required

1. Above every `if`. Above every `else if` and `else` that is not the plain complement of an `if`
   whose comment explains both; since `} else if (` shares a line, its comment is the first line of
   its body.
2. Above every `case` and `default`, or trailing on the label when it only names a value.
3. Above every loop whose exit is not in its header, and every `for (;;)`, saying how it ends.
4. Above every ternary that decides something.
5. Above every `continue`, `break` and early `return` that is not the last line of a `case`,
   unless the comment on the `if` around it already says why.
6. Above a statement that has to stay where it is: a store before a notify, a call after a flush,
   a GL state set before a draw.
7. Above a cast that changes width or sign when the reason is not plain from the types.
8. Above or on each numeric literal other than 0 and 1 (6.5).
9. Above each lambda that is stored or passed as a callback, saying what calls it and on which
   thread.
10. At the head of each group of statements (5.4), unless the group is plain bookkeeping (6.3).

### 6.3 Plain bookkeeping needs no comment

A line whose meaning and reason follow from its names alone: assigning a value by a name that says
what it is (`m_frame = frame;`), clearing state through calls whose names say so
(`m_voices.clear();`), a loop step, a `return result;`, a forwarded argument. A run of them under one
group comment is the right form.

Anything else needs a reason on the page: every mask and shift, every arithmetic with a constant,
every write into game memory, every wait, lock, notify and atomic, every cast with a consequence,
and every statement reached only under a condition.

### 6.4 Say why: the deletion test

A comment says what the code cannot: why this branch exists, what the game or the format does that
makes it necessary, why the order matters, what a number is, where a fact comes from. Read the
comment, then the line. If the comment is the line in English, delete it.

| Wrong | Right |
|---|---|
| `// If the queue is full.` | `// The game may not run further ahead than this, or the frame it shows is stale.` |
| `// Loop over the mobys.` | `// Only mobys the PVS left visible this frame are drawn.` |
| `// Return false on error.` | (cut it) |
| `// Increment the count.` | (cut it) |

If you cannot think of a reason for a branch, look at it again: either the reason is plain from a
name three lines up and a group comment covers it, or the branch should not be there.

### 6.5 Numbers, fields and where facts come from

Every numeric literal other than 0 and 1 is one of three things, and says so:

1. A named constant, whose name does the explaining and which has a doc comment (7.6).
2. A field of a record of the game's data, read with a named helper or a mask. A comment above the
   group names the record, its fields and how their layout is known.
3. A unit or a size with a reason, which the comment gives.

A comment that states a fact about the game, its data or the console's formats (a layout, an
address's meaning, an order the game relies on) ends with one of four tags, so the next person knows
how far to trust it:

| Tag | Means |
|---|---|
| `(documented)` | in public documentation; the file comment's sources line says which |
| `(seen in game code)` | shown by the decompiled C or the program's behaviour; name the function by the decompilation's name. Never paste bytes |
| `(measured)` | found by a test or an experiment on the port; say what ran if it is not plain |
| `(assumed)` | nothing confirms it yet; it stays marked until something does |

```cpp
// A moby's class index is the byte at 0xAA of its record (seen in game code: moby_update).
const std::uint8_t class_index = read_u8(record, kMobyClassOffset);
```

A mask or offset used twice in a file becomes a named constant or helper.

### 6.6 Length

| Thing | Limit |
|---|---|
| A one-line `//` comment inside a body | one sentence, 20 words at most, and it fits the line |
| A block of `//` lines inside a body | 8 lines |
| The first sentence of a doc comment | 25 words |
| A function's doc comment | 20 lines |
| A member's or constant's doc comment | one sentence, or 6 lines |
| A type's doc comment | 30 lines |
| A file comment | 40 lines |

Past a limit, the text goes into the module's README or a page under `docs/port/`, and the comment
links to it. Wrap prose to fill each line up to the 100 columns; do not break a sentence early.

### 6.7 Voice

Comments are written like a person explaining the line to a colleague at the same screen. Plain
words, flat sentences, contractions are fine.

1. Lead with the thing. "The cache keeps one texture per palette." Not "It should be noted that".
2. One idea per sentence.
3. No reassurance and no consequence essays. A comment states a fact.

Never write these in a comment, a doc comment, a string, a log message or a commit message:

| Kind | Examples |
|---|---|
| Long dashes | U+2014 (em dash) and U+2013 (en dash) |
| Tour-guide words | Furthermore, Moreover, Additionally, In addition |
| Hedges | It is worth noting, It should be noted, Note that, Please note |
| Filler | simply, just, basically, essentially, actually, obviously, of course, very, really |
| Hype | robust, comprehensive, seamless, powerful, elegant, cutting-edge |
| Machine words | ensure, utilize, leverage, facilitate, delve, in order to |
| Reassurance | "so this is safe", "nothing bad happens", "so the caller can simply retry" |
| First person plural | we, our, let's |

| Written | Fixed |
|---|---|
| `// This ensures the upload is done before we draw.` | `// Wait for the upload before the draw.` |
| `// We simply skip unknown chunks, so nothing breaks.` | `// Skip unknown chunks and count them.` |
| `// Robust handling of a bad header.` | `// A header with no entries has nothing to read; stop here.` |

### 6.8 Trailing comments

A comment after code on the same line is allowed on an enumerator or a constant, on a `case` label
that names a value, on a row of a data table, on a statement that decodes one field (eight words or
fewer), and on a member in a run of members (7.5). A trailing label is a fragment with no full stop;
a trailing comment on a member is a sentence. Logic is explained above the line, never beside it.

### 6.9 Section headings

A file longer than 300 lines may be divided with headings of this one form, the port's:

```cpp
// ---- Uploads ----
```

One blank line before and after. One to five words. No rulers, no nesting.

### 6.10 What a comment never contains

- Code that is switched off.
- `TODO`, `FIXME`, `XXX`, `HACK`. Work not done is recorded where people look for it: a known
  problem in [KNOWN_ISSUES.md](../../docs/port/KNOWN_ISSUES.md), restructure work in
  [PROGRESS.md](../../docs/restructure/PROGRESS.md), the rest as an issue.
- A date, a version, an author's name or initials, an issue number. Git knows.
- Anything from Sony's SDK source, samples or headers, from leaked or NDA material, or any bytes of
  a retail disc.
- The name of anyone who asked to be removed from these projects.

---

## 7. Documentation

The documentation is read in the source and in each module's README. It is the first thing a new
contributor reads, so it is held to the same standard as the code.

### 7.1 The file comment

Every file opens with the two licence lines, then `//` alone on a line, then the file comment: what
the file is, what it is based on, what it leaves out.

```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The port's log: one line per message, to stderr and, once a session is open, to the session's
// file too. Messages are formatted with std::format.
//
// A session is one run of a game: its log is `<folder>/<name>.log`, begun afresh each run, with
// the runs before it moved along, so a log a player sends is one run and small.

#pragma once
```

A `.cpp` that implements its own header says in its first sentence that it does (`The log's
writer: the files, the ring of recent lines, the header.`) and then only what the header does not
say. A file that implements a slice of another header, or a C contract, names it. A test file says
what it tests and where its expected values come from (section 20). C files use the same content in
their `/* */` form.

### 7.2 Functions

Every function has a doc comment: free functions, member functions, constructors, destructors that
do work, operators, and the functions of an anonymous namespace. It is written once, at the first
declaration (the header), and the definition does not repeat it.

The comment sits directly on the declaration, with no blank line, and has these parts in this order,
leaving out a part that has nothing to say:

1. A summary: one sentence starting with a verb in the third person ("Opens", "Returns",
   "Draws").
2. The parameters: each one whose meaning, unit, range or ownership is not in its name and type is
   named in backticks and described.
3. The result: what each value means, and how the caller learns of a failure (13.3).
4. What the caller must have done or must know: a state, a thread, a lock held, a lifetime.
5. What it does not do, when a reader would expect it to.

```cpp
// Moves the earlier sessions' logs along (`<name>.log` to `<name>.1.log`, ..., the one past
// `<name>.<keep>.log` removed) and opens `<dir>/<name>.log` new for this one. False when it cannot
// be written; the messages still go to stderr.
bool open_session(const std::filesystem::path& dir, const std::string& name, int keep = 5);
```

The short forms: a one-line accessor takes a one-line comment; an overload says what differs from
the main one; an override needs a comment only when it behaves in a way its base's comment does not
say; a defaulted or deleted special member needs none; a hot function says so in a line starting
`Hot path:` (section 21).

### 7.3 Types

Every `class`, `struct`, `enum`, `enum class` and type alias whose meaning is not plain from its
right-hand side has a doc comment. The first sentence says what the type is, as a noun phrase. Then,
as they apply: what it owns and how it is copied or moved (11.1), its invariants, and, for a type
more than one thread uses, which thread calls what and what guards what.

```cpp
// A Vulkan image and its memory, freed together when the owner goes.
//
// Move-only. The device it was made on outlives it (the renderer destroys its images first).
// Only the render thread touches it.
class Image {
```

### 7.4 Thread and ownership notes

A member shared between threads names its guard ("Guarded by `m_mutex`." or "Atomic; written by
the audio thread, read by the game's."). A member or parameter that points at memory it does not
own says who owns it and how long it lives.

### 7.5 Members

Every data member has a comment, in one of two forms, one form per run of members.

```cpp
// The frames each voice has left before it ends; 0 for a free slot.
std::array<std::uint32_t, kVoices> m_remaining{};

int m_width = 0;         // Window width in pixels.
int m_height = 0;        // Window height in pixels.
bool m_visible = false;  // True once the window has been shown.
```

The first form is a `//` sentence (or block of up to 6 lines) above the member. The second is a
trailing sentence for a run of up to eight related members, with no blank line inside the run.
Members are declared one to a line.

### 7.6 Constants and enumerators

A constant or a group of them has a doc comment saying what the number stands for, with a source
tag if it is a fact about the game (6.5). Each enumerator is on its own line; when its name differs
from the name the format or the game uses, a trailing label gives that name.

```cpp
// The sizes the game's sound banks are read in (seen in game code: snd_load_bank).
constexpr std::size_t kBankHeaderBytes = 0x40;
constexpr std::size_t kBankEntryBytes = 0x10;
```

### 7.7 The module README

Every module (9.1) has a `README.md` in its directory, with these sections in this order. A
section with nothing to say says so in one line rather than being left out.

```markdown
# <module>

One paragraph: what the module is for, and what it is not for.

## Interface
The public headers, one line each: what each one declares.

## Depends on
The modules below it (section 18 of CODING_CONVENTIONS.md) and the third-party libraries it wraps.

## State and threads
The state it owns, who owns the objects that hold it, and which thread calls what.

## Errors
How its public functions report failure (Result<T>, bool and a message), and what it logs.

## Configuration
The settings and OPENRAC_* variables it reads, linking docs/port/DEBUGGING.md.

## Tests
The ctest names that cover it and what each one proves.

## Further reading
The pages under docs/port that explain its design.
```

A README that exists today gains the missing sections when its module is restructured, not in a
sweep. A new module starts with all of them.

### 7.8 Documentation outside the code

- A design, a study or a decision: `docs/port/` ([its README](../../docs/port/README.md)).
- A new or moved module: its row in the port README's "What is here", and
  [docs/LAYOUT.md](../../docs/LAYOUT.md) when the tree's shape changes.
- A new `OPENRAC_*` variable: [DEBUGGING.md](../../docs/port/DEBUGGING.md) (section 16).
- A known problem that is guarded but not fixed: [KNOWN_ISSUES.md](../../docs/port/KNOWN_ISSUES.md).
- Restructure work: its row in [PROGRESS.md](../../docs/restructure/PROGRESS.md).

A change that makes a doc wrong fixes the doc in the same commit series.

---

## 8. Naming

| Thing | Style | Examples |
|---|---|---|
| Namespace | `lower_case`, the module's name | `openrac::renderer`, `openrac::assets::disc` |
| Type, alias, concept | `PascalCase`; an acronym is one word | `TexturePool`, `GameProfile`, `VkglDevice` |
| Template parameter (type or value) | `PascalCase` | `template <std::size_t Lines, typename Item>` |
| Function, method | `snake_case`, a verb or a question | `open_session`, `upload`, `is_visible` |
| Local, parameter | `snake_case` | `frame_index`, `bank` |
| Public data member (of a `struct`) | `snake_case` | `width`, `frame_rate` |
| Private or protected data member | `m_` and `snake_case` | `m_left`, `m_voices` |
| Constant: `constexpr`, namespace-scope `const`, a class's `static constexpr` | `kPascalCase` | `kSessionFileLimit`, `kBankEntryBytes` |
| Scoped enumerator | `PascalCase` | `Level::Debug`, `FrameRate::Capped` |
| Unscoped enumerator | `kPascalCase` | `kGetInfo` |
| Thread-local variable | `tls_` and `snake_case` | `tls_scratch` |
| The one namespace-scope pointer a C callback needs (11.4) | `g_` and `snake_case` | `g_crash_context` |
| Macro | `OPENRAC_UPPER_SNAKE` | `OPENRAC_ENHANCED_RENDERING` |
| File | `snake_case.h`, `snake_case.cpp`; tests `<subject>_test.cpp` | `texture_pool.cpp`, `log_test.cpp` |
| C contract function and type | `openrac_` and `snake_case` | `openrac_game_crash`, `openrac_front_end` |

The test macros of `tests/check.h` (`CHECK`) are the one exception to the macro prefix.

### 8.1 Names from outside keep their spelling

A name taken from an outside API or format is spelled as that API spells it: OpenGL's `GL_TEXTURE_2D`
and `glDrawArrays`, Vulkan's `VkImage`, SDL's `SDL_Window`, the console's format names (`PSMCT32`), a
function of the game by the decompilation's name. The byte aliases `u8`, `u16`, `u32`, `u64` of
`assets/bytes.h` are the reader modules' own (10.2).

### 8.2 Short names

A name is a word, or words joined by underscores. Short names are allowed only for a loop counter
(`n`, `row`, `column`), a coordinate or component (`x`, `y`, `z`, `u`, `v`, `r`, `g`, `b`, `a`), and a
name the outside API or format gives. `msg`, `idx`, `tmp`, `cnt`, `buf`, `ptr`, `mgr` are not names.

### 8.3 Functions and booleans

- A question reads as one (`visible()`, `is_empty(x)`, `has(flag)`); an action as an action
  (`reset`, `upload`, `flush`).
- No `get_` prefix: the reader is the noun (`width()`), its writer `set_` and the noun.
- A callback member starts with `on_`: `on_resize`, `on_frame`.
- A value in a unit that is not obvious carries it: `size_bytes`, `delay_ms`, `angle_radians`.
- A boolean reads as a statement: `m_visible`, `paused`, `has_bank`. Not `flag`, `status`, `mode`.
  Name the positive case and write `!` at the use.

### 8.4 Type names

A name says what a thing is, not how it is built. `Info`, `Data`, `Manager`, `Helper`, `Util`,
`Impl` say nothing; pick the noun. A type is named for what it does (`TexturePool`, `Mixer`,
`Window`).

---

## 9. Modules, namespaces and files

### 9.1 A module

A module is one directory under `port/`, one namespace, one CMake library target and one README.
They share a name.

| Directory | Namespace | Target | Today |
|---|---|---|---|
| `common/` | `openrac` (its headers' own: `openrac::log`, `openrac::stats`) | `openrac_common` | exists |
| `runtime/` | `openrac::runtime` | `openrac_runtime` | exists |
| `platform/` | `openrac::platform` | `openrac_platform` | exists |
| `assets/` | `openrac::assets` (`::disc`, `::rac1` for subdirectories) | `openrac_assets` | exists |
| `audio/`, `media/` | `openrac::audio`, `openrac::media` | `openrac_audio`, `openrac_media` | exist |
| `renderer/` | `openrac::renderer` (`::world` for `world/`) | `openrac_renderer` | exists; `renderer/ui/` arriving with P2-06 |
| `renderer/vulkan/` | `openrac::vkgl` | `openrac_renderer_vulkan` | exists |
| `scene/` | `openrac::scene` | `openrac_scene` | arriving with P2-02 (from `viewer/`) |
| `enhanced/` | `openrac::lighting` | `openrac_enhanced` | exists |
| `game/common/` | `openrac::host` for new code (`openrac::frontend`, `openrac::mods` today) | `openrac_host` | arriving with P3-02 |
| `game/<game>/` | `openrac::game::<game>` (`openrac::game::rac1`) | `openrac_game_<id>` | C today; C++ with the profiles (P2-03) |
| `viewer/`, `extract/` | `openrac::viewer`, `openrac::extract` | the programs | exist |

A subdirectory of a module may have a namespace of its own, named for the subdirectory
(`openrac::assets::disc` for `assets/disc/`). A namespace that differs from its directory today
(`openrac::gl` in `renderer/gl.h`, `openrac::lighting` in `enhanced/`) keeps its name until its
module's package renames it as a `refactor(port)` commit of its own; new code in such a directory
uses the namespace the directory already uses.

A module that is its own subject in several files keeps them flat. Shared declarations that only the
module's own files use go in `<subject>_internal.h`, which nothing outside the module includes.

### 9.2 One file, one subject

A header declares one main thing, named like the file (`texture_pool.h` declares `TexturePool`).
Helper types that appear only in its interface may live with it. Two independent types are two
files. A source file implements one header and has its stem. A program's `main.cpp` and a test are
the exception: one `main` and its helpers.

### 9.3 A header, from the top

1. The two licence lines, `//`, the file comment (7.1), a blank line.
2. `#pragma once`, a blank line. (C contract headers keep their include guards and the
   `extern "C"` block of 14.2.)
3. The includes, in groups (9.5).
4. `namespace openrac::<module> {`, a blank line.
5. In this order: aliases, constants, enumerations, forward declarations, types in order of
   dependency, free functions, inline and template definitions.
6. `}  // namespace openrac::<module>`.

### 9.4 A source file, from the top

1. The two licence lines, `//`, the file comment, a blank line.
2. The file's own header, a blank line, the other includes in groups.
3. `namespace openrac::<module> {`.
4. An anonymous namespace holding everything private to the file (constants, types, helper
   functions, in the order they are used), closed before the first definition.
5. The definitions, in the order the header declares them.
6. `}  // namespace openrac::<module>`.

Internal linkage is spelled with the anonymous namespace, never `static` at namespace scope. A
`static` member of a class and a `static` local are fine. A header defines a function only as
`inline`, a template, or a member defined in its class.

### 9.5 Includes

Include what the file itself uses, directly. The tool groups and sorts them; the groups, a blank
line between each:

1. The file's own header (a `.cpp` only).
2. The standard library and the C library, in angle brackets: `<array>`, `<cstdint>`.
3. SDL3: `<SDL3/SDL.h>`.
4. The port's own headers, in double quotes, written from `port/`: `"renderer/gl.h"`,
   `"common/log.h"`. The C contracts as `"openrac/guest.h"`.
5. Other third-party headers in angle brackets with a directory (`<vulkan/vulkan.h>`): the tool
   puts them last. One without a directory (`<imgui.h>`) looks like a standard header to the tool
   and sorts with group 2; leave it there.

A header forward-declares a type instead of including its definition when a pointer or reference
is enough. A header never includes a header only for its users' sake. C++ never includes a header
hostgen generates; game C never includes a C++ header (14.3). A module never includes another
module's `_internal.h`.

### 9.6 `using`

`using namespace` never appears in a header. In a source file it is allowed for the file's own
module's namespace only, after the includes. For one name, prefer a `using` declaration or an alias:
`using openrac::assets::u32;`, `namespace fs = std::filesystem;`.

---

## 10. Declarations and types

### 10.1 One declaration per statement

Each variable is declared on a line of its own, initialised where it is declared, in the narrowest
scope that holds it, at its first use. No chained assignment, no comma-separated declarators.

### 10.2 Integer types

- A value with a width (a field of the game's data, an address, a pixel) has that exact width:
  `std::uint8_t` to `std::uint64_t`, `std::int8_t` to `std::int64_t`, spelled with `std::` and
  `<cstdint>`. The asset readers that read the console's data may use the `u8` to `u64` aliases
  of `assets/bytes.h`; nothing else defines aliases of its own.
- A game address is `std::uint32_t` (the game is 32-bit), never a host pointer.
- `std::size_t` for byte counts, indices and container sizes.
- `int` for host-side counts and the results of APIs that return `int`.
- `bool` for a flag; never an `int` that holds 0 or 1.
- Never `long`, `short` or `char` for a number, or `unsigned char` for a byte (`std::byte` or
  `std::uint8_t`). `char` is for text.

### 10.3 Literals

Hexadecimal digits upper case, prefix and suffix lower case: `0x7FFFFFFFu`. An unsigned literal
carries `u`. A 64-bit constant is `0x...ull`, or `std::uint64_t{1}` when shifted by a variable. A
decimal of five digits or more uses `'` between thousands: `294'912'000`. A `float` literal carries
`f`: `1.0f`.

### 10.4 Casts

The build runs with `-Wconversion`, and every narrowing says so with a cast.

1. `static_cast<T>(x)`. Never a C-style cast `(T)x`, never a functional cast `T(x)` of a non-class
   type.
2. Reading the bits of one type as another: `std::bit_cast`, or `std::memcpy` into a value. Never
   `reinterpret_cast` of a pointer to read it as another type.
3. `reinterpret_cast` only where bytes cross to a C interface or a GPU buffer, with a comment.
4. No `const_cast`, except to pass a buffer to a C API that takes a non-const pointer and does not
   write it, with a comment naming the API.
5. A cast to `void` only for a variable or parameter deliberately unused, with a comment; prefer
   `[[maybe_unused]]`.

### 10.5 `auto`

`auto` is allowed for a lambda stored in a variable, an iterator or other long obvious type
(`auto it = map.find(key);`), a declaration whose right-hand side names the type
(`auto texture = std::make_unique<Texture>(...)`), and a structured binding. Never for a number:
the width of a value is information.

### 10.6 `const` and `constexpr`

A parameter taken by reference or pointer and not changed is `const`. A member function that does
not change the object is `const`. A constant known at compile time is `constexpr`, in the narrowest
scope that uses it. A local assigned once is `const`.

### 10.7 Enumerations

A new enumeration is an `enum class` with its underlying type written when it is stored or crosses
a boundary (`enum class Level : std::uint8_t`). An unscoped `enum` is for a group of named integers
used as integers.

### 10.8 Macros

Do not write macros. Use `constexpr`, `inline` functions and templates. A macro that has to exist (a
build switch, `__FILE__` and `__LINE__` in a check) is `OPENRAC_`-prefixed and its comment says why
it cannot be a function. No `#define` of a constant, no macro that hides control flow.

---

## 11. Ownership, lifetime and state

### 11.1 Every resource has an owner

A resource that must be released is owned by an object that releases it in its destructor: memory,
files, GPU objects (GL names, Vulkan handles and their memory), SDL handles, library handles
(`dlopen`, `LoadLibrary`), threads, mapped memory, sockets. Nothing pairs a bare open with a manual
close across branches.

- A wrapper that owns a handle is **move-only**: copy operations deleted, move operations
  `noexcept`, and the moved-from object owns nothing.
- A C handle with one release function is a `std::unique_ptr` with a deleter type:

```cpp
// Destroys an SDL window; the deleter of WindowHandle.
struct WindowDeleter {
    void operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
};

// An SDL window, destroyed when its owner goes.
using WindowHandle = std::unique_ptr<SDL_Window, WindowDeleter>;
```

- A handle that is not a pointer (a GL name, a Vulkan handle with its device) gets a small
  move-only class, written once per kind of handle in the module that owns the API.
- Follow the rule of zero: a type made of owners needs no destructor or copy operations of its own.
  A type that does write one writes or deletes all five, and its doc comment says why.

### 11.2 No raw `new`, `delete`, `malloc` or `free` in C++

Objects live on the stack, in containers, or in a `std::unique_ptr` from `std::make_unique`. A
shared owner is `std::shared_ptr` only when two owners are real, and its comment names them.
Placement `new` is `std::construct_at`. Memory that a C interface allocates and wants back is
returned through that interface's own release function, held by an owner as in 11.1.

### 11.3 Views

A reference for what must exist; a pointer for what may be absent; `std::span<T>` for a run of
elements; `std::string_view` for text that is read and not kept. A view never outlives what it
views. A member that is a view or a raw pointer says in its comment who owns the target and why it
lives long enough.

### 11.4 Module state lives in objects

A module's state is an object: created by the code that uses the module, passed by reference to
what needs it, destroyed when its owner goes. New code adds no mutable namespace-scope variable, no
function-local `static` that holds state, and no singleton (`instance()`).

The one exception is a C callback that carries no user pointer (a signal handler, the crash handler,
an `atexit` function): it may reach its state through one pointer in an anonymous namespace, named
`g_<what>`, whose comment says which callback needs it and who sets it.

Existing file-global state (`g` and its 1,125 uses in 15 files) moves into objects module by module,
each with characterisation tests first (section 24). Code you add to such a file does not add to
`g`: it takes what it needs as parameters.

### 11.5 Classes

- A constructor with one argument is `explicit`.
- An overriding function is marked `override` and nothing else; a base class with virtual functions
  has a virtual or protected destructor.
- `struct` for a plain aggregate with no invariants and no private members; `class` for everything
  else. Public data members are for aggregates only.
- Inside a class: `public:` (nested types and aliases, constants, constructors and destructor, the
  member functions grouped by purpose, in the order a reader needs them), then `protected:`, then
  `private:` (nested types, private functions, data members).
- A destructor that does work (joins a thread, closes a device) says in its comment what it waits
  for. A destructor never throws.

### 11.6 Threads

1. A type used by more than one thread says, in its comment, which thread calls which function.
2. Every member shared between threads names its guard (7.4).
3. A lock is `std::lock_guard`, `std::scoped_lock` or `std::unique_lock`, in the narrowest block.
   Never a bare `lock()` and `unlock()`.
4. A wait on a condition variable always has a predicate.
5. A notify comes after the lock is released, unless the order matters, and then a comment says
   why.
6. An atomic that does not need sequential consistency names its memory order and says why.
7. A thread is joined by the destructor of the object that started it (`std::jthread` where the
   target toolchains have it), and that type is move-only or neither copyable nor movable.

---

## 12. Functions and control flow

### 12.1 Shape of a body

Guard clauses first (12.6), then the values the work needs, then the work in groups each with its
opening comment, then the result.

### 12.2 Size

At most 60 lines from `{` to `}`, at most five parameters (more go into a struct), at most three
nested levels of `if`, `for`, `while` or `switch`. Past a limit, split on the seams the group
comments already mark: the comments become function names. The exceptions are a `switch` that
dispatches, where every case is a label and one statement, and the hot paths of section 21.

### 12.3 Parameters

- Order: the object acted on, then the inputs, then the outputs, then the optional ones.
- Small values (up to 16 bytes) by value; others by `const&`; a value the function keeps by value
  and `std::move`.
- A pointer parameter means "may be absent"; if it cannot be null, it is a reference.
- An output is the return value when it can be (a struct, `std::optional`, `Result<T>`). An output
  parameter is a non-const reference, last, and documented.
- A flag that switches two behaviours is two functions, or an `enum class` with two named values:
  `draw(true)` says nothing at the call.

### 12.4 Return values

- A function that can fail is `[[nodiscard]]` and says how in its type (13.3).
- Never return a reference or a view into a local, or into a container the caller may change.
- Return from the middle only through a guard clause or a `case`.

### 12.5 Lambdas

A lambda that is stored (a callback, a thread body, a deferred task) lists its captures:
`[this]`, `[this, frame = m_frame]`. Never `[&]` or `[=]` there; a stored lambda may outlive the
frame. `[&]` is for a lambda used and gone before the function returns (a predicate). A lambda of
more than three lines is a named function, unless it is a thread body or a callback registration,
which is commented from above.

### 12.6 Guard clauses

A function checks what must hold first and leaves at once when it does not; the rest stays at the
left margin. A guard has a comment, braces and a blank line after it.

```cpp
std::optional<Texture> TexturePool::find(std::uint32_t address) const {
    // Nothing is cached before the first level loads.
    if (m_entries.empty()) {
        return std::nullopt;
    }

    auto it = m_entries.find(address);

    // The game asked for a texture this level never uploaded.
    if (it == m_entries.end()) {
        return std::nullopt;
    }

    return it->second;
}
```

### 12.7 `else`, ternaries, `goto`

No `else` after a branch that ends in `return`, `continue`, `break` or `throw`. A ternary chooses
between two values, fits on one line or wraps as the tool wraps it, and is never nested. No `goto`.

### 12.8 `switch`

1. A `switch` on an `enum class` names every enumerator and has no `default`, so the compiler warns
   when one is added.
2. A `switch` on an integer from data has a `default`, whose comment says what an unlisted value
   means.
3. Every case ends in `break`, `return`, `continue` or `[[fallthrough]];` with a comment on why.
4. A case that declares a variable is wrapped in braces, opened on the label's line.

### 12.9 Loops

A range-based `for` when the index is not needed. A counting loop names its counter `n` (or a
coordinate) and gives it the type of what it is compared to. An endless loop is `for (;;)` with a
comment saying how it ends.

---

## 13. Errors

### 13.1 Input is not trusted

The port reads what it does not control: the player's disc and extracted data, settings files, mod
manifests and scripts, the game's own memory. A value from any of them can be anything. No input
ever makes the port read or write out of range, divide by an unchecked value, or reach undefined
behaviour. For a value it cannot use, the code does one of three things and says which in a
comment: rejects the input with a message, uses a documented fallback, or counts and skips it.

### 13.2 Exceptions stay inside a module

Inside a module, a reader may throw when its input is malformed (the asset and media readers already
do): the exception type derives from `std::runtime_error`, its message follows 17.3, and the
module's public function catches it and returns it as a failure (13.3). Exceptions are never used
for control flow, and a destructor never throws.

### 13.3 A public function reports failure in its type

A module's public function that can fail says so in its return type, and is `[[nodiscard]]`:

| Form | When |
|---|---|
| `openrac::Result<T>` (`common/result.h`, arriving with P2-08) | the default for new interfaces |
| `bool` and a `std::string& error` | the existing style; stays valid, and new code beside it may match it |
| `std::optional<T>` | when absence is the only failure and needs no message |

`Result<T>` holds either a `T` or an `openrac::Error`, a struct whose `message` follows 17.3. Its
members are named as `std::expected`'s are (`has_value()`, `explicit operator bool`, `value()`,
`operator*`, `operator->`, `error()`), and `Result<void>` holds no value, so moving to
`std::expected` when the floor allows it is a rename. Until it lands, write the `bool` form.

```cpp
// Reads the sound bank at `path`. On failure, `error` says what was wrong and where.
[[nodiscard]] bool load_bank(const std::filesystem::path& path, Bank& bank, std::string& error);

// The same, in the form new interfaces use once common/result.h exists.
[[nodiscard]] Result<Bank> load_bank(const std::filesystem::path& path);
```

### 13.4 No exception crosses a C boundary or leaves the frame loop

Every `extern "C"` function implemented in C++ catches everything and reports it: on the game side
through `openrac_game_crash` (`game_host.h`), on the host side through the log and the function's
failure value. Until P2-08 brings the wrapper, write it out:

```cpp
extern "C" int openrac_sound_play(int voice, int sound) {
    try {
        return engine().play(voice, sound) ? 1 : 0;
    } catch (const std::exception& failure) {
        openrac_game_crash(failure.what());
    } catch (...) {
        openrac_game_crash("sound: unknown exception");
    }
    return 0;
}
```

The program's frame loop calls modules only through functions that do not throw. A mod's code
catches its own (MODS.md, section 11).

### 13.5 Ending the program

Only the program (`game/common`'s `main` and the apps) ends the program, through `log::fatal`. A
library module never calls `exit`, `abort` or `std::terminate`, and never prints instead of
reporting. `assert` is for a condition the calling code controls, never for input.

### 13.6 Every call that can fail is checked

Every file operation (`std::filesystem` through its `std::error_code` overloads), SDL call, Vulkan
`VkResult`, `dlopen`, and C library call that reports failure has its result checked.

---

## 14. The C boundary

### 14.1 The contracts

The C headers are the contracts between the C the games are built from and the C++ that runs them.
They stay C.

| Header | Directory | What it is |
|---|---|---|
| `guest.h` | `port/runtime/include/openrac/` | game memory and the calls translated C makes (hostgen writes against it) |
| `ee.h`, `ee_ops.h` | `port/runtime/include/openrac/` | the EE's registers and operations as translated C uses them |
| `ps2float.h` | `port/runtime/include/openrac/` | the console's float arithmetic, for translated C |
| `game_host.h` | `port/game/common/include/openrac/` | what a game gives the host program and asks of it (`openrac_front_end`, `openrac_game_crash`) |
| `game_lib.h` | `port/game/common/include/openrac/` | the library replacements the games call (`libraries.tsv` binds them) |
| `script_binding.h` | `port/game/common/include/openrac/` | a game's binding for script mods |
| `sound_engine.h` | `port/game/common/include/openrac/` | the sound engine the games' C drives |
| `openrac_mod.h` | `sdk/include/openrac/` | the mod C ABI ([MODS.md](../../docs/port/MODS.md)) |

`memory.h` and `elf.h`, in the same directory as the runtime's contracts, are C++ headers of the
runtime (`namespace openrac::runtime`), not contracts: the rules of this section do not apply to
them, the rest of the page does.

### 14.2 The rules for a contract header

- It compiles as C11 and as C++20. Its declarations are inside an `extern "C"` block for C++
  (`#ifdef __cplusplus`), its own or that of the contract header that includes it (`ee_ops.h` is
  included by `guest.h`). It keeps its `#ifndef OPENRAC_<NAME>_H` include guard.
- C types only: fixed-width integers, pointers, structs of them, function pointers. No C++ type, no
  exception, no ownership passed without a function to give it back.
- Changing a contract is not a refactor (the restructure plan never changes them). A change is its
  own `feat` or `fix` commit; `openrac_mod.h` follows its versioning rules (MODS.md) and its hook
  ids never change.

### 14.3 Who implements, who includes

- C++ implements a contract in the file named for it: `guest.cpp` for `guest.h`, `ee.cpp` for
  `ee.h`, `sound_engine.cpp` for `sound_engine.h`, and `game_host.cpp` for `game_host.h` (P2-08
  gathers the host's side of it there from `main.cpp`, which defines `openrac_game_crash` today).
  The library replacements of `game_lib.h` are one file per library in `game/common/lib/`.
  `extern "C"` definitions appear only in those files and in tests. The check in CI holds new files
  to it; the sites elsewhere today (`frontend.cpp`, `main.cpp`, `fixes.cpp`, `mods.cpp`,
  `mod_scripts.cpp`, `platform/window.cpp`) move with P2-08.
- C++ never includes a header hostgen generates. What it needs from the game comes through a
  contract.
- Game C (generated, hand, host C) never includes a C++ header.
- Data crosses as the contract's C types; a game address is converted to a host pointer only through
  `guest.h`'s helpers.

---

## 15. Per-game knowledge

Shared code (everything outside `port/game/<game>/` and `port/game/<game>-<version>/`) knows no
game:

- it never compares a game id or version string, and never branches on which game runs;
- it never holds a game's address, symbol, structure offset or constant.

What differs per game lives in that game's tables: for C, `openrac_front_end` (`game_host.h`),
`libraries.tsv`, `hostgen.json` and per-disc headers such as `rac1_version.h`; for C++, one
`host::GameProfile` per game version (arriving with P2-03 for rac1 and P3-02 for the rest). A shared
function that needs a game fact takes it from the table or the profile. The branches on game ids in
`frontend.cpp` and the addresses in `viewer/game_state.cpp` move into the profiles with P2-03; new
code adds none.

---

## 16. Configuration

- An `OPENRAC_*` environment variable is read through `common/env.h` (arriving with P1-03): one table
  of name, type, default and one-line description, which a test checks against
  [DEBUGGING.md](../../docs/port/DEBUGGING.md). Once it lands, `getenv` appears only in
  `common/env.cpp`.
- Until then, a new variable is read once, in its module, documented in DEBUGGING.md, and named
  `OPENRAC_` and what it controls, in upper snake case.
- A choice a player makes is a setting (the settings module and the launcher), not an environment
  variable. Environment variables are for development, debugging and tests.

---

## 17. Logging and messages

### 17.1 One log

Diagnostics go through [`common/log.h`](../common/log.h): `log::debug`, `log::verbose`,
`log::info`, `log::warn`, `log::error` take a `std::format` string checked by the compiler;
`log::fatal` ends the program (13.5). A line that would come every frame goes through `log::Capped`.

| Level | Use |
|---|---|
| `Error` | a failure the player or developer needs to see: a file that cannot be read |
| `Warn` | something wrong that the port works around |
| `Info` | a notable event: a level loaded, a mod loaded, the renderer chosen |
| `Verbose` | more about what the game does (`OPENRAC_LOG=verbose`) |
| `Debug` | each frame's detail (`OPENRAC_LOG=debug`) |

`printf`, `std::cout` and `std::cerr` are not used in C++ outside tests and a program's main file,
whose output (a listing, a report) is the reason the program exists. C files keep their `printf`
(the game's C and its host C).

### 17.2 What never goes into a log

Game data, bytes of the disc, or the player's paths beyond what `log::tidy` leaves (the home folder
reads `~`).

### 17.3 Writing a message

- Say what happened, to what, with the values: `cannot read bank {}: {} bytes, header wants {}`.
- Start with a lower-case word, no full stop at the end.
- Addresses in hexadecimal at their width (`{:08x}`); counts in decimal.
- Use the names the code and the game use, not a paraphrase.
- No exclamation marks, no apology, no "please", no "failed to" where "cannot" fits.
- A message a player can act on says what to do: `no memory card folder: cannot make {} ({})`.

---

## 18. Layering

The modules form layers. A module includes and links only modules at or below its own layer:

```
common  <  runtime  <  platform, assets, audio, media  <  renderer (gl, vulkan)
        <  scene  <  enhanced  <  host (game/common)  <  game/<id>  <  apps (viewer, extractor, tools)
```

- Lower never includes higher.
- Modules on one layer do not include each other unless the checker's allow-list names the edge.
- `enhanced` plugs into `renderer` and `scene` through interfaces those modules own and it
  registers with; `renderer` never includes `enhanced/` (P2-07 removes today's include).
- Apps depend on libraries, never the other way round.
- A third-party library belongs to one module (4.5): SDL3 to `platform` (and the program), the
  graphics APIs to `renderer`, Luau to the host's script mods.

The layering checker ([`port/tools/layering/`](../tools/layering/README.md)) reads every include,
fails on an edge the diagram does not allow, and lists today's violations as known exceptions to be
removed, not added to. Its `rules.tsv` holds the modules, the layers and the edges allowed within a
layer.

---

## 19. Build

- One CMake library target per module, `openrac_<module>`, with its sources listed (no new globs).
- Dependencies match the layering: `PUBLIC` for what the module's headers expose, `PRIVATE` for
  the rest.
- Every target the port compiles calls `openrac_warnings(<target>)`.
- Compile options are set per whole target, never per file.
- Tests are registered through one helper, `openrac_add_test` for C++ and `openrac_add_c_test` for
  C ([cmake/Tests.cmake](../cmake/Tests.cmake), which documents their arguments). A test target is
  `<subject>_test` and the ctest name is `<subject>`. A C test names the game C's options it needs
  (`NO_STRICT_ALIASING`, `WRAPV`, `NO_FP_CONTRACT`, `MATH`) rather than spelling the flags.
- A new module brings its README (7.7), its row in the port README's "What is here" and its edges
  in the layering checker's allow-list.

---

## 20. Tests

### 20.1 The framework

[`tests/check.h`](../tests/check.h) is the framework: `CHECK(condition)` records a failure and goes
on, and `main` returns `openrac::test::result()`. Do not add another framework; extend this one in a
commit of its own.

### 20.2 A test file

Laid out like a source file (9.4): the licence lines and a file comment saying what it tests and
where its expected values come from; the includes; an anonymous namespace with the helpers and the
test functions, each documented; `main`, which runs them in the order they are written.

### 20.3 One test, one behaviour

A test function checks one behaviour, is named `test_` and that behaviour
(`test_a_full_queue_drops_the_oldest_line`), is at most 60 lines, and its doc comment states the
behaviour in one sentence.

### 20.4 Expected values

Expected values come from documentation, from arithmetic, or from the decompiled C's behaviour,
written as their derivation (`0.75f + 2.25f`, or a constant with its meaning). Never from a retail
program's bytes, and never copied from a run of the code under test, with one exception: a
characterisation test written before a refactor pins today's behaviour, and its file comment says
so (24.2).

### 20.5 What a test never contains

Game code, data or bytes; a path to a disc image or to the player's files; a dependence on the clock,
the number of cores or the order of threads. A check that needs the player's data is a tool, or part
of the regression run ([REGRESSION.md](../../docs/port/REGRESSION.md)), which the maintainers run.

---

## 21. Performance-sensitive code

The inner loops (the renderer's per-draw and per-vertex paths, the mixer, translated code's calls
into `guest.h`) may break a few rules to be fast, only these, and every break is marked:

| Rule | May be broken for | Mark |
|---|---|---|
| 12.2 size and nesting | one dispatch or one fused loop | `Hot path:` line in the doc comment naming the limit |
| 12.6 guard clauses | a loop that tests once at the top | the comment on the test |
| 12.7 ternaries | a branch-free select the compiler would not find | a comment giving the plain form |

Braces, comments, naming, layout and documentation never deviate. A `Hot path:` line names the rule
it breaks and how the cost was measured. `[[likely]]`, `[[unlikely]]` and inlining attributes need a
measurement in their comment; "faster" without a number is not a reason.

---

## 22. Reuse

- **Search before you write.** Before adding a helper, a constant or a reader, search for it by what
  it does. Look first in `common/`, the module you are in, and its neighbours.
- **The rule of two.** The second copy of a piece of logic moves both into one place. Two copies are
  where a fix lands on one and not the other.
- **No abstraction without two users.** A new interface, base class or template is written when its
  second user exists, not before.
- **Put the logic where the data is.** A function that reaches through three members of another
  object belongs to that object.
- **Use what is already resolved.** A value computed earlier is used, not worked out again; a number
  in two places is one constant.

---

## 23. The project's constraints

These come from [AGENTS.md](../../AGENTS.md), [SOURCING.md](../../docs/policy/SOURCING.md) and the
[port README](../README.md), gathered so that a review can check them in one place.

1. **Licence lines.** A C++ file starts with `// SPDX-License-Identifier: GPL-3.0-or-later` and
   `// Copyright (c) 2026 the OpenRAC contributors`. A C file has the same two lines in its comment
   form (`/* SPDX-License-Identifier: GPL-3.0-or-later` then ` * Copyright ...`). Code adapted from
   a permissively licensed project keeps its licence's identifier, says so in the file comment, and
   is credited in [THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md).
2. **No game content.** No game code, data or bytes in a source, a test, a log or a comment. A tool
   that reads a game file prints counts, never the content.
3. **No Sony SDK.** Nothing written from Sony's SDK source, samples or headers, or from leaked or NDA
   material. The library replacements are written from what the game's own code shows and what is
   publicly known of each API. Reviewers look twice at code that arrives suspiciously complete or
   with SDK-style names.
4. **`games/` is not touched** from port work: not for names, formatting or includes. The game's C
   reaches the port through hostgen, and a change to it goes to its decompilation.
5. **Generated files are regenerated, never edited**: change the generator.
6. **Names.** The third-party NTSC decompilation of Ratchet & Clank is "the NTSC decomp". No
   author's name, handle or address. Nobody who asked to be removed is named, and nothing removed at
   someone's request comes back.
7. **Credit what you reuse**: the project, the file or function, and its licence.

---

## 24. Migrating existing code

Most of the port was written before this page. It reaches the rules gradually, never in a sweep
over the tree (the restructure plan forbids mass renames and reformatting).

### 24.1 The migrations and who does them

| From | To | When |
|---|---|---|
| `static` at namespace scope | anonymous namespace | when a file is restructured, in a `refactor(port)` commit |
| file-global `g` state | a state object (11.4) | one module per package: `renderer/vulkan` with P3-03, the rest as each module is restructured |
| `getenv` | `common/env.h` | P1-03 (runtime, platform, renderer), P2-09 (game/common, vulkan) |
| `extern "C"` outside contract files, no catch-all | the contract's file, the wrapper | P2-08 |
| game-id branches and addresses in shared code | `GameProfile` | P2-03, P3-02 |
| test stanzas written out in `cmake/*.cmake` | `openrac_add_test`, `openrac_add_c_test` | when the file is restructured |
| raw `new`/`delete`, `printf`, missing doc comments | owners, the log, documentation | when the function is touched |

### 24.2 Characterisation tests first

Before restructuring code with thin coverage, add tests that pin its current behaviour, in a commit
of their own, passing before and after. Their file comment says they pin behaviour for a refactor.

### 24.3 Proving that a pass changed nothing

A layout or comment pass does not change the compiled code, and you can prove it: build before and
after with debug information off, and compare the objects.

```sh
cmake -S port -B /tmp/before -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-g0 -DCMAKE_C_FLAGS=-g0
cmake --build /tmp/before
# ... the pass ...
cmake -S port -B /tmp/after -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-g0 -DCMAKE_C_FLAGS=-g0
cmake --build /tmp/after

# Every object outside the tests must be identical: CHECK writes __LINE__ into the test objects.
(cd /tmp/before && find . -name '*.o' ! -path '*test*' | sort) | while read -r object; do
    cmp -s "/tmp/before/$object" "/tmp/after/$object" || echo "DIFFERENT: $object"
done
```

No `DIFFERENT` line means the compiler saw the same program. A refactor that moves code changes
objects; it proves itself instead with ctest, hostgen's generated C unchanged (the hashes of the
`gen/*.c` files before and after), and, for the host program, the regression comparison the
maintainers run ([REGRESSION.md](../../docs/port/REGRESSION.md)).

### 24.4 Changed lines

The rules of this page hold on every line you change. CI reports clang-format's changes on changed
lines (`check_conventions.py --touched`) without failing yet, so work already in flight can land; it
becomes a failure once the restructure's phase 1 has landed (PROGRESS.md records the switch). Do not
reformat the untouched lines around your change: a formatting pass is a commit of its own.

---

## 25. Self-check before you report done

Run this against your own diff. Every item is countable. Do not report the work as done until every
line reads yes.

**Machines**

1. `clang-format --dry-run -Werror` is silent on every file you added, and on the lines you changed?
2. `check_conventions.py --since <base> --touched` and `layering.py` report nothing to fix?
3. clang-tidy reports nothing on the C++ you added or changed?
4. The build has no new warning, and ctest passes (with the Python tests for the tools you touched)?

**Comments and documentation**

5. Every file you added has the licence lines and a file comment?
6. Every function, type, member and constant you added has a doc comment in the form of section 7?
7. Every branch, guard, `case` and deciding ternary you added has a comment?
8. Every numeric literal other than 0 and 1 has a name or a comment, and every fact about the game a
   source tag?
9. Every `//` comment inside a body is one sentence of 20 words or fewer, or a block of 8 lines or
   fewer?
10. Zero em or en dashes? (`git diff -U0 | perl -CSD -ne 'print if /\x{2014}|\x{2013}/'`)
11. Zero banned words from 6.7? (`git diff -U0 | grep -niE 'furthermore|moreover|additionally|robust|comprehensive|seamless|simply|basically|ensure|utilize|leverage'`)
12. Zero comments that restate their line; zero `TODO`, dates or author names?
13. Every comment you did not break is byte for byte unchanged?
14. A new module has its README with the sections of 7.7, and the docs that describe what you moved
    are updated?

**Structure**

15. Names as section 8 has them?
16. Files laid out as 9.3 and 9.4; includes in their groups; nothing includes a higher layer, an
    `_internal.h` of another module, or a generated header?
17. File-local code in an anonymous namespace (C++)?

**Code**

18. Every resource owned by an RAII type; no raw `new`, `delete`, `malloc` or `free`; no new
    file-global state?
19. Every public function that can fail says so in its type and is `[[nodiscard]]`?
20. Every `extern "C"` function in C++ is in its contract's file and catches everything?
21. No game id, address or per-game branch in shared code?
22. Logging through `common/log.h`; no new `getenv` outside its one read (or `common/env.h`)?
23. Functions of 60 lines, five parameters and three nested levels at most, or a `Hot path:` line?
24. Guard clauses first, no `else` after `return`, no nested ternary, no `goto`, every `switch` as in
    12.8?
25. Did you search for an existing helper before writing one? Does any logic appear twice?

**Tests and proof**

26. Each new test checks one behaviour, has a `test_` name and a doc sentence, and derives its
    expected values?
27. For a refactor or formatting pass: the proof of 24.3, and what you could not run said in the
    report?

A documentation pass that adds more words than understanding has failed, however correct each
comment is.
