# C++ coding conventions for the runtime

The rules for every C++ file under `runtime/`: `src/ps2`, `src/sys`, `src/host`, `src/app` and
`tests`. The runtime models hardware, and a model is only as good as the next person's ability to
check it against the hardware documentation. So the code is written to be read line by line by
someone who knows the PlayStation 2 and has never seen this file.

Anyone who touches the runtime follows this page: people, Claude, Codex, any agent. It builds on
[CONTRIBUTING.md](../../CONTRIBUTING.md), [AGENTS.md](../../AGENTS.md) and the
[runtime README](../README.md), and none of them is loosened here. After a context compaction, read
this page again from the top instead of working from a summary of it.

Every rule below can be counted. If you cannot count your own compliance against your diff, you have
not complied. The list at the end ([section 17](#17-self-check-before-you-report-done)) is the
count.

## Which rules apply to which work

| Work | What the rules cover |
|---|---|
| New code | Every rule. |
| A function or file you change for another reason | Every rule on the lines you touch, plus the function's doc comment. Leave the rest of the file as it is. |
| A formatting pass over existing code | Layout and comments only. [Section 16](#16-reformatting-existing-code) says how. |

Commit types follow CONTRIBUTING.md: `style(runtime)` for layout alone, `docs(runtime)` for
comments alone, `refactor(runtime)` for renames and moves. A commit never mixes layout or comments
with a change of behaviour.

## The layout is the tool's

[`runtime/.clang-format`](../.clang-format) is the reference for layout: indentation, wrapping,
braces, spacing, include order. This page explains the choices and covers what a tool cannot see.
When the file and this page disagree about layout, the file wins and this page gets fixed.

It needs clang-format 19 or newer. It was written and checked with Apple clang-format 21.0.0
(`xcrun clang-format --version`). On this machine the binary is `xcrun clang-format`; elsewhere it
is usually `clang-format`.

From the `runtime/` directory:

```sh
xcrun clang-format -i src/ps2/dma.cpp                  # apply it to a file you are changing
xcrun clang-format --dry-run -Werror src/ps2/dma.cpp   # check it; prints nothing when clean
```

---

## Hard rules

These ten are not negotiable. Each one is countable. Count them against your own diff before you say
the work is done.

### 1. The layout is what `.clang-format` produces

Four spaces, never a tab. At most 100 columns. The tool reports nothing on the files you touched.
([Section 1](#1-layout))

Count: tab characters, lines over 100 columns, and the output of
`clang-format --dry-run -Werror` on your files. All three are zero.

### 2. Every body has braces

Every `if`, `else`, `for`, `while`, `do` and `switch` body is a `{ }` block with its statements on
their own lines. No exceptions, not for a `return`, not for a `break`.

Wrong:

```cpp
if (!found) return kNoSuchEntry;
```

Right:

```cpp
if (!found) {
    return kNoSuchEntry;
}
```

Why: a body of one line becomes a body of two the first time somebody adds a log line, and then the
second line runs on every path while the indentation says it does not. The tool inserts the braces,
so the rule costs nothing to follow and nothing to check.

### 3. Every declaration is documented

A file, a type, a function, a method, a member variable and a group of constants each has a doc
comment in the `/** */` form, as [section 4](#4-documentation-blocks) lays out. The only exceptions
are `= delete` and `= default` special members.

Count: declarations in your diff with no doc comment. Zero.

### 4. Every branch and every unexplained number says what it is or why it is there

Each `if`, `else if`, `case`, guard clause and deciding ternary has a comment. Each numeric literal
other than 0 and 1 has a name, or a comment that names the field or fact it stands for. Each group of
statements opens with a comment unless the lines are plain bookkeeping.
([Section 3](#3-comments))

Count: branches with no comment; literals with no name or comment; statement groups with no opening
comment. All zero.

### 5. A comment says why. It is one sentence, or it is a note block

A `//` comment is one sentence of 20 words or fewer. A longer thought is a `/* */` note block of at
most 8 lines, or it goes in the doc comment. The comment says what the code cannot say: why it is
here, which hardware fact it follows, how that fact is known. It never repeats the line below it.

Count: `//` comments over 20 words; `//` runs of three lines or more; note blocks over 8 lines;
comments that restate their line. All zero.

### 6. Never rewrite a comment you did not break

If a comment is correct and follows these rules, leave it byte for byte as it is. Reworded comments
are noise in the diff and they get reverted. Touch a comment only when your change made it wrong, or
when it breaks a rule on this page.

### 7. No em dashes and no en dashes, anywhere

Not in comments, strings, doc blocks, this repository's Markdown or commit messages. Use a comma, a
period, a colon or parentheses. A plain hyphen between numbers (`bits 0-14`) is fine.

### 8. Comments sound like a person wrote them

Short, flat, specific. No filler, no hedging, no reassurance, none of the words on the list in
[section 3.8](#38-voice). Write the way you would explain the line to a colleague who is looking at
the same screen.

### 9. The project's constraints hold in every file

The first two lines of every source file are the SPDX line and the copyright line. No game code,
data or bytes in a source or a test. Nothing written from Sony's SDK source, samples or headers. The
NTSC decompilation is called "the NTSC decomp", and nobody who asked to be removed is named.
([Section 15](#15-the-projects-constraints))

### 10. A formatting pass changes no behaviour

If you only changed layout and comments, the object files of `src/` built before and after are
byte-identical, and the tests pass. Say how you checked.
([Section 16](#16-reformatting-existing-code))

---

## Contents

1. [Layout](#1-layout)
2. [Blank lines](#2-blank-lines)
3. [Comments](#3-comments)
4. [Documentation blocks](#4-documentation-blocks)
5. [File layout](#5-file-layout)
6. [Declarations](#6-declarations)
7. [Naming](#7-naming)
8. [Functions](#8-functions)
9. [Control flow and guard clauses](#9-control-flow-and-guard-clauses)
10. [Errors, logging and messages](#10-errors-logging-and-messages)
11. [Threads and resources](#11-threads-and-resources)
12. [Performance-sensitive code](#12-performance-sensitive-code)
13. [Reuse](#13-reuse)
14. [Tests](#14-tests)
15. [The project's constraints](#15-the-projects-constraints)
16. [Reformatting existing code](#16-reformatting-existing-code)
17. [Self-check before you report done](#17-self-check-before-you-report-done)

---

## 1. Layout

All of this section is applied by the tool. It is written down so that you can predict what the tool
does and so that you can write code that needs no fixing.

The samples in this page show their subject. Samples about layout leave out the comments that
section 3 would require, to stay short. Names, numbers and measurements in the samples are made up
to show the form; the register layouts and hardware facts in them are the documented ones.

### 1.1 Indentation

Four spaces per level. Tabs never appear in a source file. A line that continues a statement goes in
one more level (four spaces) than the statement.

Four and not two because the interpreter and rasteriser code nests three and four levels deep, and
two spaces make a `switch` inside a `for` inside an `if` hard to follow by eye.

Wrong, two spaces (or a tab, which is worse):

```cpp
void Vu::reset() {
  for (auto& reg : vf) {
    reg.fill(0);
  }
}
```

Right:

```cpp
void Vu::reset() {
    for (auto& reg : vf) {
        reg.fill(0);
    }
}
```

### 1.2 Line length

At most 100 columns, counting the indentation and any trailing comment. The limit holds for code,
comments, doc blocks and strings. A string literal that cannot fit is split in two literals on two
lines; the compiler joins them.

A declaration is not squeezed to fit. If a constant, a member or a prototype does not fit in 100
columns, the tool wraps it as in 1.4 and 1.5, and you accept the result.

### 1.3 Braces

The opening brace is on the line of the statement it belongs to; the closing brace is alone on its
line, at the statement's indentation, or followed by `else`, `while` of a `do`, or `;`.

```cpp
if (up & kInterruptBit) {
    on_step(at, up, low);
} else if (pc == stop_at) {
    running_ = false;
} else {
    step();
}
```

A function with an empty body is `{}` on the signature's line. A function defined inside its class
whose body is one short statement, an accessor, stays on one line:

```cpp
class Vu {
public:
    /** True once the program has stopped. */
    bool stopped() const { return !running_; }
};
```

Nothing else shares a line with its body. That includes `if`, `for`, `while`, free functions and
out-of-class member functions. The only other exception is a lambda passed as an argument (1.8).

### 1.4 Wrapped calls and parameter lists

Write a call on one line when it fits. When it does not, break after the opening `(` and put the
arguments on one line at the next indent level. If they still do not fit there, put one argument on
each line. The closing `)` goes alone on its own line at the statement's indent.

The same holds for a parameter list in a declaration. There is no alignment under the opening
parenthesis, so renaming a function never forces a re-indent of its arguments.

Right. The arguments fit on one line at the next indent, so they stay together:

```cpp
void put_entry(
    Machine& m, u32 at, const std::string& name, bool directory, u64 size, std::time_t changed
);
```

Right. They do not fit there, so each has a line of its own:

```cpp
std::fprintf(
    stderr,
    "frame %d: pc %08x ra %08x, %llu primitives\n",
    frame + 1,
    machine.ee.pc,
    static_cast<u32>(machine.ee.gpr[31].lo),
    static_cast<unsigned long long>(machine.graphics.gs.stats.primitives)
);
```

If a call wraps only because one argument is a long expression, give the expression a name first.

Wrong:

```cpp
put_entry(m, table + static_cast<u32>(n) * kEntryBytes, entries[n].name, entries[n].directory, entries[n].size, entries[n].changed);
```

Right:

```cpp
const Entry& entry = entries[n];
u32 at = table + static_cast<u32>(n) * kEntryBytes;

put_entry(m, at, entry.name, entry.directory, entry.size, entry.changed);
```

### 1.5 Wrapped expressions and ternaries

A long condition or sum breaks before an operator, never after it. The operands line up under the
first one, so the operators stand in a column where the eye looks for them.

```cpp
bool feeds_itself = (need != 0 && (reads.intersects(written) || reads.intersects(depth)))
                    || depth.intersects(written);
```

A ternary that does not fit on one line starts its two continuation lines with the `?` and the `:`.

```cpp
u32 colour = alpha_blend_is_enabled && source_alpha_is_nonzero
                 ? blend(source_colour, destination_colour, alpha)
                 : source_colour;
```

If an expression needs more than three lines, it is doing too much. Give its parts names.

### 1.6 Spaces

One space on each side of a binary operator and of `=`. No space after a unary operator, `!`, `~`,
`*`, `&`, `++`. No space between a function name and `(`, one space between a keyword and `(`
(`if (`, `for (`, `while (`, `switch (`). One space after a comma, none before. No space inside
`( )`, `[ ]` or `< >`. In braced initialisers no space inside the braces: `u64{1}`, `{0, 0, 0, 1}`.

A pointer or reference marker hugs the type, as it does today: `u8* data`, `const Image& image`.
Two spaces separate code from a trailing comment, and the tool lines up the trailing comments of
neighbouring lines.

### 1.7 Namespaces, classes and access specifiers

A namespace is not indented. Its closing line names it. Namespaces that open on consecutive lines
stay together; the first declaration inside the innermost one follows a blank line.

```cpp
namespace sys {
namespace {

/** The numbers sceMcSync gives for the function that finished. */
enum : int {
    kGetInfo = 1,
};

}  // namespace

}  // namespace sys
```

`public:`, `protected:` and `private:` sit at the class's own column, in that order, once each.
Members are indented one level under them. There is no blank line directly after a specifier.

```cpp
class Drawing {
public:
    explicit Drawing(ps2::Graphics& graphics);

private:
    ps2::Graphics& graphics_;
};
```

### 1.8 `switch`, `case` and lambdas

`case` and `default` are indented one level under `switch`; a case's statements go one more in.

```cpp
switch (flg) {
    case 0:
        count = nreg ? nreg : 16;
        return read_packed(tag, count);

    case 1:
        count = nreg ? nreg : 16;
        return read_reglist(tag, count);

    default:
        return read_image(tag);
}
```

A `case` label is alone on its line and its statements follow, one to a line. The label may carry a
trailing label comment naming it (3.9). A case body of two lines or more is followed by a blank
line. A case that needs a local variable is wrapped in braces, and the braces open on the label's
line:

```cpp
case kPresent: {
    ps2::Image buffer = std::move(spare_picture_);
    show(std::move(buffer));
    break;
}
```

A lambda passed as an argument may stay on one line if its body is one short statement and the whole
line fits. In every other place (a lambda stored in a variable, one with two statements) it is
written out, with its body indented from the line its signature starts on.

```cpp
std::sort(list.begin(), list.end(), [](const Entry& a, const Entry& b) { return a.size < b.size; });

auto apply = [this](u32& s) {
    s = (s & ~0x30u) | q_flags_ | (q_flags_ << 6);
};
```

### 1.9 Constructor initialisers

If the initialiser list fits on the signature's line, it stays there. If not, the colon starts the
next line, indented one level, and each initialiser has a line of its own.

```cpp
Gs::Raster::Raster(Gs& gs)
    : gs_(gs),
      thread_([this] { loop(); }),
      queue_limit_(kMaxQueuedBatches),
      idle_(true) {}
```

Initialisers are written in the order the members are declared; the compiler warns when they are
not. A member that has the same value in every constructor gets a default member initialiser in the
class instead.

### 1.10 Hand-laid tables and `clang-format off`

A table the tool would spoil, for instance rows of a microprogram or of a packet laid out so that the
columns mean something, is fenced with the tool's own directives and a reason:

```cpp
// clang-format off: one GIF quadword per row, so the fields line up with the format.
const u64 packet[] = {
    1 | (u64{1} << 15) | (u64{3} << 60), 0x551,
    0,                                   0,
};
// clang-format on
```

This is the only way to switch the tool off. The reason is mandatory. The directive never covers
more than one table, and never covers a function body. Tables of numbers that can be packed (a plain
list with no trailing comma) need no directive.

---

## 2. Blank lines

Blank lines are the other half of the layout. The tool keeps at most one in a row and removes them
at the start of a block, and it puts one between definitions. The rest you place.

### 2.1 In a file

| Between | Blank lines |
|---|---|
| The licence lines and the file comment | 1 |
| The file comment and `#pragma once` or the first `#include` | 1 |
| Two include groups | 1 |
| The last include and the first declaration | 1 |
| Two functions, two types, a function and a type | 1 |
| A doc comment and the declaration it documents | 0 |
| A `namespace` line and the first declaration inside it | 1 (0 if another `namespace` follows) |
| The last declaration inside a namespace and its closing line | 1 |

There are never two blank lines in a row, anywhere.

A doc comment sits directly on its declaration, with no blank line. This is deliberate: a blank line
detaches a comment from what it describes, to the reader and to the documentation tools.

### 2.2 In a class

One blank line between member functions. One blank line between members that have a doc comment
above them. A run of members that each carry a trailing comment (see 4.5) has no blank lines inside
it and one at each end. One blank line between the public, protected and private parts is added by
the tool.

### 2.3 In a function body

A body is made of groups: a few lines that do one step. Blank lines separate the groups.

1. One blank line before each group that starts with a comment. A comment line and the statements it
   describes have no blank line between them.
2. One blank line after the closing brace of an `if`, `for`, `while`, `switch` or `do`, unless the
   next line is another `}`, an `else`, or the `while` of a `do`.
3. One blank line after the last local declaration of a block of declarations, before the first
   statement. Declarations that belong together are not separated from each other.
4. One blank line before the final `return` of a function when more than three lines come before it.
5. No blank line directly inside a `{`, or directly before a `}`.
6. No blank line between two `case` labels that share one body, or between cases whose bodies are one
   statement each. One blank line after a case body of two statements or more, unless the closing
   brace of the `switch` comes next.

Wrong:

```cpp
u64 Vu::advance(u64 instructions) {
    fp::want_toward_zero();
    u64 count = 0;
    while (running_ && count < instructions) {
        step();
        count++;
    }
    return count;
}
```

Right:

```cpp
u64 Vu::advance(u64 instructions) {
    // The unit computes with the console's rounding while it runs (documented).
    fp::want_toward_zero();

    u64 count = 0;

    // Stop early if the program ends or sets the E bit before the budget is spent.
    while (running_ && count < instructions) {
        step();
        count++;
    }

    return count;
}
```

### 2.4 After a guard clause

A guard clause is a braced `if` whose body ends in `return`, `continue`, `break` or `throw`. A blank
line follows its closing brace. Because each guard also carries a comment, a run of guards reads as
a list of rules:

```cpp
// No card in this slot: the answer is the library's "no card" result.
if (!present(m, port, slot)) {
    finish(m, kOpen, kNoCard);
    return;
}

// The game named something outside the card's folder.
if (path.empty()) {
    finish(m, kOpen, kNoSuchEntry);
    return;
}

// Three files at most are open at once on the console.
if (m.card.open.size() >= 3) {
    finish(m, kOpen, kTooManyOpen);
    return;
}
```

---

## 3. Comments

### 3.1 The three forms

| Form | Used for |
|---|---|
| `/** ... */` | A doc comment: file, type, function, member, group of constants. See section 4. |
| `//` | A one-sentence comment inside a body, a trailing comment on a table line, a section heading. |
| `/* ... */` | A note block: one explanation, inside a body, that does not fit in one sentence. |

Nothing else. No `///`, no `/*!`, no `#if 0`, no commented-out code (it is in git).

A `//` comment starts with `// ` (a slash pair and one space). Above a line it is a complete
sentence: capital letter, full stop. Trailing labels are the exception (3.9). A note block opens
with `/*` alone on its line, continues with lines starting ` * `, and closes with ` */` alone:

```cpp
/*
 * A branch tests the integer register as it was before the pair ahead of it ran. If
 * that pair read the flags into the register, or the branch had to wait, it sees the new
 * value instead (measured).
 */
u16 tested = branch_vi(reg);
```

A note block sits directly on the code it explains, with no blank line. It is at most 8 lines and 100
columns. If the idea needs more, it belongs in the function's doc comment or in a file under `docs/`.

### 3.2 Where a comment is required

A comment is required in each of these places. Nothing else is required to have one (see 3.4).

1. Above every `if`, and above every `else if` and `else` that is not the plain complement of an `if`
   whose comment already explains both. An `else if` cannot have a comment above it (the `}` and the
   `else` share a line), so its comment is the first line of its body and says why this case is
   reached.
2. Above every `case` label and `default`, or trailing on the label (3.9) when it only names an
   instruction or register.
3. Above every loop whose exit is not in its header, and above every `for (;;)`, saying how it ends.
4. Above every ternary that decides something, and every `?:` that picks a hardware behaviour.
5. Above every `continue`, `break` and early `return` that is not the last line of a `case`, unless
   the comment on the `if` around it already says why.
6. Above a statement that has to stay where it is: an ordering a hardware rule fixes, a store that
   must come before a notify, a call that must come after a flush.
7. Above a cast that changes width or sign when the reason is not obvious from the types on the line.
8. Above or on each numeric literal other than 0 and 1 (3.5).
9. Above each lambda that is stored or passed as a callback, saying what calls it and on which thread.
10. At the head of each group of statements (2.3), unless the group is plain bookkeeping (3.3).

### 3.3 Plain bookkeeping needs no comment

Bookkeeping is a line whose meaning and reason follow from its names alone, and which touches no
hardware fact. These lines do not need a comment, and a run of them under one group comment is the
right form:

- initialising or copying a value by a name that says what it is: `pc = address;`
- clearing, resetting or filling state with calls whose names say so: `vf.fill(0);`
- a loop counter step, a `return result;` of a local, a forwarded argument
- registering a callback that the doc block of the callback already explains

Anything else needs a reason on the page. That includes every bit mask and shift, every arithmetic
with a constant, every store to guest memory or to a register by address, every wait, lock, notify
and atomic operation, every cast with a consequence, and every statement that is only reached under
a condition.

Over-commented. Every line carries a comment, and none of them survives deletion:

```cpp
void Vu::reset() {
    // Clear the float registers.
    for (auto& reg : vf) {
        reg.fill(0);
    }

    // Set VF0 to one.
    vf[0][3] = as_u32(1.0f);

    // Clear the integer registers.
    vi.fill(0);

    // Clear the accumulator.
    acc.fill(0);
}
```

Right. One comment for the group of clears, and one for the line that is a hardware fact:

```cpp
void Vu::reset() {
    // Power-on state: everything zero except the two registers that are constant.
    for (auto& reg : vf) {
        reg.fill(0);
    }

    vi.fill(0);
    acc.fill(0);

    // VF0 reads as (0, 0, 0, 1) whatever a program does to it (documented).
    vf[0][3] = as_u32(1.0f);
}
```

### 3.4 Say why. The deletion test

A comment says what the code cannot: why this branch exists, which fact of the hardware it follows,
why the order matters, what a magic number is, where a fact comes from. It does not say what the
line does, because the line does that.

The deletion test: read the comment, then read the line. If the comment is the line in English,
delete the comment. This applies to a branch as much as to a statement.

| Wrong | Right |
|---|---|
| `// Shift right by 58 and mask with 3.` above `bits(tag, 58, 2)` | `// FLG: how the data after the tag is laid out (documented).` |
| `// If the queue is full.` above `if (queue_.size() >= kMaxQueued)` | `// The EE's side may not run further ahead than this, or the frame it shows is stale.` |
| `// Loop over the pairs.` above a `for` over the pairs | `// Look three pairs back: that is as far as a register can still be in flight.` |
| `// Return false on error.` above `return false;` | (cut it) |
| `// Increment the count.` above `count++;` | (cut it) |

If you cannot think of a reason for a branch, look at the branch again. Either the reason is plain
from a name three lines up, and the line can stay bare with a group comment above it, or the branch
should not be there.

### 3.5 Magic numbers and bit fields

Every numeric literal other than 0 and 1 is one of three things, and says so:

1. A named constant. The name does the explaining, and the constant has a doc comment
   ([4.6](#46-constants-and-enumerators)).
2. A field of a register or packet, read or written with `bits()` or a mask. A comment above the
   group names the register, its fields and where their layout is known from.
3. A unit or a size with a reason. The comment gives it.

A group of lines that decode one register shares one comment above the group.

Wrong:

```cpp
u32 nloop = tag & 0x7FFF;
bool eop = (tag >> 15) & 1;
u32 flg = (tag >> 58) & 3;
u32 nreg = (tag >> 60) & 0xF;
```

Right:

```cpp
// GIF tag, low 64 bits: NLOOP bits 0-14, EOP bit 15, FLG bits 58-59, NREG bits 60-63 (documented).
u32 nloop = static_cast<u32>(bits(tag, 0, 15));
bool eop = bits(tag, 15, 1) != 0;
u32 flg = static_cast<u32>(bits(tag, 58, 2));
u32 nreg = static_cast<u32>(bits(tag, 60, 4));  // 0 means 16 registers
```

A mask or shift that is used more than once in a file becomes a named helper or constant, as `ee.cpp`
does with `rs_of`, `rt_of` and `rd_of`.

### 3.6 Hardware facts and where they come from

A comment that states how the hardware behaves (a timing, a bit layout, an ordering, a quirk) ends
with one of four tags in parentheses. The tag says how the fact is known, so the next person knows
how far to trust it.

| Tag | Means |
|---|---|
| `(documented)` | Written in public hardware documentation. The file comment's `Sources:` line says which. |
| `(seen in game code)` | Shown by how the retail program behaves, read from its code. Name the function by the name the decompilation gives it. Never paste bytes. |
| `(measured)` | Found by a test or an experiment on this model. Say what was run if it is not obvious. |
| `(assumed)` | Nothing confirms it. It stays marked until something does. |

```cpp
// Flags set by an instruction reach the next one four cycles later (documented).
flag_pipe_[flag_count_].at = cycle_ + 4;

// The pair that loads I still reads the old I (seen in game code).
needs.upper_run(*this, up);

// A divide result is ready seven cycles after it starts (documented).
q_at_ = cycle_ + kDivideCycles;

// Eight batches in flight were enough to keep the EE's thread from stalling (measured).
done_.wait(lock, [this] { return queue_.size() < 8; });
```

Group the facts: one tag per comment or per note block is enough when they all come from the same
place. A label on a decode line (`// FLG`) needs no tag if the group comment above it has one.

The file comment lists what the file is based on (4.1). A tag points at that list. Facts taken from
another project say so at the code and in
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md).

### 3.7 Length

| Thing | Limit |
|---|---|
| A `//` comment | One sentence, 20 words at most, and it fits the line |
| A run of `//` lines | Two lines at most. Three or more is a note block |
| A note block | 8 lines |
| The first sentence of a doc comment | 25 words |
| A function or method doc comment | 20 lines |
| A member or constant doc comment | One sentence, or a block of 6 lines |
| A type doc comment | 30 lines |
| A file comment | 40 lines |

A hardware model needs more words than a web server, which is why the limits are higher than a
terse project would set. They are still limits. A type that models timing rules may use its 30
lines as a list, one rule to a line, introduced by a sentence:

```cpp
/**
 * A vector unit running microprograms: VU1 behind VIF1, or VU0 beside the EE.
 *
 * It keeps the timing microprograms depend on (documented):
 * - both instructions of a pair see the same register state;
 * - a float register written by one pair can be read four cycles later;
 * - flags appear four cycles after the instruction that set them;
 * - Q and P arrive when their divider and function unit finish;
 * - XGKICK sends its packet one instruction late.
 */
```

Anything past those limits goes in a file under `docs/` and the comment links to it. Wrap prose in
doc blocks and note blocks to fill each line up to the 100 columns; do not break a sentence early.

### 3.8 Voice

Comments are written like a person explaining the line to a colleague at the same screen. Plain
words, flat sentences, contractions are fine. Three rules and a list.

1. Lead with the thing. "The M bit marks a point the EE waits for." Not "It should be noted that".
2. One idea per sentence. If a sentence has two commas and an "and" in the middle, split it.
3. No reassurance and no consequence essays. A comment states a fact. It does not promise that
   things will be fine.

Never write these in a comment, a doc block, a string or a commit message:

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

Replacements: cut the word, or say the specific thing.

| Written | Fixed |
|---|---|
| `// This ensures the GIF is idle before we read.` | `// Wait for the GIF to go idle before the read.` |
| `// Note that the flags arrive late.` | `// The flags arrive four cycles late.` |
| `// We simply skip unknown codes, so nothing breaks.` | `// Skip unknown codes and count them.` |
| `// Robust handling of a bad tag.` | `// A tag with NLOOP 0 sends nothing; stop here.` |

### 3.9 Trailing comments

A comment after code on the same line is allowed in four places and nowhere else:

1. On an enumerator or a constant in a table: its hardware name or meaning.
2. On a `case` label that names an instruction or a register, in a switch that decodes one:
   `case 0x30:  // MOVE`
3. On a row of a data table.
4. On a statement that decodes one field or names one constant, to name it, in eight words or fewer:
   `u32 ft = bits(up, 16, 5);  // FT, bits 16-20`

A trailing comment on a table line or a decode line is a label, not a sentence: a fragment with no
full stop. A member variable may also carry a trailing comment, which is a full sentence (4.5). A
trailing comment is never used to explain logic. Logic is explained above the line.

### 3.10 Section headings

A source file longer than 300 lines may be divided with headings of this form and no other:

```cpp
// --- Transfers ---
```

A heading is preceded by one blank line and followed by one. It names a topic in one to three words.
Do not draw rulers of dashes or equals signs, and do not nest headings.

### 3.11 What a comment never contains

- Code that is switched off.
- `TODO`, `FIXME`, `XXX`, `HACK`. Something the model does not do yet is reported by the model itself
  (`gstodo` in `gs.h`, `Machine::note`), or recorded in [DESIGN.md](DESIGN.md). Neither a comment nor
  a person's name tells the next reader when it will be done.
- A date, a version, the name or initials of the author, or an issue number. Git knows.
- Anything copied from Sony's SDK source, samples or headers, from leaked or NDA material, or any bytes
  of the retail disc.
- The name of any person who asked to be removed from these projects.

---

## 4. Documentation blocks

### 4.1 The file comment

Every `.h` and `.cpp` file opens with the two licence lines, one blank line, then a file comment.
The file comment says what the file is, what it is based on, and what it leaves out.

```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The memory card library, answered from a directory on the host.
 *
 * The card in the first slot is a directory, a folder on it a directory in that, a file a file.
 * Every function only starts its work on the console; the program asks with sceMcSync whether it
 * is done. Here the work is done at once and the answer is kept for that call.
 *
 * Sources: the interface of the library as publicly documented, and what the games' own card
 * code checks in every result.
 */

#include "sys/machine.h"
```

A header has `#pragma once` after the file comment and a blank line. A source file has its own
header's include after the file comment. A test file says what it tests and where its expected values
come from (section 14).

### 4.2 Types

Every `class`, `struct`, `enum`, `enum class` and `using` alias of a type that is not obvious from
its right-hand side has a doc comment. The first sentence says what the type is, as a noun phrase. A
type that models a hardware unit says which unit, and what it leaves out. A type that is used by
more than one thread says which thread calls what, and what is guarded by what.

```cpp
/**
 * Packets on their way from the EE's thread to the drawing thread.
 *
 * The EE's thread calls `push` and `wait_empty`. The drawing thread calls `pop`. Nothing else
 * touches it. At most `kMaxQueued` packets wait; `push` blocks beyond that, so the EE never runs
 * far ahead of what has been drawn.
 */
class PacketQueue {
```

### 4.3 Functions and methods

Every function and method has a doc comment: free functions, member functions, constructors,
destructors that do work, operators, lambdas that are stored. It is written once, at the first
declaration. The definition in a `.cpp` file does not repeat it. A function that exists only in a
`.cpp` file (in an anonymous namespace) is documented where it is defined.

The block has these parts in this order:

1. A summary: one sentence, starting with a verb in the third person ("Runs", "Returns", "Decodes").
2. Optionally, one or two short paragraphs after a blank ` *` line: what it assumes, what it does
   not do, which hardware rule it follows (with a tag).
3. `@param name Text.` for every parameter, in the parameter order. Mark an output parameter
   `@param[out]`. The text says the unit, range or format when that is not in the type.
4. `@return Text.` for every function that does not return `void`. Say what each value means.
5. `@pre Text.` for what the caller must have done or must know: a state, a thread, a lock held.
6. `@tparam Name Text.` for every template parameter.

Leave out a part that has nothing to say, but never leave out a `@param` or a `@return`.
`@param` text starts with a capital letter and ends with a period.

```cpp
/**
 * Runs from instruction `address` until an instruction with the E bit has finished.
 *
 * The E bit ends the program one pair later, so the pair after it still runs (documented).
 *
 * @param address First pair to run, counted in pairs from the start of program memory.
 * @param limit Most instructions to run. The unit stops there even if the program has not ended.
 * @return How many instructions ran.
 * @pre Program memory holds a program. Call `program_changed` after writing to it.
 */
u64 run(u32 address, u64 limit = u64{1} << 26);
```

A function that returns a status says what each result means, and one that can fail says how the
caller learns:

```cpp
/**
 * Turns a name on the card into a place in the host's card directory.
 *
 * @param card The host directory that stands for the card.
 * @param name A name as the game gave it: parts separated by `/`.
 * @return The host path, or an empty path when the name would lead out of the card directory.
 */
fs::path host_path(const fs::path& card, const std::string& name);
```

The short forms:

- A one-line accessor takes a one-line doc comment: `/** True once the program has stopped. */`
- An overload documents what differs from the main one and says which it forwards to.
- An override needs a doc comment only if it behaves in a way its base's comment does not say.
- A callback member (`std::function`) says when it is called, with what, and on which thread.
- A constructor documents its parameters. A defaulted or deleted special member needs nothing.
- A function that is hot (section 12) says so in its comment, in a line starting `Hot path:`.

### 4.4 Test functions

See [section 14](#14-tests).

### 4.5 Members

Every data member has a comment. There are two forms, and a class uses one at a time for a given run
of members.

Form A. A one-sentence `/** */` on the line above, or a block of up to six lines for a member that
needs its layout spelled out. A blank line separates the member from the one before:

```cpp
/**
 * The cycle from which each field of each float register can be read.
 *
 * A pair that reads a field sooner waits until then (documented).
 */
std::array<std::array<u64, 4>, 32> readable_{};

/** The cycle the divider's result arrives, or 0 when none is on its way. */
u64 q_at_ = 0;
```

Form B. A trailing `//` comment, one sentence that fits on the line, for a run of up to eight
related members. The run has no blank lines inside it, one at each end, and the tool aligns the
comments:

```cpp
u64 cycle_ = 0;         // Time in cycles; an instruction takes one, a wait takes more.
bool running_ = false;  // True while a microprogram is running.
u32 pc_mask_ = 0;       // Program memory size in pairs, minus one.
```

Never mix the two forms inside one run. Members are declared one to a line; no `u32 a = 0, b = 0;`.
A run of at most four members that differ only by a number or an axis (`x0`, `y0`, `x1`, `y1`) may
share one doc comment above them, written in the plural.

A member shared between threads names its guard: "Guarded by `mutex_`." or "Atomic; written by the
drawing thread, read by the EE's." A member that points at memory it does not own says who owns
it.

### 4.6 Constants and enumerators

A `constexpr` constant or a group of them has a doc comment saying what the number stands for, with
a tag if it is a hardware fact.

```cpp
/**
 * Cycles each function unit takes: the instruction this many later sees the result (documented).
 */
constexpr unsigned kDivideCycles = 7;
constexpr unsigned kSqrtCycles = 7;
constexpr unsigned kRsqrtCycles = 13;
```

Each enumerator is on its own line. When its C++ name differs from the hardware's name for it, a
trailing comment gives the hardware's name.

```cpp
/** The primitive types of the GS's PRIM register, field PRIM (documented). */
enum : u32 {
    kPoint,
    kLine,
    kLineStrip,
    kTriangle,
    kTriangleStrip,  // TRISTRIP
    kTriangleFan,    // TRIFAN
    kSprite,
};
```

---

## 5. File layout

### 5.1 One file, one subject

A header declares one main thing, named like the file (`gif.h` declares the GIF's types). Helper
types that appear only in its interface may live in the same header. Two independent types are two
files. A source file implements one header and has its stem (`gif.cpp` for `gif.h`). A file that
implements a slice of another header (`memcard.cpp` for part of `machine.h`) says so in its file
comment. Tools in `src/app` and tests are the exception: one `main` and its helpers per file.

### 5.2 A header, from the top

1. The two licence lines.
2. A blank line, the file comment, a blank line.
3. `#pragma once`, a blank line.
4. Includes, in groups (5.4).
5. A blank line, then `namespace`.
6. Inside the namespace, in this order: aliases, constants, enumerations, forward declarations,
   types in order of dependency, free-function declarations, inline and template definitions.
7. The closing namespace line, with its name.

### 5.3 A source file, from the top

1. The two licence lines.
2. A blank line, the file comment, a blank line.
3. The file's own header, a blank line, the other includes in groups.
4. A blank line, then `namespace`.
5. An anonymous namespace holding everything private to the file: constants, types, helper
   functions, in the order they are used. It closes before the first definition.
6. Definitions, in the order the header declares them: constructors, destructor, public functions,
   then private ones. Free functions come last.
7. The closing namespace line.

Internal linkage is spelled with the anonymous namespace, never with `static` on a function or a
variable. A `static` member of a class is fine, and so is `static` on a local.

### 5.4 Includes

Include only what the file itself uses, directly. Four groups, a blank line between them, each
sorted alphabetically (the tool does both):

1. The file's own header (in a `.cpp` file only).
2. The C++ standard library and C library headers, in angle brackets: `<array>`, `<cstdio>`.
3. Third-party headers: `<SDL3/SDL.h>`.
4. This project's headers, in double quotes, written from the `src` directory: `"ps2/gs.h"`,
   `"sys/machine.h"`. A header next to the file, in `tests`, is `"check.h"` and sorts with them.

A header that is included only under an `#ifndef` is included inside the `#ifndef`, after the
groups. A header never includes a header only so that its users get the include; if a user needs a
header, the user includes it. A header forward-declares a type instead of including its definition
when a pointer or reference is enough.

```cpp
#include "sys/drawing.h"

#include <condition_variable>
#include <deque>
#include <mutex>

#include "ps2/graphics.h"
```

This is the top of `src/sys/drawing.cpp`.

### 5.5 Class layout

Inside a class, in this order:

1. `public:` holds nested types and aliases, constants, then the constructors, the destructor and the
   deleted copy operations, then the member functions grouped by what they are for, then the public
   data members.
2. `protected:`, if any.
3. `private:` holds nested types, private member functions, then the data members.

The order of the functions inside a group is the order a reader needs them: set up, run, ask. Do not
interleave a getter and a setter. Public data members are for plain state that is a documented part
of the hardware's interface, such as registers (`Vu::vf`). Everything else is private.

Use `struct` for a plain aggregate with no invariants and no private parts, and `class` for
everything else.

---

## 6. Declarations

### 6.1 One declaration per statement

Each variable is declared on a line of its own, initialised where it is declared, in the narrowest
scope that holds it. No chained assignment (`q = p = i = 0;`) and no comma-separated declarators.

Wrong:

```cpp
unsigned ft = (up >> 16) & 31, fs = (up >> 11) & 31;
q = p = i = 0;
```

Right:

```cpp
unsigned ft = static_cast<unsigned>(bits(up, 16, 5));  // FT, bits 16-20
unsigned fs = static_cast<unsigned>(bits(up, 11, 5));  // FS, bits 11-15
q = 0;
p = 0;
i = 0;
```

Declare a variable at its first use. The exception is the block of decoded fields at the top of an
instruction handler, which is one group under one comment (3.5).

### 6.2 Integer types

Code that models hardware uses the fixed-width aliases of `ps2/types.h`: `u8`, `u16`, `u32`, `u64`,
`s8`, `s16`, `s32`, `s64`. A value that has a width in the hardware (a register, an address, a
field, a pixel) has that exact width.

- `unsigned` is for loop counters, register numbers and field indices that are not a hardware width.
- `std::size_t` is for byte counts and container sizes.
- `int` is for host-side counts, return codes of the host's functions, and the library's signed
  results.
- Never `long`, `short`, `char` for a number, or `unsigned char` for a byte. `char` is for text.
- `bool` for a flag. Never an `int` that holds 0 or 1.

### 6.3 Literals

Hexadecimal digits are upper case, the `0x` prefix and the suffix are lower case: `0x7FFFFFFFu`.
An unsigned 32-bit literal carries `u`. A constant of 64 bits is `0x...ull`, or `u64{1}` when it is
shifted by a variable. A decimal number of five digits or more uses `'` between thousands:
`294'912'000`. A floating-point literal that is a `float` carries `f`: `1.0f`.

### 6.4 Casts

The build runs with `-Wconversion`, and every narrowing or change of sign says so with a cast.

1. Use `static_cast<T>(x)`. Never a C-style cast `(T)x`, never a functional cast `T(x)` with a
   non-class type.
2. A cast that changes width or sign when the reason is not clear from the types gets a comment
   (3.2, item 7).
3. Reading bits of one type as another goes through `load<T>`, `store<T>`, `as_float`, `as_u32` or
   `std::bit_cast`. Never `reinterpret_cast` on a pointer to read it as another type.
4. `reinterpret_cast` appears only where host data crosses to a C interface, with a comment.
5. `const_cast` is not used.
6. Do not silence a warning by casting to `void`, except for a parameter or variable that is
   deliberately unused: `(void)window_wanted;`, with a comment saying why.

### 6.5 `auto`

`auto` is allowed in four places:

1. A lambda stored in a variable.
2. The result of a call whose type is long and obvious: an iterator, `std::make_unique<T>()`,
   `std::filesystem` results.
3. A declaration whose right-hand side names the type: `auto x = static_cast<u32>(y);`.
4. A structured binding.

Never `auto` for a number. The width of a value in this code is information.

### 6.6 `const` and `constexpr`

- A parameter taken by reference or pointer and not changed is `const`.
- A member function that does not change the object is `const`.
- A constant known at compile time is `constexpr`, in the narrowest scope that uses it.
- A local that is assigned once is `const`. (New code and lines you change; a layout pass leaves
  existing lines alone.)

### 6.7 Macros

Do not write macros. Use `constexpr`, `inline` and templates. The two that exist are `CHECK` and
`CHECK_EQ` in the tests, which need `__FILE__` and `__LINE__`, and the `OPENRAC_NO_WINDOW` switch the
build defines. A new macro needs a reason in its doc comment and an `OPENRAC_` prefix.

### 6.8 `using`

`using namespace` never appears in a header. In a source file it is allowed for the project's own
namespace only (`using namespace ps2;`), after the includes and before the first declaration.
Prefer a `using` declaration or an alias for a single name: `using ps2::u32;`,
`namespace fs = std::filesystem;`.

---

## 7. Naming

The rule behind every row: keep the names the hardware documentation uses.

| Thing | Style | Examples |
|---|---|---|
| Namespace | short, lower case | `ps2`, `sys`, `host`, `fp`, `vuasm` |
| Type, template type parameter | `PascalCase`; an acronym is one word | `Vu`, `Gs`, `Vif1`, `GuestMemory`, `DmaChannel` |
| Function, method | `snake_case`, a verb or a question | `run_frame`, `host_path`, `is_zero` |
| Local, parameter | `snake_case` | `transfer_tag`, `clip0` |
| Public data member | `snake_case` | `vf`, `mac`, `program_name` |
| Private data member | `snake_case_` with a trailing underscore | `running_`, `q_at_` |
| Constant | `kPascalCase` | `kClusters`, `kEeHz` |
| Hardware register, command or format | spelled as the documentation spells it, upper case, inside a namespace or enum named for the family | `gsreg::PRIM`, `gspriv::CSR`, `PSMCT32` |
| A value of a field, and every other constant | `kPascalCase` | `kTriangle`, `kBandShift` |
| Enum class and its values | `PascalCase`, both | `enum class Op { Add, Sub }` |
| Template non-type parameter | `snake_case` | `to_acc` |
| Macro | `OPENRAC_UPPER_SNAKE` | `OPENRAC_NO_WINDOW` |
| Thread-local variable | `tls_` prefix | `tls_pixels` |
| File | `snake_case.h` and `.cpp` | `gs_memory.cpp`, `vu_asm.h` |

### 7.1 Short names

A name is a word, or words joined by underscores. Short names are allowed in exactly three kinds of
place:

1. Names the hardware documentation gives: `ft`, `fs`, `fd`, `dest`, `fbp`, `psm`, `tbp`, `clut`,
   `nloop`, `vi`, `vf`.
2. A loop counter: `n`, or `row` and `column` for pixels.
3. A coordinate or a colour or texture component: `x`, `y`, `z`, `u`, `v`, `s`, `t`, `q`, `r`, `g`,
   `b`, `a`.

Anything else is spelled out. `msg`, `idx`, `tmp`, `cnt`, `buf`, `ptr` are not names.

### 7.2 Functions

- A function that answers a question reads as one: `stopped()`, `is_zero(x)`, `has(dest, field)`,
  `intersects(other)`. A function that changes state reads as an action: `reset`, `start`, `flush`.
- There is no `get_` prefix. A reader is the noun (`control(reg)`, `f(reg, field)`), and its writer is
  `set_` plus the noun (`set_control(reg, value)`).
- A callback member starts with `on_`: `on_kick`, `on_step`, `on_vblank`.
- A function that decodes a field from a word is named for the field with `_of` when it has a
  family: `rs_of`, `rt_of`.
- A unit-suffix goes on a value that is not in the obvious unit: `size_bytes`, `delay_cycles`.

### 7.3 Boolean names

A boolean reads as a statement that is true or false: `running_`, `looked_at`, `sticky_readers_`,
`program_dirty_`. Not `flag`, not `status`, not `mode` (those are values). Do not negate in the name
(`not_ready`, `no_wait`): name the positive case, and write `!` at the use.

### 7.4 Names for types

A name says what a thing is, not how it is built. A suffix such as `Info`, `Data`, `Manager`,
`Helper`, `Util` says nothing; pick the noun. The types of the hardware model are named for the
hardware unit (`Gs`, `Vif1`, `Ee`); the types around it are named for what they do (`Drawing`,
`Disc`, `Window`).

---

## 8. Functions

### 8.1 Shape of a body

A function body has these parts, in this order:

1. Guard clauses: the checks that end the function early (section 9).
2. The decoded or prepared values the main work needs.
3. The main work, in groups, each with its opening comment.
4. The result.

### 8.2 Length and size

- At most 60 lines from the opening `{` to the closing `}`, counting comments and blank lines.
- At most five parameters. More than five go into a struct.
- At most three nested levels of `if`, `for`, `while` or `switch` inside the function.

Past a limit, split the function on the seams its group comments already mark. The comments become
function names, and a function name cannot go stale the way a comment can. The exceptions are a
`switch` that decodes or dispatches, where every case is a label and one statement, and the cases
in section 12.

### 8.3 Parameters

- Order: the thing being acted on, then the inputs, then the output parameters, then the optional
  ones with defaults. A context object (`Machine& m`) is first.
- Small values (up to 16 bytes) by value. Everything else by `const&`, unless the function keeps it,
  in which case by value and `std::move`.
- A pointer parameter is for "may be absent" or for a raw guest buffer. If it cannot be null, it is
  a reference.
- An output goes in the return value (a struct, `std::optional`, `std::pair`) when it can. When a
  function must give two things, an output parameter is a non-const reference, last, and marked
  `@param[out]`. An error message is the one pointer output the code already uses:
  `std::string* error = nullptr`.
- A flag parameter that switches two behaviours is two functions, or an enum with two named values.
  `draw(true)` says nothing at the call.

### 8.4 Return values

- A function that can fail returns `bool`, a status enum, or `std::optional`, and is
  `[[nodiscard]]`. Its doc comment lists what each result means.
- Return early for the failure, then continue at the left margin (9.1).
- Return a value from one place at the end where the function is a plain computation. Return from
  the middle only through a guard clause or a `case`.
- Do not return a reference or pointer into a local, or into a container that the caller may
  change.

### 8.5 Lambdas

- A lambda captured and stored (a callback, a thread body, a deferred task) lists its captures:
  `[this]`, `[this, odd = command.value != 0]`. Never `[&]` or `[=]` there; a stored lambda may
  outlive the frame.
- `[&]` is allowed for a lambda that is used and gone before the function returns: a sort predicate,
  a wait predicate.
- A lambda with more than three lines is a named function or method instead, unless it is the body
  of a thread or a callback registration, where it is written out and commented from above.
- A stored lambda has a comment above it saying what calls it and on which thread.

### 8.6 Templates

Use a template when the same code is needed for several types, or when a value must be known at
compile time so the compiler can specialise a hot path (`arith<op, from, to_acc>`). Do not write one
to avoid typing. Each template has a doc comment with `@tparam` for every parameter and says what
the parameter must satisfy. Constrain a type parameter with `requires` or `static_assert`.

### 8.7 Classes

- Follow the rule of zero. If a class owns a thread, a file or a lock, delete its copy operations
  (`Drawing(const Drawing&) = delete;`) and say why in the type's comment.
- A constructor with one argument is `explicit`.
- A function that overrides is marked `override` and nothing else.
- A member function that does not change the object is `const`.
- A default member initialiser sets a value that is the same in every constructor.
- A destructor that does work (joins a thread, closes a file) has a doc comment saying what it
  waits for.

---

## 9. Control flow and guard clauses

### 9.1 Guard clauses

A function checks what must hold first, and leaves at once when it does not. The rest of the body
then stays at the left margin. A guard has a comment (3.2), braces, and a blank line after it (2.4).

Wrong:

```cpp
int open_file(Machine& m, u32 port, u32 slot, const std::string& name, u32 mode) {
    int result = kNoCard;
    if (present(m, port, slot)) {
        fs::path path = host_path(m.card.directory, name);
        if (!path.empty()) {
            if (m.card.open.size() < 3) {
                result = open_host_file(m, path, mode);
            } else {
                result = kTooManyOpen;
            }
        } else {
            result = kNoSuchEntry;
        }
    }
    return result;
}
```

Right:

```cpp
int open_file(Machine& m, u32 port, u32 slot, const std::string& name, u32 mode) {
    // No card in this slot.
    if (!present(m, port, slot)) {
        return kNoCard;
    }

    fs::path path = host_path(m.card.directory, name);

    // The name would have led out of the card's folder.
    if (path.empty()) {
        return kNoSuchEntry;
    }

    // The console's library keeps three files open at most.
    if (m.card.open.size() >= 3) {
        return kTooManyOpen;
    }

    return open_host_file(m, path, mode);
}
```

### 9.2 `else`

No `else` after a branch that ends in `return`, `continue`, `break` or `throw`. A chain of
`else if` is for choosing one of several equal cases (decoding a field). When the cases are values
of one variable, use `switch` instead.

### 9.3 `switch`

1. A `switch` on an `enum class` names every enumerator and has no `default`, so the compiler warns
   when one is added.
2. A `switch` on an integer from the hardware has a `default`, and the `default` has a comment saying
   what an unlisted value means: undefined, ignored, or counted as unknown.
3. Every case ends in `break`, `return` or `continue`, or in `[[fallthrough]];` with a comment on
   why it falls through. A label that shares a body with the next one is not a fallthrough and needs
   no marker.
4. A case that declares a variable is wrapped in braces.
5. A decoding switch names each case, above the label or as a trailing label comment (3.9). It is not
   exempt from the comment rule for what the case does when that is not its name.

```cpp
switch (command(code)) {
    case kStcycl: {
        // STCYCL: CL is bits 0-7 of the immediate, WL bits 8-15 (documented).
        cl = static_cast<u32>(bits(code, 0, 8));
        wl = static_cast<u32>(bits(code, 8, 8));
        break;
    }

    case kBase:
        base = static_cast<u32>(bits(code, 0, 10));
        break;

    default:
        // A code this model does not know: count it and carry on.
        unknown_codes++;
        break;
}
```

### 9.4 Loops

- Use a range-based `for` when the index is not needed.
- A counting loop is `for (unsigned n = 0; n < count; n++)`, with the counter named `n` (or a
  coordinate name). The counter's type matches what it is compared to.
- An endless loop is `for (;;)`, never `while (true)`, and its comment says how it ends.
- `break` and `continue` are branches and carry a comment.
- A loop that waits on a condition variable always gives the wait a predicate (section 11).

### 9.5 Ternaries

A ternary chooses between two values and fits on one line, or wraps as in 1.5. It is never nested.
Three or more outcomes are an `if` chain or a `switch`.

Wrong:

```cpp
int whence = from == 1 ? SEEK_CUR : from == 2 ? SEEK_END : SEEK_SET;
```

Right:

```cpp
// The library's "from" argument: 0 start, 1 current position, 2 end (seen in game code).
int whence = SEEK_SET;

if (from == 1) {
    whence = SEEK_CUR;
}

if (from == 2) {
    whence = SEEK_END;
}
```

### 9.6 No `goto`, no exceptions in the model

`goto` is not used. The hardware model (`src/ps2`, `src/sys`) does not throw and does not catch:
guest data is not trusted and never raises an exception (section 10). `src/app` and `src/host` may
use the standard library's throwing functions only inside a try block that turns the error into
a message and an exit code.

---

## 10. Errors, logging and messages

### 10.1 Guest data is not trusted

The program being run is the input. A value it supplies (an address, a count, a code, a tag) can be
anything, including nonsense a real console would have crashed on. Model code never reads or writes
out of range, never divides by a value it has not checked, and never ends in undefined behaviour
because of what the guest did.

For a value outside what the model supports, the code does one of these, and says which in a comment:

1. Does what the hardware does, if the hardware's behaviour is known (a masked address, a wrap).
2. Reports it once with `note` (`Machine::note`, or `Gs::note` with a `gstodo` bit) and carries on.
3. Counts it (`unknown_ops`, `unknown_codes`) so that a tool can print the total.

It never guesses silently. This is the rule in the runtime README, and it is the one the
rest of this section serves.

### 10.2 Reporting an error to the caller

- A function that can fail says so in its type: `bool`, a status enum, `std::optional`, or a result
  struct. An error text is returned through `std::string* error`.
- `src/ps2` and `src/sys` do not call `exit`, `abort` or `std::terminate`, and do not print to
  `stdout`. Apart from `Gs::note` and `Machine::log`, they do not print at all. A condition that
  cannot be handled is reported through a return value or `Machine::halted`.
- `assert` is for a condition the programmer of the calling code broke, never for guest data.
- A host call that can fail has its result checked: every `fopen`, `fread`, `fwrite`, `fclose`,
  `std::filesystem` call (use the overload with `std::error_code`), SDL call.

### 10.3 Logging

All diagnostics from `src/sys` go through `Machine::log(level, format, ...)`, whose format string is
checked by the compiler. The levels:

| Level | Use |
|---|---|
| 0 | A failure the user needs to see: a disc read that could not be done. Always printed. |
| 1 | A notable event: a file opened on the card, something not modelled (`note`). |
| 2 | A trace of every call. For working on the model with `--verbose 2`. |

`src/ps2` has no machine to log through. It counts what it did not understand, and its one place
that prints is `Gs::note`, the once-only "not modelled yet" report on `stderr`.
`src/host` and `src/app` write to `stderr` directly. Output a tool exists to produce (a listing, a
picture, a count) goes to `stdout` or a file.

### 10.4 Writing a message

- Say what happened, to what, with the values: `cannot read %u sectors at %u to %08x`.
- Start with a lower-case word, no full stop at the end. A list of facts is separated by commas.
- Addresses and register values are hexadecimal with the width of the thing: `%08x` for a 32-bit
  address. Counts are decimal.
- Use the console's own names (`VIF1`, `sceMcSync`, `STCYCL`) and the project's, not a paraphrase.
- A `%llu` is given `static_cast<unsigned long long>(x)`.
- No em dashes, no exclamation marks, no apology, no "please", no "failed to" when "cannot" fits.
- A message a user can act on says what to do: `no memory card: cannot make DIR (REASON)`.

Wrong:

```cpp
m.log(0, "Oops! Something went wrong while trying to read the disc!");
```

Right:

```cpp
m.log(0, "disc read: cannot read %u sectors at %u to %08x", count, sector, buffer);
```

---

## 11. Threads and resources

### 11.1 Threads

The drawing path, the GS's rasteriser and the sound mixer run on threads of their own. The rules:

1. A class used by more than one thread says, in its type comment, which thread calls which method.
2. Every member shared between threads says what guards it (4.5).
3. A lock is taken with `std::lock_guard` or `std::unique_lock`, in the narrowest block that holds it.
   Never a bare `lock()` and `unlock()`, except `unique_lock` unlocking around a call that must run
   without it, with a comment.
4. A wait on a condition variable always has a predicate, and the predicate is a lambda that says what
   is being waited for.
5. A notify is called after the lock is released, unless the order matters, and then a comment says
   it does.
6. An atomic that does not need sequential consistency names its memory order and says why.
7. A thread is joined in the destructor of the object that started it. The class deletes its copy
   operations.
8. A thread-local variable is prefixed `tls_`, and its comment says when its contents are handed
   on.

```cpp
void Drawing::give(Command&& command) {
    // Without a drawing thread, the caller's thread does the work now.
    if (!thread_.joinable()) {
        run(command);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(command));
    }

    // Wake the drawing thread after the lock is gone, so it does not wake only to block on it.
    work_.notify_one();
}
```

### 11.2 Files and other resources

1. A resource that must be released is owned by an object that releases it in its destructor
   (`std::fstream`, `std::unique_ptr<std::FILE, int (*)(std::FILE*)>`, a class with a destructor).
   New code does not pair a bare `fopen` with a manual `fclose` across branches.
2. A path is a `std::filesystem::path`, never a `std::string` that is cut with `find`.
3. A name that comes from the guest is turned into a host path through one function that rejects
   anything leading out of the intended folder (`host_path` in `memcard.cpp`). Nothing else builds a
   host path from guest text.
4. A file written is written whole or not at all where a partial file would be mistaken for a good
   one: write to a temporary name, then rename.
5. A file in the user's memory card folder is deleted or truncated only when the guest program asked
   for exactly that.

---

## 12. Performance-sensitive code

The inner loops of the project (the VU1 interpreter in `Vu::step`, the arithmetic in `Vu::arith`, the
EE's `run` loop, the GS's `pixel`, `shade` and `draw_*`) run billions of times per minute of play.
They may break some rules to be fast. They break only those listed here, and every break is marked.

### 12.1 What may deviate

| Rule | May be broken for | Mark |
|---|---|---|
| 8.2 length and nesting | A function that is one dispatch or one fused loop | `Hot path:` line in the doc comment naming the limit broken |
| 9.1 guard clauses | A loop that tests once at the top, not at every call | the comment on the test |
| 8.4 one `return` at the end | A tight function with two exits | the comment on the exit |
| 6.2 `unsigned` for counters | A counter that must be 32 bits wide | its name and the comment |
| 9.5 ternaries | A branch-free select the compiler would not find | a comment giving the plain form |

### 12.2 What never deviates

Braces. The comment rules for branches, numbers and bit fields. Naming. The 100-column limit. The
layout tool. The doc comment: a hot function is documented like any other, and says it is hot.

### 12.3 How to mark it

A hot function has a line in its doc comment, starting `Hot path:`, that names which rule it breaks
and how the cost was found.

```cpp
/**
 * Draws one row of a triangle between its two edges.
 *
 * Hot path: one loop of 80 lines and no guard clauses, because the span test, the blend and the
 * store share registers; splitting them cost 11 percent in `openrac-boot --report` (measured).
 */
void draw_row(const Env& e, s32 y, s32 x0, s32 x1);
```

A local trick is explained where it is, with the plain form beside it, so a reader can check them
against each other:

```cpp
// Same as `count != 0 ? count : 256`, without a branch: a zero count means 256 (documented).
u32 n = ((count - 1) & 0xFF) + 1;
```

An annotation such as `[[likely]]`, `[[unlikely]]`, `[[gnu::always_inline]]` or `__builtin_expect`
needs a comment giving the measurement or the reason the path is rare. A claim of "faster" without a
number is not a reason.

---

## 13. Reuse

### 13.1 Search before you write

Before you add a helper, a constant, a decoder or a formula, search for it. If it exists, call it.
A second copy of anything the code already does is the mistake nobody notices until the two drift
apart. Search by what the code does, not by the name you were about to give it.

Look here first:

- `ps2/types.h`: the integer aliases, `bits`, `load`, `store`, `as_float`, `as_u32`.
- `ps2/fp.h` and `fp_quad.h`: the console's float arithmetic. Do not write another.
- `ps2/vu_asm.h`: encoders for vector unit instructions. Tests build their programs with it.
- `ps2/vu_dis.h`: the disassembler, for any listing.
- `ps2/gs_memory.h`: the block and column layout of every pixel format.
- `tests/check.h`: the test macros and the table runner.
- The class you are in, its parent, and its neighbours.

### 13.2 The rule of two

The second time you write a piece of logic, move it to one place. Not the third time. Two copies is
where a fix lands on one and not the other. Three duplicated lines that encode a hardware rule matter
more than twenty lines of plumbing.

If the shared piece is used by two files, it goes in the header both already include, or in a new
small header. If it is used by two games, it does not belong in `src/ps2` unless the hardware is the
same; nothing specific to one game goes in `src/ps2` or `src/host`.

### 13.3 A block pasted with two words changed wants a parameter

Identical control flow with one or two values different is a function with parameters. The fix for
`sceMcRead` and `sceMcWrite` finding their file twice is a function that finds it once.

### 13.4 Put the logic where the data is

Behaviour belongs on the type that owns the state it reads. A function that reaches through three
members of another object to compute something is a method of the object at the end of that chain.

### 13.5 Use what is already resolved

A value computed earlier in the function is used, not worked out again from its source. Pass the
object, not its pieces, when the callee would put them together again. A fixed number that appears in
two places is a constant, declared once, with its name.

---

## 14. Tests

### 14.1 The framework

`tests/check.h` is the whole framework: `CHECK(cond)`, `CHECK_EQ(actual, expected)`, a table of
`TestCase{"name", function}` and `run_tests`. Do not add another. If it lacks something, extend it in
its own commit.

### 14.2 File layout

A test file is laid out like a source file (5.3), with these parts:

1. The licence lines, then a file comment: what unit the file tests, and where its expected values
   come from.
2. Includes, then `using namespace` for the namespaces under test.
3. An anonymous namespace holding the helper types and functions, each documented.
4. The test functions, in the order the unit's header declares what they test.
5. `main`, with the table of tests in the same order as the functions above it.

### 14.3 One test, one behaviour

A test function checks one behaviour of the unit, and its name says which. It is at most 60 lines.
When it grows past that, it is two tests.

- Function name: `test_` and the behaviour in snake case: `test_branch_sees_the_older_integer`.
- Table name: the same behaviour as a lower-case phrase, no `test` in it:
  `{"a branch sees the older integer", test_branch_sees_the_older_integer}`.
- The doc comment states the behaviour in one sentence, and the source of the expectation with a tag
  (3.6).

### 14.4 Expected values

Expected values come from the hardware documentation and from arithmetic, never from a retail
program's output and never from a number copied from a run of the model itself.

1. Write the value as the derivation when it has one: `0.75f + 2.25f`, or a constant with its
   meaning beside it: `0x33C00000u  // 3 * 2^-25`.
2. A magic expected number gets a comment naming what it is (3.5), the same as any other.
3. `CHECK_EQ` takes the actual value first and the expected second, and both are the same type; write
   the type on the literal: `CHECK_EQ(ran, u64{11})`.
4. A group of checks that belong to one step has one comment above it saying what the step proves.
5. Test programs and packets are built with the encoders in the headers (`vu_asm.h`, a `vif()`
   helper), never from bytes copied from a game.

```cpp
/**
 * A pair reads the register file as it was before either half ran (documented).
 */
void test_pair_reads_old_values() {
    Unit u;
    u.set(1, 1.0f, 1.0f, 1.0f, 1.0f);
    u.set(2, 2.0f, 2.0f, 2.0f, 2.0f);

    Program p;

    // The lower half stores vf1 while the upper half changes it: memory gets the old value.
    p.add(add(XYZW, 1, 1, 2), sq(XYZW, 1, 8, 0));
    finish(p);

    u.run(p);

    // The upper half's result is in the register, the lower half's store holds the old one.
    CHECK(u.vu.f(1, 0) == 3.0f);
    CHECK_EQ(u.word(8, 0), as_u32(1.0f));
}
```

### 14.5 What a test never contains

Game code, microcode, data or bytes. A path to a disc image. A dependence on the clock, on the
number of cores, on the order of threads, or on the user's files. A test that needs a disc is not a
test of this directory; it is a tool (`openrac-vuscan`).

---

## 15. The project's constraints

These come from [AGENTS.md](../../AGENTS.md), [SOURCING.md](../../docs/policy/SOURCING.md) and the
[runtime README](../README.md). They are here so that a review of a diff can check them in one place.

1. **Licence lines.** The first line of every source file is
   `// SPDX-License-Identifier: GPL-3.0-or-later`, the second is
   `// Copyright (c) 2026 the OpenRAC contributors`. A new file gets both. Code adapted from a
   permissively licensed project says so in the file comment and in
   [THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md).
2. **No game content.** No game code, microcode, data or bytes in a source or in a test. An expected
   value comes from documentation and arithmetic (14.4). A tool that reads a game file prints counts
   and never the content.
3. **No Sony SDK.** Nothing written from Sony's SDK source, samples or headers, from leaked or NDA
   material. If a function looks like SDK sample code, decode it from the program's behaviour.
   Reviewers look twice at code that arrives suspiciously complete or with SDK-style names.
4. **Public hardware documentation and the games' own behaviour** are the sources, and a comment says
   which (3.6).
5. **Nothing specific to one game** in `src/ps2` or `src/host`. What differs per game is a table
   generated from that game's files, kept outside these directories.
6. **Names.** The third-party NTSC decompilation is called "the NTSC decomp" in code, comments and
   commit messages. No author's name, handle or address. Nobody who asked to be removed from these
   projects is named anywhere, and nothing that was removed at someone's request comes back in.
7. **Credit what you reuse**: the project, the file or function, and its licence.
8. **Do not push or open pull requests** unless the person you work for asks for that action.

---

## 16. Reformatting existing code

This is for the pass that applies this page to code written before it. The goal is a diff that a
reviewer can approve without reading the logic.

### 16.1 What you may change

1. Layout, by running the tool on whole files.
2. Comments: add the doc comments and the missing branch and number comments. Fix a comment that
   breaks a rule in section 3 (a dash, a banned word, a restatement).
3. Blank lines (section 2).
4. The grouping and order of the includes, and their spelling from the `src` directory.

### 16.2 What you may not change

1. Behaviour. Not even a bug you see: write it down, and fix it in its own commit.
2. A comment that is correct and follows the rules. It stays as it is, byte for byte, even if you
   would word it differently. (Hard rule 6.) Moving a comment with the line it describes is not
   rewording it.
3. Names. A rename is a `refactor(runtime)` commit of its own, done for names that break section 7,
   all uses in the same commit. A layout pass does not rename.
4. The order of declarations, the types of variables, `const`, casts or `auto`, and the size of a
   function. Those rules apply to new and changed code (6.4 to 6.6, 8.2), not to a layout pass. Adding a missing cast to quiet a warning
   is a change of code and has its own commit.
5. Anything in a file somebody else has uncommitted changes in. Look at `git status` first and leave
   those files alone until their owner commits.

### 16.3 A comment you cannot write

A comment that states a hardware fact must be true. If you do not know why a branch exists or what a
number means, do not invent a reason. Write what you can verify from the code, tag it `(assumed)`
if it is a guess about the hardware, and list the branch in your report so a person who knows can
check it. A wrong comment is worse than none.

### 16.4 How to work

1. Read the file and the matching header first. Understand what each function does before you
   comment it.
2. Do one file at a time, or one directory. Format, then add comments, in two commits:
   `style(runtime): ...` for the tool's output, then `docs(runtime): ...` for the comments.
3. Run the tool on every file you touch. Never format a file you are not also committing.
4. Keep the diff of the first commit to what the tool does. If a line is odd after the tool, fix
   the cause (a wrapped call that should be split, 1.4), not the tool's output.

### 16.5 How to check that behaviour did not change

Layout and comments do not change the compiled code, and you can prove it. Build with debug
information off before you start, build again after, and compare the object files of `src/`:

```sh
# Before touching anything.
cmake -S runtime -B build/before -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-g0
cmake --build build/before

# After the pass, in a second build directory, with the same source paths.
cmake -S runtime -B build/after -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-g0
cmake --build build/after

# Every object of src/ must be identical. The test objects are left out: CHECK writes __LINE__.
for f in $(cd build/before && find CMakeFiles -name '*.o' | grep -v tests | sort); do
    cmp -s "build/before/$f" "build/after/$f" || echo "DIFFERENT: $f"
done
```

No `DIFFERENT` line means the compiler saw the same program. (When this page was written, the tool was run over all of `src/` and
the 18 objects built this way were identical.) If a file differs, find the
line; it is a change of code, not layout.

Then run what the runtime README names:

```sh
cmake --build build/after
ctest --test-dir build/after
```

The build must have no new warnings (the flags are `-Wall -Wextra -Wpedantic -Wshadow -Wconversion`).
If you have a disc, `openrac-boot` with `--frames 600 --ppm` before and after should write the same
picture. Say in your report what you ran, and what you could not.

---

## 17. Self-check before you report done

Run this against your own diff. Every item is countable. Do not report the work as done until every
line reads yes.

**Hard rules**

1. Is the tool clean on every file you touched? (`clang-format --dry-run -Werror`; no output.)
2. Zero tab characters in your diff? (`git diff -U0 | grep -n "^+.*$(printf '\t')"`)
3. Zero lines over 100 columns in your diff?
4. Every `if`, `else`, `for`, `while`, `do` body in braces? (The tool inserts them; check nothing
   escaped in a macro.)
5. Every new declaration has a `/** */` doc comment? Count the bare ones.
6. Every branch, guard, `case` and deciding ternary has a comment? Count the bare ones.
7. Every numeric literal other than 0 and 1 has a name or a comment? Count the bare ones.
8. Every group of statements opens with a comment, or is plain bookkeeping? Count the bare ones.
9. Is every `//` comment one sentence of 20 words or fewer? Count the longest.
10. Zero stacks of three `//` lines? Every note block 8 lines or fewer?
11. Zero em or en dashes? (`git diff -U0 | perl -CSD -ne 'print if /\x{2014}|\x{2013}/'`)
12. Zero banned words from 3.8? (`git diff | grep -niE 'furthermore|moreover|additionally|robust|comprehensive|seamless|powerful|simply|basically|ensure|utilize|leverage'`)
13. Zero comments that restate the line? Zero `TODO`, `FIXME`, `XXX`, `HACK`, dates or author names?
14. Every comment that states a hardware fact ends in a source tag?
15. Every comment you did not break is byte-for-byte unchanged?
16. Do the first two lines of every file you created read as the SPDX and copyright lines?
17. Zero game bytes, SDK-derived text or removed names?

**Doc blocks**

18. Does every function doc have a `@param` for each parameter and a `@return` when it returns?
19. Does every type that more than one thread uses name the thread that calls each method?
20. Does every shared member name its guard?
21. Is each doc comment directly on its declaration, with no blank line?

**Layout and structure**

22. Blank lines: one between functions and groups, none doubled, none inside a `{` or before a `}`,
    one after each closing brace of a block?
23. Includes in the four groups, the project's from the `src` directory?
24. Anonymous namespace, not `static`, for private functions?
25. Classes in the order of 5.5: types, constructors, functions, public data, private functions,
    private data?

**Code**

26. Functions of 60 lines or fewer, five parameters or fewer, three nested levels or fewer? Any
    exception carries a `Hot path:` line.
27. One declarator per declaration, no chained assignment, no nested ternary, no `goto`?
28. Fixed-width aliases for hardware values, `static_cast` for every narrowing, no C-style cast?
29. `auto` only in the four allowed places, never for a number?
30. Guard clauses first, no `else` after a `return`, every `switch` handled as in 9.3?
31. Messages in the form of 10.4, through `Machine::log` in `src/sys`?
32. Locks scoped, waits with predicates, stored lambdas with explicit captures?
33. Did you search for an existing helper before writing a new one? Does any logic in the diff appear
    twice?

**Tests and checks**

34. Each new test: one behaviour, `test_` name, table name as a phrase, doc sentence with a tag,
    expected values derived and not copied?
35. Build with no new warnings, `ctest` passes?
36. For a formatting or comment pass: object files of `src/` identical before and after?

A documentation pass that adds more words than it adds understanding has failed, however correct
each comment is.
