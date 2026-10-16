# The frame: render order and the display list

How a RAC1 frame is built: the main loop, the world render function and the
order of its passes, the second half that lights and patches on the EE while
VU1 draws, the chain builders, the GS state each pass sets, and texture
paging. [RENDERER.md](../../port/RENDERER.md) describes the same frame from
the PAL executable; this page adds ReRAC's reading of the NTSC-U level 01
program (`SCUS_971.99`), with PAL names for every function the two share.

**Games.** RAC1. RAC2's boot has the same section shape and VU program
table ([README.md, section 2](../README.md#2-the-games-and-their-programs));
its frame is not measured.

## 1. Main loop

The level's main loop (NTSC-U level 01 0x259C40, PAL `func_L00_002465F8`),
each iteration:

1. `VU1_sendChain` (PAL `func_002349B8`) sends the chain built last
   iteration; `VU1_swapChain` (`func_00234948`) switches buffers.
2. `PutDrawBufferSmall`, the anti-aliasing blur pass (`AA_BlurPass`
   `func_001FB848`), `PutDrawBufferLarge`.
3. A switch on the game mode: in play, the state update
   (`dispatch_game_state_update` `func_00232200`) then the world render
   (`DrawWorld`, NTSC-U boot 0x1F39D0, PAL `func_001F3D78`, which PAL's
   names table calls `DrawDebugProfiler`); mode 4 a pause variant
   (`DrawWorldPaused` `func_002305A0`); mode 5 the vendor screen render
   (`VendorModeRender` `func_L00_002A1540`).
4. `VU1_syncChain(1)`.

The console draws frame N − 1 while the game builds frame N.

## 2. The world render, in order

Each pass is switched by a bit of the draw mask (PAL `D_0018A3B0`):

1. Full-screen clear packet, only when the level has no sky or the sky's
   clear flag is set ([SHRUB_SKY.md](../formats/SHRUB_SKY.md#8-clears-and-rotation-level-code)).
2. `UpdateOcclusion` ([OCCLUSION.md](OCCLUSION.md)).
3. `ResetGsRegisters` (`func_001F3C10`).
4. **Sky**: per-level dispatch, shells drawn by the EE.
5. **Terrain**: `DrawTfrag` (`func_002346C0`), then an interrupt tag
   (0x2010000).
6. **Ties**: `DrawTies_1` or `DrawTies_2` (`func_00236BE0`,
   `func_00236CA8`); ReRAC reads `DrawTies_2` as the PAL variant chosen by the
   PAL flag, not a reflection pass. Interrupt tag 0x2020000.
7. **Shrubs**: `DrawShrubs` (`func_00229E50`). Interrupt tag 0x2040000.
8. A fogged full-screen sprite.
9. **Mobys**: `DrawMobys` (`func_0020E2B0`). Interrupt tag 0x2080000.
10. Registered draw callbacks (`ExecuteDrawCallbacks` `func_001F4A00` and
    three siblings): weapons and effects.
11. **Particles**: `PartProc` (`func_00218B10`).
12. The anti-aliasing blit, when enabled.
13. HUD GS state (TEST_1 0x5380B, ALPHA_1 0x44), then the HUD, help box and
    screen fade: the HUD is drawn **after** the blit ([HUD_TEXT.md](HUD_TEXT.md)).
14. `DoGifPaging` (`func_001F4748`).
15. Underwater tint and black or white fades.
16. VU0 program 436083 is loaded.

Then the **EE half**, between `VU1_syncChain(2, 4, 8, 0x10)` waits on the
interrupt tags above: `LightTfrags` and `PatchTfragGifs`, `LightTies` and
`PatchTieGifs`, `LightShrubs` and `PatchShrubGifs`, `PatchMobyGifs`, and
`UpdateFog`. So lighting and texture addresses computed this frame are used by
next frame's chain ([LIGHTING.md](LIGHTING.md)).

## 3. VU programs and how they are loaded

- **VU0** programs go through `VU0_loadMicroProgram` (NTSC-U boot 0x2334D8,
  PAL `func_002347F0`), a VIF0 chain transfer: 104691 by `DrawMobysSetup`
  every frame, 436083 at the end of the world render and at level load, the
  upper part of 28259 once by `InitOnce`.
- **VU1** programs are REF tags in the VIF1 chain (`VU1_addDataRef`
  `func_00234B48`, or written straight into a scratchpad-built chain). A
  global records the resident program (6 mobys, 7 sprites, 8 particles), reset
  each frame, so a program is re-sent only when it changes.

Which program each pass uses: [VU1.md](VU1.md).

## 4. Chain builders

Shared helpers (PAL names): `write_vif_unpack_packet` (`func_00234BA0`;
STCYCL 4/4 and UNPACK V4-32), `VU1_addGSregister` (`func_00234C98`; a
two-quadword DIRECT A+D write), `VU1_setScissor` (`func_00234D58`),
`write_dma_channel` (`func_001F9AF0`; scratchpad to memory), `Vif1ChainCmd`
(`func_00235290`; the interrupt tags).

| Pass | What its builder does |
|---|---|
| Terrain | reserves a NEXT tag, unpacks two camera matrices (VU 5 and 0x14D), runs `TfragProc` (fragment headers streamed to the scratchpad 32 at a time, culled with VU0 macro code, packets built in the scratchpad and copied out), splices the texture uploads (`DmaTfragTextures` `func_002344D8`), copies out the visible list for lighting |
| Ties | `TieProc`, then the visible list and texture uploads |
| Shrubs | `ShrubProc`, the visible list, texture uploads |
| Mobys | `DrawMobysSetup` (program 13859, TEST_1 0x5360B, chain start), `MobyProc`, `DrawMobysCleanUp`; `DrawMobyList` (`func_0020E180`) is the per-list version for menus |

**Texture address patches**: each pass's patch walks the GS setups it
listed this frame and ORs the paged TBP0 (and CBP) into TEX0: terrain from a
(TBP, TBP1) table by texture index ([TFRAG.md](../formats/TFRAG.md#5-textures-the-gs-setup)),
ties (`PatchTieGifs` `func_00236A98`), shrubs, mobys (`PatchMobyGifs`
`func_0020DD48`).

## 5. GS state

`ResetGsRegisters` sends two A+D blocks (19 and 11 registers) and FOGCOL.
`SetPalMode` (`func_001F3890`) fills them:

| | NTSC | PAL |
|---|---|---|
| Frame buffer, Z buffer, display (GS addresses) | 0x0E0000, 0x1B0000, 0x280000 | 0x100000, 0x1E0000, 0x2C0000 |
| Draw / display size | 512 × 416 / 512 × 448 | 512 × 448 / 512 × 512 |
| Video mode set at init | 2, interlaced, FFMD 0 | 3, interlaced, FFMD 0 |

Z is PSMZ24; XYOFFSET is (0x800 ± offset) × 16. Per-pass state through
`VU1_addGSregister`: TEST_1 0x5360B for opaque world (alpha GEQUAL 0x60 with
keep-colour on fail, Z GEQUAL), 0x3004B, 0x2004B and 0x31801 for UI and alpha
passes; ALPHA_1 mostly (Cs − Cd) · As + Cd, also additive forms; CLAMP_1 0 or
5. Full-screen anti-aliasing and field resolve packets are built once
(`SetupFS_AA_buffer`) and re-sent each frame.

## 6. Texture uploads

`BuildTfragTextureDma` builds, in the scratchpad, per texture an A+D block
(BITBLTBUF, TRXPOS, TRXREG, TRXDIR) and an IMAGE tag, from a usage bitmap
`ComputeTfragTextureUsage` fills; the per-pass DMA functions splice the
sub-chain in with NEXT. HUD and 2D textures use `SetupGifPaging`
(`func_001F4630`, reserves four quadwords) and `DoGifPaging`; HUD banks go
through `Hud_sendTexture` (`func_00201348`) and `Hud_SendResidentBank`
(`func_001FF958`); particles through `_part_load_tex` (`func_00219704`).
GS block 0x3FFB holds a permanent 8 × 8 texture of 0x80 texels, used by
untextured moby faces ([MOBY.md](../formats/MOBY.md#3-packets)).

## Open

- The exact role of the fogged full-screen sprite between shrubs and mobys.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/render_pipeline.md`,
`docs/plan/hud_text.md`; PAL names from
`games/rac1/config/overlays/us_map.tsv` and
[RENDERER.md](../../port/RENDERER.md).
