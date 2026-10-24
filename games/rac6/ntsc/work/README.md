# Work templates

The private workspace lives on the local disk, next to the working clone; the
repository holds only the **templates** of that layout, so that a fresh
machine can be set up in minutes. The real paths are in `LOCAL.md`
(git-ignored).

```
work/                        (private, local, never published)
  E0/          the foundation lot: extraction, identity, bounds, shortlist, harness
    modules/   the extracted target
    bounds/    the bounded candidates
  lots/        the lot tables (L01.tsv, L02.tsv, ...), one per agent
  w1/          the wave 1: one dossier per lot
    L01/         bodies/        one authored C source per placement
                 abandonnes/    parked attempts, with their first difference
                 work/          the agent's scratch (never published)
                 resultats.tsv  the measured table
                 RAPPORT.md     what was measured
  briefs/      CONSIGNE-VAGUE-N.md and BRIEF-COMMUN.md of the wave
  tools/       the kit's scripts (never published)
  STATE.md     the anchor of the workspace
```

`CONSIGNE-VAGUE-N.md` is the root's plan for a wave: the departure state, the
lots, and the integration steps. `BRIEF-COMMUN.md` is the brief every agent
of the wave reads. `RAPPORT.md` is what an agent hands back.
