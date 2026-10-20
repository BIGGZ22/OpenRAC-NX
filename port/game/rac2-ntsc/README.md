# Going Commando NTSC-U native start-up

`openrac-rac2-ntsc` boots Ratchet & Clank: Going Commando (SCUS-97268) from
its decompilation to the front end's main menu and works it: the splash and
the memory card check, the intro movies (skipped with `--skip-movies`), the
menu's three buttons with their labels (New Game, Load, Options), Up and Down
moving the selection, Cross opening an entry and Triangle backing out. Each
entry's page draws: Options (16:9, Progressive Scan, Vibration, Subtitles,
navigable), Load (with no card: "Please insert a Memory Card ... Go back?")
and New Game (with no card: "No Memory Card (PS2) inserted ... Continue
anyway? Yes / No"). Yes there starts the game: the first level's load is
where it stops (below).

A run that does all of it, hidden, with the frames on which the pad is
pressed (Cross 0x4000, Start 0x8, Down 0x40, Up 0x10, Triangle 0x1000):

```sh
OPENRAC_HIDDEN=1 OPENRAC_PRESS=300:4000:10,500:8:10,700:4000:10,900:8:10,1300:40:8,1340:40:8,1400:4000:8,1500:40:8,1600:1000:8,1700:10:8,1760:4000:8,2000:1000:8 \
  port/build/release/openrac-rac2-ntsc --data build/rac2-data --window --frames 2200 \
  --no-card --skip-movies --stop-on-missing
```

## Building and running

The decompilation this was brought up with is the live `rac2-decomp`
checkout (about 170 more functions in C than OpenRAC's `games/rac2/ntsc`,
synced at 86dbd852); its build products are used where present:

```sh
cd port
cmake --preset release -DOPENRAC_RAC2_NTSC_SOURCE=<rac2-decomp checkout>
cmake --build --preset release -j 8 --target openrac-rac2-ntsc
python3 tools/extractor.py <SCUS_972.68 image> --game rac2 --proj-path build/rac2-data -e --boot-only   # from the top
OPENRAC_HIDDEN=1 OPENRAC_PRESS=300:4000:10,500:8:10,700:4000:10,900:8:10 \
  port/build/release/openrac-rac2-ntsc --data build/rac2-data --window --frames 1500 \
  --no-card --skip-movies --stop-on-missing
```

What the translation reads from the decompilation (`hostgen.json`):

- `symbol_addrs/usa/symbol_addrs.txt`, and `build/boot/placed.json` (as
  aliases and as symbols): the values the decompilation's own build derived
  for the names it carries from rac1-decomp (`r1_func_...`, `r1_D_...`) and
  for its family-qualified ones (`F<hash>_FUN_<addr>`). Without a build of
  the decompilation (CI) they are absent and those functions are stubs.
- `baserom/SCUS_972.68` (`asm.retail`): every function still in assembly is
  translated from the retail executable's instructions, entries found by
  their calls (`discover_entries`). Its `include_asm.h` defines the markers
  to nothing, so `defines` sets `INCLUDE_ASM_H` and gives `ASM_FUNC` the
  marker hostgen reads.
- `skip`: libgcc's `__udivdi3`, `__umoddi3` (host rows) and fp-bit's
  `__pack_f` file, whose `mode` typedefs Clang reads but hostgen cannot
  prototype; translated from assembly instead.
- `from_assembly`: 15 wrappers whose C omits registers they forward
  (`arguments_filled_with_zero` in the report), translated from assembly.

## The port's own code (`host/`)

| File | Replaces |
|---|---|
| `font.c` | the font prints (`0x0027F538` and nine siblings) and the GUI text element's draw (`0x00336A78`), passing the font's TEX0 from `0x0027C878` whole, as rac1's port does for FontPrintWindow (`rac1/host/hand.c`) |
| `entry.c` | crt0 (`0x00131AE8`): `_InitSys`, `FlushCache(0)`, `main(argc, argv)` with the block at `0x00146B40`, without the thread and heap system calls; `_InitSys`'s scan of kernel memory (`0x0011F718`) and its Deci2 console thread (`0x0011B800`) |
| `dma.c` | `sceDmaSend` (`0x00127220`), leaving CHCR as after an end tag so the game's VIF1 handler (`0x002FCE18`) retires the whole frame wait mask (`0x001AD520`); `KickVif0Chain` (`0x002FC070`) |
| `io.c` | `sceOpen` (`0x0011E0D8`), declared variadic |
| `movie.c` | `PlayFmvMovie` (`0x0034EF58`: first sector, size, ...): the shared native player |
| `print.c` | `printf`, `sprintf`, `kprintf`, `scePrintf`: formatted by the port |
| `sound.c` | `snd_SendCommandSync` (`0x00132C48`) and `snd_QueueCommandToRing` (`0x00132DF0`): commands the native engine knows, the others answered 0 without an IOP |
| `trig.c` | the VU0 sine and cosine (`0x00283198`, `0x00283180`), from the microprogram the game loads at `0xC80` |
| `wad.c` | the WAD decompressor (`0x0029D4D0`) |

`libraries.tsv` binds 164 entry points to the shared replacements: the
names in the symbol table, the kernel's system-call wrappers by the number
each loads into `$v1`, and the SDK functions whose instructions (relocations
masked) are those of rac1-pal's or rac4's bound ones. Several of the latter
start four bytes after their catalogue place, at the linker's fill word.

## The texture word (TEX0)

`0x0027C878` (rac1's GetEffectTex) returns a texture's whole TEX0 from its
cache at `0x001B9740`: TBP0, width and format in the low word, CBP and the
palette's load control in the high one. It is in `returns_64`, the glyph
and quad builders that take it (`0x0027E1A0` DrawTexturedQuad2d, `0x0027F2A8`,
`0x0027FC70`, `0x0027F640`) have 64-bit `asm.signatures` (DrawTexturedQuad2d
keeps it with an `sd` in a call's delay slot, which the register inference
does not see as an input), and the C that held it in an `int` is replaced by
`host/font.c`. This is rac1's pattern (`returns_64` on func_001F4868 and its
FontPrintWindow wrappers).

## Not there yet

- The first level's load (New Game, then Yes): after the intro cinematic
  (skipped) the loader (0x002EC408 -> 0x002EAB18 -> 0x00294070 -> 0x00293D30)
  reads 635 sectors at 1277354 to 0x00A54000 and hands them to the WAD
  decompressor, which finds no WAD header there. Levels also need their
  programs and the overlay hook, the level-copy functions (`func_..._N`)
  that register at one address in overlay 30 (logged at start), and the
  world renderer.
- The menu's selected button differs from the others only faintly (a pixel
  difference of at most 32 of 255): its highlight may not be drawn as the
  game draws it.
- The Options page's value boxes (beside each row) stay empty: their four
  32x32 8-bit icons (TBP0 0x2C88 to 0x2C98) are drawn with a TEX0 whose
  upper word is 0 (no palette, CBP 0) in the display list at 0x0035C200;
  the full value is never in memory, so something between their VRAM
  allocation and the packet keeps it in 32 bits. Not found yet.
- Sound: only play and parameter commands reach the native engine; the
  banks, the master volume and the streams are dropped with a warning.
- `rom0:ROMVER` cannot be opened (the game logs it and goes on).
- Levels: their programs and the overlay hook, the level-copy functions
  (`func_..._N`) that register at one address in overlay 30 (logged at
  start), the world renderer.
