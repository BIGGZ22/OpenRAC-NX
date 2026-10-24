# What drawing costs: the counters and the optimisation registry

The port draws every level at native resolution with every upscaled feature, in full detail. The
render packages of the restructure (phase R, [PROGRESS.md](../restructure/PROGRESS.md)) lower what
drawing that same picture costs; none of them changes a pixel unless its entry below states a
tolerance the maintainers accepted, and none brings back the PS2's level of detail, draw distance or
fog distance, adds a placeholder or a skipped draw, or delays a visible update.

Two rules decide whether such a change pays:

- **Counts and bytes decide, milliseconds inform.** The counters below are exact: the same work
  gives the same numbers on any machine. A package's pass or fail is the counter it names; the
  milliseconds are reported beside it, as a median of at least five runs with the spread.
- **Every optimisation can be switched off.** Each one is an entry of the registry, so its
  reference path stays and can be compared with it frame by frame.

## The optimisation registry

[`port/renderer/render_opt.h`](../../port/renderer/render_opt.h) holds one table of every render
optimisation: its name, what it makes cheaper, whether it is on by default, and its tolerance.

- The draw code asks `render_opt::enabled(Opt::...)` and takes the reference path (the code as it
  was before the optimisation) when it is off. The question is one array read after the first call,
  so a draw may ask it each time.
- `OPENRAC_RENDER_OPT_OFF=name,name` switches the named entries off for a run, and
  `OPENRAC_RENDER_OPT_OFF=all` every one. Spaces around a name and empty items are ignored; a name
  that is no entry's is logged as a warning and switches nothing. The variable is read once, at the
  first question ([DEBUGGING.md](DEBUGGING.md)).
- `render_opt::set` and `render_opt::reset` switch entries between frames for the tests and the
  benchmark.
- The tolerance is zero (byte-identical frames) unless the entry says how many of a frame's pixels
  may differ and by how much in a channel. A tolerance is the maintainers' to accept, in the
  package's report.

The twelve older `*_OFF` switches of [RAC1_GPU_DRIVEN.md](RAC1_GPU_DRIVEN.md) stay what they are;
no optimisation adds an environment variable of its own.

A new entry is an enumerator of `render_opt::Opt` and a row of the table in `render_opt.cpp` at the
same place, and a section on this page: what it changes, the counter it lowers with its numbers
before and after, and how its frames are shown to be the same.

| Name | Default | Tolerance | What it does |
|---|---|---|---|
| `registry-selftest` | on | none (exact) | Draws nothing. The entry the registry's own tests and the equivalence harness's switch. |

Its tests are `render_opt` (the table, and the parsing of the variable: a name, a list, `all`, an
unknown name kept apart; it runs in the headless preset too) and `render_opt_environment` (the
variable as ctest sets it, read and applied, with its warning).
