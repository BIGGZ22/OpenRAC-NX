# Method: how a decompilation wave runs

This is the protocol the sibling projects use, described once so that any
agent can be dropped into any lot and know what to do, what to produce and
what the root will do with the result.

## 0. Vocabulary

| Term | Meaning |
| --- | --- |
| placement | one address in one module where a function body lives |
| extent | the byte range a body really occupies (a real return ends it) |
| candidate | a bounded entry with a measured extent, not yet a proven function |
| family | bodies whose raw bytes are identical, carried by several placements |
| profile | a pinned compiler + flags + assembler combination, with hashes |
| gate | a private script that rebuilds a complete program and compares hashes |
| lot | one work unit, one directory, one agent |

## 1. The foundation lot (E0)

One agent prepares the ground and **does not** translate anything:

- extract the target locally (image, modules, symbols) into `work/E0/`;
- pin the identity of the input: size, SHA-1, SHA-256, and a per-module
  manifest;
- compute the **bounds**: for every candidate entry, the extent that a real
  return ends, the evidence that justifies it (a direct call, a pointer, an
  export), and the bytes of zero padding that follow;
- build the **shortlist**: `module, offset, size, bound_source, evidence,
  body_sha256, shared_modules, shared_placements`.

The shortlist is ranked by **leverage first**: how many placements carry the
same raw body. One authored source can satisfy dozens of placements, so a
shared family is worth more than a rare large body.

The foundation is written once and never rebuilt by an agent.

## 2. Cutting the lots

The shortlist is cut into lots, one per agent, so that:

- each lot owns one region of the program space (a module group), and
- no two lots own the same placement.

Bands matter: bodies under ~400 bytes and bodies above it fail for different
reasons and need different approaches. Keep the hard band in its own lots.

## 3. The translation lot

An agent works one lot and writes, in `work/<LOT>/` only:

- `bodies/<MODULE>_<OFFSET>.c` — one authored source per placement;
- `resultats.tsv` — `module, offset, size, profile, as, statut`;
- `abandonnes/` — the parked attempts, with their first difference and a
  reopening condition;
- `RAPPORT.md` — what was measured, in the language of the project.

The loop is always the same: read the retail disassembly of the target,
write the C, **prove** it, read the first difference, fix the source, repeat.

### The proof

The harness compiles the candidate source with the pinned profile, links it at
the measured offset, and compares **every byte** with the retail bytes. The
only accepted result is a byte-identical body. A refusal (unresolved symbol,
unknown extent) is not an attempt and is not counted. A near miss is not a
match.

## 4. Families: one source, many placements

When a body is proved at one placement, look for the other placements that
carry the same raw body. One source can then serve dozens of placements, and
each placement is proved **on its own** — never inferred from a sibling.

This is where the bulk of the measurable progress comes from, and it is also
where the temptation to cheat lives: a placement counts only if its own
linked bytes match its own retail bytes.

The bounds carry the raw hash of every bounded body, so the family of a
proved source is known without reading the code again. The root expands a
family by re-measuring every one of its placements with the published
source; a placement whose relocation context differs is refused and stays
out. The expansion never infers a placement from a sibling: it measures it.

## 5. The root integration

The root is the only writer in the clone. It:

1. re-measures every candidate body (recompiles it, verifies the object
   hash) — a lot's claim is a claim, not a fact;
2. merges bodies that produce the same compiled code into one source, so a
   shared family is one file and several rows;
3. copies the sources into `src/`, and writes the placement table
   (`module, offset, size, source, source hash, object hash, profile, as`);
4. runs the **complete-program gate**: rebuild every affected program,
   require its hash to equal the pinned retail hash, and require the
   previously measured load images to match;
5. regenerates the catalogues and the progress snapshot with their tools —
   never by hand;
6. runs the tests and the repository guard;
7. updates the documentation, commits on a branch, and opens a pull request.

## 6. What the gate does and does not prove

A gate that rebuilds the complete program and compares its hash proves that
the bytes are right **at that build**. It does not prove that the build is the
original build, that the game behaves identically, or that the source is
semantically the original one. Those are separate claims, stated separately.

## 7. When a body will not match

Three families of failure recur. Treat them differently:

| Symptom | Reading | Route |
| --- | --- | --- |
| same instruction sequence, different register numbers | the allocator, not the source | recover the authentic form; compare with the sibling project that matched the same routine |
| correct size, one instruction different | an idiom or a constant | look for the authentic source, the literal, the flag |
| refuses to link | a missing symbol or a wrong extent | fix the mapping and the bound, not the code |

After three attempts in the same class, park the body with its first
difference and a reopening condition, and move on. A parked body with a
condition is progress; a body claimed without a proof is not.
