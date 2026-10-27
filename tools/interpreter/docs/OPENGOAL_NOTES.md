# How OpenGOAL draws: notes for the runtime

[OpenGOAL](https://github.com/open-goal/jak-project) (jak-project, ISC) is the
native port of the Jak and Daxter games, which ran on the same hardware in
the same years and reach it through the same Sony libraries. These notes
record how its renderer and platform layer work and what of that carries
over to the Ratchet & Clank games. [DESIGN.md](DESIGN.md) is built on them.

Read from jak-project at `efb21c3e8` (2026-09-30). Paths are relative to that
repository; line counts are `wc -l` of the files named (cpp and h unless
stated). Everything here is **reference** in OpenRAC's evidence levels: it
describes another project's code. A statement marked **(inference)** is a
conclusion and is not something that code states. The large generated files
(`OceanMid_PS2.cpp`, `OceanNear_PS2.cpp`, `Shadow_PS2.cpp`,
`TextureAnimator*.cpp`) were read for their structure and samples only. No
code from jak-project is in OpenRAC; if any is adapted later, it is listed in
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md) with its notice.

---------------------------------------------------------------------------------------------------

## 0. Orientation: what OpenGOAL actually is (this shapes everything below)

* The Jak games are ~98% GOAL (Naughty Dog's Lisp). The project decompiles that to OpenGOAL source (`goal_src/`), compiles it with its own compiler (`goalc/`) to **native x86-64 machine code** (an ARM64 backend, `goalc/emitter/IGenARM64.*`, exists and is "experimental, unsupported" on Mac), and loads that code into a 128 MB host buffer that plays the role of EE RAM (`game/runtime.cpp` `ee_runner`: `mmap` at `EE_MAIN_MEM_MAP = 0x2123000000`, "intentionally > 32-bit to catch pointer bugs", `common/goal_constants.h`). The game's 32-bit "pointers" are offsets from `g_ee_main_mem`.
* So the game code runs **natively on a game thread** and builds **real PS2 DMA chains in that buffer** (real DMA tags, real VIF codes, real GIF packets, real VU1 data-memory layouts). The renderer then walks those chains. This is why `common/dma/` is such a faithful PS2 model.
* BUT the game code was **modified for the PC port** (`#when PC_PORT`; 65 files in `goal_src/jak1` mention `PC_PORT`). It (a) injects extra "PC_PORT" packets (a custom VIF command, `vif-cmd pc-port` = 8, `goal_src/jak1/engine/dma/dma-h.gc:297`) carrying camera/level-name/model-name/bone-pointer data, (b) skips sending VU1 microprograms (`MPG`) and, for background and merc, skips the geometry DMA entirely because the renderer draws pre-converted geometry instead (section 3). Quote from `game/graphics/opengl_renderer/sprite/Sprite3.cpp`: "next would be the program, but it's 0 size on the PC and isn't sent."
* Level/model/texture assets are **extracted offline** by the decompiler (`decompiler/level_extractor/`, 18k lines) into `.fr3` files (zstd-compressed `tfrag3::Level`, `common/custom_data/Tfrag3Data.h`) that the runtime loads (`game/graphics/opengl_renderer/loader/`).
* Only one graphics backend exists: OpenGL via SDL3 (`GfxPipeline` enum has just `Invalid` and `OpenGL`; `game/graphics/gfx.h`). The `GfxRendererModule` struct of `std::function`s is an abstraction point for a second backend, but nothing else implements it.
* Jak 1 is the closest sibling to R&C1 (first-generation, same era, fewer features). Jak 2/3/X code paths exist side by side (`GameVersion` switches everywhere); These notes follow the Jak 1 paths.

---------------------------------------------------------------------------------------------------

## 1. Frame hand-off: game -> renderer

### 1.1 Threads (`game/runtime.cpp`, `exec_runtime`, lines ~415-525)

| Thread | Created by | Role |
|---|---|---|
| main (process main thread) | `exec_runtime` | `Gfx::Init` (SDL, window, GL context) then `Gfx::Loop`: `while (MasterExit == RUNNING) Display::GetMainDisplay()->render();` = the **render thread** (comment: "run video loop on main thread") |
| "EE" | `tm.create_thread("EE")`, `ee_runner` | maps EE RAM, runs `jak1::goal_main` = the whole game, native code |
| "IOP" | `iop_runner` | emulated IOP: overlord (file/stream/sound-command IRX reimplemented in C++), IOP kernel with coroutines (section 7) |
| "DMP" | `deci2_runner` | TCP debugger/REPL link (not needed by us) |
| "EE-Worker" | `ee_worker_runner` | background job queue for the EE thread |

`Gfx::Init` runs on the main thread **before** the EE thread starts ("initialize graphics first - the EE code will upload textures during boot and we want the graphics system to catch them"). All GL calls happen on the main thread only (no GL call reachable from the EE thread: `texture_upload_now` only edits tables, see 5.2). **(inference from reading: the main-thread rule is the macOS/Cocoa windowing constraint; the code comment just says "main thread")**.

### 1.2 The call chain, game side to renderer entry point

1. GOAL `display-sync` / `display-frame-finish` in `goal_src/jak1/engine/draw/drawable.gc` (~l.1180-1235):
   ```
   (sync-path 0 0)                      ; wait for previous frame's rendering to finish
   ...
   (let ((syncv-result (syncv 0)))      ; wait for the vsync (renderer swapped buffers)
     ...
     (__send-gfx-dma-chain (the-as dma-bank-source #x10009000)
                           (-> dma-buf-to-send data-buffer))   ; hand over this frame's chain
   ```
   (`#x10009000` is the VIF1 DMA channel address; it is ignored on PC.) `sync-path` and `syncv` are bound to C++ in `game/kernel/jak1/kmachine.cpp:579-581` (`sceGsSyncPath`, `sceGsSyncV` in `game/graphics/sceGraphicsInterface.cpp`, 34 lines). `sceGsSyncPath` = `Gfx::sync_path()`, `sceGsSyncV` = `Gfx::vsync()`.
2. `__send-gfx-dma-chain` is registered in `game/kernel/common/kmachine.cpp:1119` and implemented at l.488:
   ```cpp
   void send_gfx_dma_chain(u32 /*bank*/, u32 chain) {
     if (Gfx::GetCurrentRenderer())
       Gfx::GetCurrentRenderer()->send_chain(g_ee_main_mem, chain);
   }
   ```
3. `gl_send_chain(const void* data, u32 offset)` in `game/graphics/pipelines/opengl.cpp` (~l.690):
   ```cpp
   std::unique_lock<std::mutex> lock(g_gfx_data->dma_mutex);
   if (g_gfx_data->has_data_to_render) { lg::error("... called multiple times per frame?"); return; }
   g_gfx_data->dma_copier.set_input_data(data, offset, run_dma_copy);   // run_dma_copy == false
   g_gfx_data->has_data_to_render = true;
   g_gfx_data->dma_cv.notify_all();
   ```
4. Render thread: `GLDisplay::render()` (opengl.cpp ~l.360-480) per loop: poll input, SDL events, ImGui new frame, then `render_game_frame(...)`, which does
   ```cpp
   got_chain = dma_cv.wait_for(lock, std::chrono::milliseconds(40), [=]{ return has_data_to_render; });
   if (got_chain) {
     ...
     g_gfx_data->ogl_renderer.render(DmaFollower(g_gfx_data->dma_copier.get_last_input_data(),
                                                 g_gfx_data->dma_copier.get_last_input_offset()), options);
   }
   // before vsync, mark the chain as rendered:
   has_data_to_render = false; sync_cv.notify_all();
   ```
   (The 40 ms timeout is so ImGui stays responsive if the game produces nothing.)
5. **Renderer entry point:** `OpenGLRenderer::render(DmaFollower dma, const RenderOptions& settings)` (`game/graphics/opengl_renderer/OpenGLRenderer.cpp:984`, class in `OpenGLRenderer.h`, 152 lines). It does `setup_frame` -> loader update -> `dispatch_buckets` (the bucket loop, section 3) -> `blit_display` -> `do_pcrtc_effects` (final blit to window, letterbox, brightness/contrast, blackout fade) -> optional screenshot/debug windows.

### 1.3 What exactly is handed over

* **A raw pointer to the whole 128 MB EE RAM image plus a 32-bit offset of the first DMA tag.** No copy, no ownership transfer, in the default configuration. `constexpr bool run_dma_copy = false;` at the top of `opengl.cpp`.
* The renderer thread therefore reads live game memory while the game thread keeps running. Safety relies on the game's own double-buffering (two frames of DMA buffers; `display-sync` waits in `sync-path` before reusing the buffer). Data *referenced* from the chain (bone matrices, lights, textures, vis bits) is not protected. Example: `Merc2::handle_pc_model` resolves matrix pointers inside the packet with `setup.data - setup.data_offset + addr` into live memory. The author's own comment in `gl_send_chain` lists the motivation for the optional copy: "if the game code has a bug and corrupts the DMA buffer, the renderer won't see it ... the copied DMA is much smaller than the entire game memory, so it can be dumped to a file ... it verifies the DMA data is valid early on. But it may also be pretty expensive. Both the renderer and the game wait on this."
* Optional copier (off): `common/dma/dma_copy.{h,cpp}` `FixedChunkDmaCopier` (254 lines). Treats RAM as 128 KB chunks (1024 chunks), walks the chain with `DmaFollower`, marks every chunk containing a tag or its payload, copies only those, and **patches the `addr` field of each REF/NEXT/CALL tag** to the compacted layout (`Fixup` list). Has a `verify` mode that flattens both chains (`flatten_dma`) and diffs them, and `serialize_last_result` to dump a frame to disk. Useful idea for capture/replay and unit tests.
* Side channels that bypass the chain (all called from the EE thread, defined in `GfxRendererModule`, `gfx.h`): `texture_upload_now(tpage, mode, s7)` (-> `TexturePool::handle_upload_now`, takes a mutex, no GL), `texture_relocate`, `set_levels`/`set_active_levels`/`force_reload_*` (-> `Loader`), `set_pmode_alp` (PCRTC blackout alpha). Plus `Gfx::vsync/sync_path`.

### 1.4 Vsync and timing

State in `GraphicsData` (opengl.cpp): `sync_mutex/sync_cv`, `dma_mutex/dma_cv`, `frame_idx`, `frame_idx_of_input_data`, `has_data_to_render`, `FrameLimiter frame_limiter`, `Timer engine_timer`, `last_engine_time`.

* `gl_sync_path()` (EE thread, = `sceGsSyncPath`): records `last_engine_time = engine_timer.getSeconds()`, then if `has_data_to_render`, waits on `sync_cv` until the renderer cleared it. i.e. "wait until the GS has finished the last frame".
* `gl_vsync()` (EE thread, = `sceGsSyncV`): `init_frame = frame_idx_of_input_data; sync_cv.wait(... frame_idx > init_frame ...)`; returns `frame_idx & 1` (odd/even field, used by the game as `*oddeven*`). It is released at the end of `GLDisplay::render()` after `SDL_GL_SwapWindow`: `frame_idx++; sync_cv.notify_all()` (comment: "toggle even odd and wake up engine waiting on vsync").
* Pacing is on the render thread, not the game: after rendering, `FrameLimiter::run(target_fps, experimental_accurate_lag, sleep_in_frame_limiter, last_engine_time)` (`common/util/FrameLimiter.{h,cpp}`, 81+ lines) sleeps most of the remaining time then spin-waits; `SDL_GL_SwapWindow`; `SDL_GL_SetSwapInterval(vsync)` is toggled lazily from settings. Default `target_fps = 60`, `vsync = true`, `framelimiter = true` (`gfx.h` `GfxGlobalSettings`). The `experimental_accurate_lag` mode rounds the engine time up to a multiple of 1/60 s to imitate PS2 frame-skipping lag (30 fps when the game misses 60).
* Resulting pipeline: the game builds frame N+1 while the renderer draws frame N (one frame of latency), exactly the PS2 behaviour. `sceGsSyncV` blocks the game until the *swap*, so the game cannot run ahead more than one frame.
* The renderer calls back **into the game** from the render thread: `vif_interrupt_callback(bucket_id)` (`kmachine.cpp:417`) calls `call_goal(vif1_interrupt_handler, bucket_id, ...)` after each bucket, to imitate VIF1 interrupts (the game uses them for profiling). A cross-thread call into game code is a hazard you may not want to copy **(inference)**.
* Resolution handling: the game always renders into an FBO of `game_res_w x game_res_h` (default 640x480, optional MSAA), then `do_pcrtc_effects` blits it to the window with letterboxing (`draw_region_*`, `draw_offset_*`), applying `shaders/post_processing.frag` (brightness/contrast) and a blackout alpha from `pmode_alp` (emulates PCRTC PMODE.ALP fades).

---------------------------------------------------------------------------------------------------

## 2. DMA, VIF and GIF decoding utilities

### 2.1 Files

| File | Lines | What it contains |
|---|---|---|
| `common/dma/dma.h` | 174 | `DmaStats`; `DmaTag` (decodes a 64-bit tag: `qwc`, `addr` (31 bit), `spr`, `kind` = REFE/CNT/NEXT/REF/REFS/CALL/RET/END); `emulate_dma(src_base, dst_base, tadr, dadr)` (tiny chain copier); `VifCode` (decodes a 32-bit VIF code: `interrupt`, `kind`, `num`, `immediate`; `Kind` enum has NOP, STCYCL, OFFSET, BASE, ITOP, STMOD, MSK3PATH, MARK, FLUSHE/FLUSH/FLUSHA, MSCAL/MSCNT/MSCALF, STMASK, STROW, STCOL, MPG, DIRECT/DIRECTHL, UNPACK_V4_32/V4_16/V3_32/V4_8/V2_16, plus fake `PC_PORT`=8, `PC_PORT2`=9); `VifCodeStcycl{cl,wl}`; `VifCodeUnpack{addr_qw,is_unsigned,use_tops_flag}` |
| `common/dma/dma.cpp` | 129 | `DmaTag::print`, `VifCode::print` (debug strings) |
| `common/dma/dma_chain_read.h` | 134 | `DmaTransfer` (pointer + offset + size + the 64 bits of VIF tags that ride in the upper half of the DMA tag: `vif0()`, `vif1()`, `vifcode0()`, `vifcode1()`, `read_val<T>`), and **`DmaFollower`** (header-only chain walker) |
| `common/dma/dma_copy.{h,cpp}` | 56 + 198 | `FixedChunkDmaCopier`, `flatten_dma`, `diff_dma_chains` (see 1.3) |
| `common/dma/gs.h` | 592 | GS/GIF types, see 2.3 |
| `common/dma/gs.cpp` | 463 | `print()` helpers and register-name tables |
| `game/graphics/opengl_renderer/dma_helpers.{h,cpp}` | 50 + 139 | `verify_unpack_with_stcycl`, `verify_unpack_no_stcycl`, `unpack_to_stcycl`, `unpack_to_no_stcycl`, `verify_mscal`: assert the transfer is exactly the expected STCYCL/UNPACK/MSCAL shape, then `memcpy` the payload |
| `common/texture/texture_conversion.h` | 225 | GS local-memory address functions `psmct32_addr(x,y,w)`, `psmt8_addr`, `psmt4_addr_half_byte`, `psmct16_addr` (block/column tables from the GS manual) and `rgba16_to_rgba32`. Self-contained. |
| `common/texture/texture_slots.{h,cpp}` | 7 + 201 | static lists of animated-texture slot names for Jak 2/3 (not needed) |

### 2.2 `DmaFollower` semantics (this is the core iteration primitive)

```cpp
DmaFollower dma(base_ptr, start_offset);
while (!dma.ended()) { DmaTransfer t = dma.read_and_advance(); /* t.data, t.size_bytes, t.vifcode0()/1() */ }
dma.current_tag(), current_tag_vifcode0()/1(), current_tag_offset(), ended()   // peek without advancing
```
* One call = one DMA tag = one `DmaTransfer`. The 64 bits after the tag (`transferred_tag`) are the two VIF codes the PS2 would have pushed down VIF1 ahead of the payload.
* Tag handling: `CNT` (asserts `addr == 0`, data follows the tag), `NEXT` (data follows, jump to `addr`), `REF/REFS` (data at `addr`, next tag is +16; REFS treated as REF), `REFE` (ends), `CALL` (data follows, push return address; **stack depth limit 2**, `m_stack[2]`), `RET` (pop), `END`. `ASSERT(!tag.spr)`: **scratchpad-sourced chains are not supported**. TTE/PCE bits are not modelled.
* It is a *reader of a snapshot*: never validates addresses against the buffer size. Malformed input = memory error (this is why the optional copier has a verify mode).
* Where it is used: everywhere in `game/graphics/opengl_renderer/*`. Each bucket renderer receives `DmaFollower& dma` by reference and advances it until `dma.current_tag_offset() == render_state->next_bucket`.

### 2.3 GS/GIF model in `common/dma/gs.h`

* `GifTag` (16 bytes): `nloop()`, `eop()`, `pre()`, `prim()`, `flg()` (PACKED/REGLIST/IMAGE/DISABLE), `nreg()`, `reg(i)` (`RegisterDescriptor`: PRIM, RGBAQ, ST, UV, XYZF2, XYZ2, TEX0_1/2, CLAMP_1/2, FOG, XYZF3, XYZ3, AD, NOP).
* `GsRegisterAddress` enum (all the A+D addresses: PRIM, RGBAQ, ST, UV, XYZF2, XYZ2, TEX0_x, CLAMP_x, FOG, TEX1_x, TEX2_x, XYOFFSET_x, MIPTBP1/2_x, TEXA, FOGCOL, TEXFLUSH, SCISSOR_x, ALPHA_x, DIMX, DTHE, COLCLAMP, TEST_x, PABE, FBA_x, FRAME_x, ZBUF_x, BITBLTBUF, TRXPOS, TRXREG, TRXDIR, HWREG, SIGNAL, FINISH, LABEL).
* Bit-field wrappers around 64-bit register values, each with named accessors: `GsPrim` (kind POINT..SPRITE, `gouraud()`, `tme()`, `fge()`, `abe()`, `aa1()`, `fst()`, `ctxt()`, `fix()`), `GsTex0` (tbp0, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm), `GsTex1` (lcm, mxl, mmag, mmin, mtba, l, k), `GsTexa`, `GsTest` (atest, aref, afail, date/datm, zte, ztest), `GsAlpha` (a/b/c/d mode + fix), `GsZbuf` (zbp, psm, zmsk), `GsFrame` (fbp, fbw, psm, fbmsk), `GsScissor`, `GsXYOffset`.
* `AdGifData` = 5 quadwords (tex0, tex1, miptbp, clamp-or-zbuf, alpha, each with its A+D address): this is Jak's in-memory "shader" record and appears in all geometry types. `is_normal_adgif()` checks the addresses. (`AdgifHandler.h`, 49 lines, wraps it as `AdgifHelper`.)
* **`DrawMode`** (also in `gs.h`, ~200 lines): the *renderer-side* packed 32-bit state key: depth write, ztest (2b), clamp s/t, filter, tcc, aref (8b), alpha test, ate, zte, abe, alpha-fail, alpha-blend preset (3b: `DISABLED, SRC_DST_SRC_DST, SRC_0_SRC_DST, SRC_0_FIX_DST, SRC_DST_FIX_DST, ZERO_SRC_SRC_DST, SRC_SRC_SRC_SRC, SRC_0_DST_DST`), !decal, fog. Used as a sort/batch key (e.g. Sprite3 uses `(tbp << 32) | mode.as_int()`; Generic2 adds `fix` and hud flags). This 32-bit key + TBP is how all the "3D" renderers batch draws.

### 2.4 Dependencies / self-containment

* Headers needed: `common/common_types.h` (36 lines, fixed-width typedefs), `common/util/Assert.h` (50) + `Assert.cpp` (41; the failure path calls `lg::die` from `common/log/log.h`), `fmt` (header `fmt/format.h`, used for `print()`), `<cstring>`. `dma_copy.cpp` additionally uses `common/goal_constants.h` (for `EE_MAIN_MEM_LOW_PROTECT`), `common/util/Timer.h`, `common/log/log.h` and `common/util/Serializer.h` (233, header-only, only for `serialize_last_result`). `DmaFollower` itself needs only `dma.h`.
* Practical assessment: `dma.h`, `dma_chain_read.h`, `gs.h/.cpp`, `texture_conversion.h` can be lifted nearly verbatim (swap `ASSERT` and `fmt`). `dma_helpers.*` also (it uses `lg::error` and `ASSERT`). Roughly 1,750 lines for common/dma plus 190 for dma_helpers.
* **What is NOT there (important):**
  * There is **no generic VIF interpreter**. No STMOD/STMASK/STROW/STCOL handling, no V4_8/V4_16 unpack conversion, no write-recycle (STCYCL cl/wl skipping) engine, no MPG upload, no MSKPATH3, no PATH2/PATH3 mixing. Each renderer hard-codes the exact shape of the transfers the game emits and `ASSERT`s on any deviation ("fail fast"). The exceptions are `OceanMid.cpp` (hand-coded unpack of `STCYCL 0x404/0x204 + UNPACK_V4_8` into a VU data array, including the cl=4,wl=2 skip pattern `addr_off = 4*(i/2) + i%2`) and the plain `memcpy` of V4_32 data.
  * There is **no generic GS emulator**. GIF data is interpreted by `DirectRenderer` (2.5 below and section 4) for the subset of registers the game's "direct" paths use, and by the specialised renderers for their own packets.
  * `common/dma` does not know VU1 at all; VU-side knowledge lives in the renderers (section 4).
* The renderers are **not tolerant**: they `ASSERT` hundreds of structural expectations (e.g. `TFragment::handle_initialization`, `Merc2::handle_setup_dma`). It makes debugging easy but means new game data shapes crash the renderer **(inference: in our project we control the producer so this is acceptable, but expect many such asserts during bring-up)**.

### 2.5 GIF decode loop (shape of the code you'd reuse)

`DirectRenderer::render_vif` (VIF stream with DIRECT commands) -> `render_gif(data, size)`: loops tags until EOP, then for each tag switches on `tag.flg()`:
* PACKED: if `tag.pre()` call `handle_prim(tag.prim())`; for `nloop x nreg`: dispatch on register descriptor (AD -> `handle_ad` which switches on the A+D address; ST, RGBAQ, XYZF2, XYZ2, PRIM, TEX0_1, UV), `offset += 16` each.
* REGLIST: 8-byte entries: PRIM, RGBAQ, XYZF2 only.
* IMAGE: only recognised for a BITBLTBUF/TRXPOS/TRXREG/TRXDIR sequence and skipped (asserts).
* Anything else: `ASSERT_MSG(false, "Register X is not supported in packed mode yet")`.
Size check at the end: `(offset + 15) / 16 == size / 16`.

---------------------------------------------------------------------------------------------------

## 3. Bucket renderers

### 3.1 How a frame is split into buckets (Jak 1)

* **Game side** (`goal_src/jak1/engine/dma/dma-bucket.gc`, `dma-buffer-add-buckets`, `dma-buffer-patch-buckets`, `dma-bucket-insert-tag`): at frame start the game allocates `BUCKET_COUNT` (70 on PC) 16-byte `dma-bucket`s at the front of the DMA buffer, each initialised as an empty `NEXT` tag pointing at the next bucket. Anything that wants to draw appends its own DMA sub-chain to a bucket (`dma-bucket-insert-tag`). At the end the chains are stitched so that the whole frame is **one linked list that visits buckets in id order**. Bucket order = draw order (sky, ocean, level-0 tfrag, ..., sprites, debug).
* **Preamble:** the very first tag is a `CALL` to a shared "default registers" chain (10 qw of A+D that reset GS state; `OpenGLRenderer::dispatch_buckets_jak1` asserts `default_regs_tag.qwc == 10` and copies the fog color from byte 144). Each bucket ends by CALLing that chain again, so every bucket starts from clean GS state.
* **Renderer side** (`OpenGLRenderer.cpp`): `m_bucket_renderers` is a `vector<unique_ptr<BucketRenderer>>` indexed by `BucketId` (`buckets.h`, 1089 lines, one enum per game: `jak1::BucketId` has 70 entries, e.g. `SKY_DRAW=3, OCEAN_MID_AND_FAR=4, TFRAG_TEX_LEVEL0=5, TFRAG_LEVEL0=6, ..., SPRITE=66, DEBUG=67, DEBUG_NO_ZBUF=68, SUBTITLE=69`). `init_bucket_renderers_jak1()` (l.653-960) builds them with `init_bucket_renderer<T>(name, BucketCategory, BucketId, args...)`; unset ones become `EmptyBucketRenderer`. `dispatch_buckets_jak1`:
  ```cpp
  m_render_state.buckets_base = dma.current_tag_offset() + 16; next_bucket = buckets_base;
  ... (consume the default-regs CALL/CNT/RET)
  for (bucket_id = 0; bucket_id < m_bucket_renderers.size(); bucket_id++) {
    renderer->render(dma, &m_render_state, bucket_prof);
    ASSERT(dma.current_tag_offset() == m_render_state.next_bucket);   // must end exactly at next bucket
    m_render_state.next_bucket += 16;
    vif_interrupt_callback(bucket_id);
    ...
  }
  ```
* **Interface** (`BucketRenderer.h/.cpp`, 162+141 lines): `virtual void render(DmaFollower&, SharedRenderState*, ScopedProfilerNode&) = 0; init_shaders(ShaderLibrary&); init_textures(TexturePool&, GameVersion); draw_debug_window()`. Helper classes in the same file: `RenderMux` (switch between implementations at run time), `EmptyBucketRenderer` (verifies the bucket really is empty: NEXT, CALL default regs, CNT, RET, NEXT), `SkipRenderer`, `PrintRenderer`.
* **`SharedRenderState`** (BucketRenderer.h): `ShaderLibrary`, `TexturePool`, `Loader`, `buckets_base/next_bucket/default_regs_buffer`, `ee_main_memory`, `fog_color`, camera data copied from PC_PORT packets (`camera_planes`, `camera_matrix`, `camera_hvdf_off`, `camera_fog`, `camera_pos`), per-level `occlusion_vis[32]` (2048-bit vis strings), FBO geometry (`render_fb`, `render_fb_w/h`, `draw_region_*`), `eye_renderer*`, `stencil_dirty`, `frame_idx`.
* Compilation: one translation unit per renderer, all listed in `game/CMakeLists.txt` `RUNTIME_SOURCE`.

### 3.2 Table of renderer classes (Jak 1 bucket in brackets)

Legend for "Input": **DMA-interp** = decodes the game's VIF/GIF/VU1 data at run time; **Offline** = draws pre-converted geometry from the `.fr3` level files loaded by `Loader`, the DMA only carries camera/state/names; **Mixed** = both. Line counts include the matching `.glsl` shader(s) where noted (`+sh`).

| Class (file) | Jak 1 bucket(s) | PS2 path it replaces | Input | Approx. lines |
|---|---|---|---|---|
| `SkyRenderer` (`SkyRenderer.*`) with embedded `DirectRenderer` | SKY_DRAW [3] | GIF-direct sky-dome strips | DMA-interp (GIF) | 267 |
| `SkyBlendHandler` + `SkyBlendGPU` / `SkyBlendCPU` (`SkyRenderer.*`, `SkyBlendGPU.*`, `SkyBlendCPU.*`) | TFRAG_TRANS0_AND_SKY_BLEND_LEVEL0/1 [32, 39] | EE/GS cloud-texture blending (sky textures are *generated* by blending two textures with time-of-day weights) | DMA-interp; result inserted into the texture pool | 243 + 288 (GPU = render-to-texture, CPU = software blend) |
| `OceanMidAndFar` (`ocean/OceanMidAndFar.*`) + `OceanTexture` + `OceanMid` + `OceanEnvmap` + `CommonOceanRenderer` | OCEAN_MID_AND_FAR [4] | VU1 ocean-texture, ocean-mid, ocean-far programs | DMA-interp; mid/texture run a **mechanical VU1->C++ port** (4.2) | mid 5,446; texture 1,384 (+sh); envmap 382; common 889 |
| `OceanNear` (`ocean/OceanNear.*`, `OceanNear_PS2.cpp`) | OCEAN_NEAR [63] | VU1 ocean-near program | DMA-interp, mechanical VU1->C++ | 4,587 |
| `TFragment` (`background/TFragment.*`, `tfrag3.vert/frag`) | TFRAG_LEVEL0/1 [6, 13], dirt [34, 41], ice [36, 43], trans/sky-blend [32, 39] | VU1 tfrag/tfrag-near programs + the DMA of tfrag geometry from level RAM | **Offline** (tfrag3 vertex/index buffers + BVH + per-vertex time-of-day colour tables); DMA only supplies `TfragPcPortData` (camera planes/matrices, itimes, level name) | 980 |
| `Tie3` / `Tie3WithEnvmapJak1` / `Tie3AnotherCategory` (`background/Tie3.*`, `etie*.vert/frag`, `tie_wind.*`) | TIE_LEVEL0/1 [9, 16] | VU1 tie programs (+ envmap variant) | **Offline**, with per-instance data & wind | 1,615 |
| `Shrub` (`background/Shrub.*`, `shrub.vert/frag`) | SHRUB_NORMAL_LEVEL0/1 [20, 26] | VU1 shrub program | **Offline** | 581 |
| `Hfrag` (`background/Hfrag.*`) | Jak 3 only | height-field terrain | Offline | 625 |
| `background_common.*` | shared | tfrag/tie draw-mode -> GL state, BVH culling, time-of-day interpolation, multi-draw list building | n/a | 938 |
| `VisDataHandler` (`VisDataHandler.*`) | Jak 2/3 (Jak 1 does it inside TFragment) | per-frame visibility bit strings copied out of the DMA | DMA (PC_PORT) | 119 |
| `Merc2` / `Merc2BucketRenderer` (`foreground/Merc2.*`, `merc2.vert/frag`, `emerc.*`) | MERC_* [10, 17, 45, 49, 52, 55, 58, 61] | VU1 merc programs (skinned characters, ~"mercneric") | **Offline** model data (`tfrag3::MercModel`) + per-draw PC_PORT packet (model name, 1 light set, bone matrix pointers); skinning on GPU | 1,983 |
| `Generic2` / `Generic2BucketRenderer` (`foreground/Generic2*.cpp`, `generic.vert/frag`) | GENERIC_* [11, 18, 24, 30, 46, 50, 53, 56, 59, 62] | VU1 "generic" program (the game's general-purpose geometry processor: shrubs near, water, effects, lightning, warp) | **DMA-interp**: parses fragment/adgif/vertex packets into GL vertex+index buffers | 2,007 |
| `ShadowRenderer` + `Shadow_PS2.cpp` (Jak 1) | SHADOW [47] | VU1 shadow program (shadow volumes) | DMA-interp; mechanical VU1->C++ run on CPU | 2,483 |
| `Shadow2` (`foreground/Shadow2.*`, `shadow2.*`) | Jak 2/3 | same, rewritten from understanding | DMA-interp (index/vertex tables), stencil volumes on GPU | 727 |
| `EyeRenderer` (`EyeRenderer.*`, `eye.*`) | MERC_EYES_AFTER_PRIS [54] | GS render-to-texture of eye sprites into texture pages that merc then samples | DMA-interp; render-to-texture, registers results in TexturePool | 749 |
| `DepthCue` (`DepthCue.*`, `depth_cue.*`) | DEPTH_CUE [64] | GS trick that re-samples the framebuffer as a texture in 16 slices (draw to a "base page", then back to the screen) for a depth-cue/fog-like effect | DMA-interp (GS setup structs asserted), reads the FBO | 907 |
| `Sprite3` (`sprite/Sprite3.*`, `sprite_common.h`, `Sprite3_Distort.cpp`, `Sprite3_Glow.cpp`, `sprite_3d.*`, `sprite3_3d.*`, `sprite_distort*.*`) | SPRITE [66] | VU1 sprite programs (3D billboards, 2D world sprites, HUD sprites, distorters, glow) | DMA-interp; **VU1 math ported to a vertex shader** (4.1) | 2,792 |
| `GlowRenderer` (`sprite/GlowRenderer.*`, `glow_*.glsl`) | inside SPRITE | sprite-glow VU1 program + GS depth probing for lens-flare occlusion | DMA-interp; "glow_math" C++ + several shader passes | 1,214 |
| `DirectRenderer` (`DirectRenderer.*`) | DEBUG [67], DEBUG_NO_ZBUF [68], SUBTITLE [69] (custom), and sub-component of Sky, Sprite3, TextureUploadHandler, OceanMidAndFar | PATH2/DIRECT GIF packets (fonts, HUD, debug text/lines, sky strips) | DMA-interp (GIF) | 1,815 (+ direct_basic*.glsl) |
| `DirectRenderer2` (`DirectRenderer2.*`, `direct2.*`) | **not instantiated anywhere in this checkout** (compiled, `ShaderId::DIRECT2` created, nothing uses it) | batched variant of the above | DMA-interp (GIF) | 971 (+shader) |
| `ProgressRenderer` (`ProgressRenderer.*`) | Jak 2 progress menu | DirectRenderer subclass that follows FRAME_1 switches to draw a mini-map off-screen | DMA-interp | 93 |
| `TextureUploadHandler` (`TextureUploadHandler.*`) | every `*_TEX_*` bucket [5, 12, 19, 25, 31, 38, 48, 51, 57, 60, 65] | the per-level texture DMA uploads to GS VRAM | DMA (PC_PORT packets only; the GS image uploads themselves are dropped) | 157 |
| `TextureAnimator` (`TextureAnimator.*`, `TextureAnimatorDefs.cpp`, `tex_anim.*`) | Jak 2/3/X only | texture effects the game did on the EE/GS by writing into VRAM (fire, water, clut blends, scrolling) | DMA-interp + fake VRAM + GL render-to-texture | 5,849 |
| `BlitDisplays` + `SlowTimeEffect` (`BlitDisplays.*`, `slow_time.*`) | Jak 2/3 | frame-to-texture copies, zoom blur, colour filter, slow-time | DMA (PC_PORT cmds 0x10-0x15) + FBO copies | 355 |
| `Warp` (`Warp.*`) | Jak 2/3 | frame copy then Generic2 in WARP mode | DMA | 46 |
| `CollideMeshRenderer` (`CollideMeshRenderer.*`, `collision.*`) | debug overlay drawn mid-frame | none (debug visualisation of extracted collision mesh) | Offline | 507 |
| `OpenGLRenderer` (`OpenGLRenderer.*`) | the bucket loop, FBOs, PCRTC emulation | the whole GS output stage | n/a | 1,836 |
| infrastructure | | | | `Shader.*` 226, `opengl_utils.*` (FBO helpers: `FramebufferTexturePair`, `FramebufferCopier`, `FullScreenDraw`) 436, `Profiler.*` + `debug_gui.*` 751, `loader/*` 1,597, `texture/TexturePool.*` etc. 1,057, `*_tpage_dir.*` 2,117 (static tables of texture-page layouts) |

Totals: `game/graphics/opengl_renderer/**` cpp+h = **45,186 lines**, of which ~19,000 are the three mechanical VU ports (`OceanMid_PS2` 4,991, `OceanNear_PS2` 4,232, `Shadow_PS2` 1,894) plus `TextureAnimator*` (5.8k) and `buckets.h` (1,089). Shaders: 92 files, 3,112 lines. Plus `game/graphics/pipelines/opengl.*` 942, `gfx.*` 371, `display.*`, `sceGraphicsInterface.*` ~200.

### 3.3 The two data-flow strategies, side by side

1. **Interpret the game's own DMA at run time** (sprite, generic, sky, ocean, shadow, eyes, depth-cue, direct). Requires mirroring the game's data layout in C++ structs (`SpriteFrameData`, `Generic2::Fragment`, `OceanMid::Constants`, `DepthCueGsSetup` ... each with `static_assert(sizeof == N)`) and reproducing the VU1 math.
2. **Skip the PS2 geometry path and draw offline-converted data** (tfrag, tie, shrub, merc, collision, textures). The game still runs its *CPU-side* work (visibility, LOD, time-of-day colours, skeleton animation) and emits a tiny packet with the results; the renderer fetches meshes by **level name / model name** from the loader. The VU1 programs are not run, ported or even sent. Example `TFragment::handle_initialization`: the original tfrag DMA (matrices, `mscal` setup) is read and mostly discarded; the useful part is the `TfragPcPortData` packet (`background_common.h`): `GoalBackgroundCameraData{planes[4], itimes[4], camera[4], hvdf_off, fog, trans, rot[4], perspective[4]} + char level_name[32]`, built by `add-pc-tfrag3-data` in `goal_src/jak1/engine/gfx/tfrag/tfrag.gc` (25 qw).
   For merc, `pc-merc-draw-request` (`goal_src/jak1/engine/gfx/foreground/bones.gc` ~l.925) writes: model name (128 bytes), 1 light set (7 qw), a water flag, a "matrix slot string" (which of 128 bone slots are used), pointers (not copies!) to the bone matrices, per-effect enable/alpha-ignore masks and fades.

Strategy 2 is the one that gave them GPU-friendly meshes (multi-draw/indexed, time-of-day via a texture lookup in the vertex shader, BVH culling on the CPU) and made big perf wins, at the cost of an entire offline extractor.

---------------------------------------------------------------------------------------------------

## 4. Replacing VU1 microprograms

Three distinct methods appear. For each: how inputs are read, how the math was ported, where results go.

### 4.1 Method A: port the VU program into a GL vertex shader, keep the VU data layout as a CPU struct (Sprite3, Generic2, Merc2)

**Sprite3** (`sprite/Sprite3.{h,cpp}` 213+905, `sprite_common.h` 245, `shaders/sprite3_3d.vert` 177 lines).
* The header documents the VU1 memory map as C++ enums so the DMA parser can place data where the VU program would have found it:
  ```cpp
  enum SpriteDataMem { Header = 0, Vector = 1, Adgif = 145, Buffer0 = 0, Buffer1 = 400,
                       GiftagBuilding = 800, Matrix = 900, FrameData = 980 };
  enum SpriteProgMem { Init = 0, Sprites2dGrp0 = 3, Sprites2dHud_Jak1 = 109, Sprites3d = 211 };
  ```
  Programs are identified by the **MSCAL/MSCALF entry address** in the DMA stream; the microcode itself is never uploaded or executed.
* Input reading: `handle_sprite_frame_setup` reads the per-frame constants packet and `memcpy`s it into `SpriteFrameData` after asserting the exact VIF shape (`STCYCL 4,4`, `UNPACK_V4_32` to qw 980, no TOPS flag). Then per chunk of up to 48 sprites (`SPRITES_PER_CHUNK`): header packet, vector data (`SpriteVecData2d`: position+sx, flag/rot/sy, rgba), adgif data (`AdGifData` per sprite), then the `MSCAL` that would have run VU1 (`unpack_to_no_stcycl(&m_vec_data_2d, vec_data, UNPACK_V4_32, size, SpriteDataMem::Vector, false, true)`).
* What the CPU does per sprite (`do_block_common`): cull 2D sprites against `render_state->camera_planes`; decode the adgif (`handle_tex0/tex1/zbuf/clamp/alpha` -> a `DrawMode` plus a TBP); batch into `Bucket{ids}` keyed by `(tbp << 32) | draw_mode`; emit **4 identical `SpriteVertex3D` (64 B) per sprite**, with a `vert_id` (0-3) stored in `info[2]` and the render mode (2D/HUD/3D) in `info[3]`. Index buffer: triangle strip of 4 with `UINT32_MAX` primitive restart. The GS-side effects of the VU program (building the GS packet, XGKICK) disappear.
* The math is in the vertex shader. Excerpt (variable names keep the VU register names; in `sprite3_3d.vert`):
  ```glsl
  // STEP 2: perspective transform for distance
  vec4 transformed_pos_vf02 = matrix_transform(rendermode == 2 ? hud_matrix : camera, position);
  float Q = pfog0 / transformed_pos_vf02.w;
  // STEP 3: fade out sprite!
  ... fragment_color.w *= min(scales_vf01.x, 1.0);
  // STEP 4: 3D: sprite_quat_to_rot(quat) ; 2D/HUD: xy_array[vert_id + flags] rotated by basis_x/basis_y * sin/cos ...
  // STEP 5: final adjustments (VU output space -> OpenGL clip space)
  transformed.xy -= 2048.;  transformed.z /= 8388608;  transformed.z -= 1;
  transformed.x /= 256;     transformed.y /= -128;     transformed.xyz *= transformed.w;
  transformed.y *= SCISSOR_ADJUST * HEIGHT_SCALE;
  ```
  The constants (`xy_array[8]`, `xyz_array[4]`, `st_array[4]`, `basis_x/y`, `hvdf_offset`, `pfog0`, ...) are uploaded as uniforms from the `SpriteFrameData` copy.
* **The coordinate convention to remember** (identical in every shader they ported): VU1 outputs GS-space floats (x,y in pixels with 2048 offset, y down, z as a 24-bit depth value up to 2^24, stored in the "w" lane after the perspective divide trick). Step 5 maps them to NDC. Depth uses `glClearDepth(0.0)` + `GL_GEQUAL` (see 5.1), i.e. larger z = closer, same as the GS.

**Generic2** (the biggest VU1 program in the game, the "generic" renderer). Files `foreground/Generic2.{h,cpp}`, `Generic2_DMA.cpp` (728), `Generic2_Build.cpp` (387), `Generic2_OpenGL.cpp` (341), `shaders/generic.vert/frag` (101+82).
* `Generic2_DMA.cpp` consumes the VU1 upload stream: bucket setup (`test/zbuf` GS packet, then a 160-byte constants unpack: `pfog0, fog_min, fog_max`, giftags, `hvdf_offset`, `hmge_scale`, ...), then repeated "fragment" packets (a 7-qw header + adgifs + vertices) into flat arrays (`m_fragments`, `m_adgifs`, `m_verts`), up to 500k vertices / 10k fragments / 10k adgifs per bucket. The comment in `handle_bucket_setup_dma`: "setup packet 2 is constants that normally go to VU1 data memory. we're not going to be super strict checking the exact details of the unpack command, it's a waste of time since we're the ones generating it anyway."
* `Generic2_Build.cpp`: `setup_draws` = `link_adgifs_back_to_frags`, `process_matrices`, `determine_draw_modes` (re-derives the GS state each adgif sets, **including state left over from earlier adgifs**; the comment says the game "do[es] a bunch of tricks where some of the GS state is left over from the previous draw"), `draws_to_buckets` (group by `(DrawMode, tbp, fix, hud)`), `build_index_buffer`.
* `generic.vert` is a line-by-line port of the VU1 code, with the original asm kept as comments next to each GLSL line:
  ```glsl
  // lq.xy vf22, 0(vi10)          texture load?
  // mulaw.xyzw ACC, vf11, vf00   matrix multiply W
  // maddax.xyzw ACC, vf08, vf16  matrix multiply X
  // div Q, vf01.x, vf12.w        perspective divide
  float Q = fog_constants.x / transformed.w;
  // itof12.xyz vf18, vf22        texture int to float
  tex_coord = tex_coord_in / 4096.f;
  ```
  Vertex: `xyz(float3), rgba(u8x4), st(float2), tex_unit/flags/adc (u8)` = 32 bytes.
* VU-specific bits that needed hacks: ADC bit (vertex kick-without-draw) is carried per vertex; the **fan/strip decomposition** is done on the CPU when building the index buffer; "warp" mode samples with a special region clamp (`warp_sample_mode`).

**Merc2** (`foreground/Merc2.{h,cpp}` 265+1,360, `merc2.vert` 103, `emerc.*`).
* Offline model data: `tfrag3::MercModel` (extracted from the game's merc-ctrl; vertex = position, normal, 3 skinning weights, st, rgba, 3 bone indices), per-effect `MercDraw`s with a `DrawMode` and texture, uploaded once per level by the loader.
* Runtime DMA: `handle_setup_dma` reads the frame constants (10 qw: low-memory unpack of `hvdf_offset`, 4x4 perspective, fog) asserting `BASE=442`, `OFFSET=-442`: the comment in `Merc2.h`: *"this negative offset is what broke jak graphics in Dobiestation for a long time"* (i.e. real VIF BASE/OFFSET semantics matter for emulators; the port just ignores them). `handle_merc_chain` loops over `PC_PORT` packets, one per model instance, and `handle_pc_model` resolves the model by name, copies the bone matrices into a uniform-buffer ring (`alloc_bones`), lights (`VuLights`), then queues `Draw`s per level bucket. `flush_draw_buckets` issues multi-draws.
* The shader (`merc2.vert`) is GPU skinning with a `std140` UBO `ub_bones { MercMatrixData bones[128]; }` and keeps the VU1 asm in a comment block:
  ```
  mula.xyzw ACC, vf15, vf08 / maddz.xyzw vf09, vf16, vf08 ...
  ```
  `vtx_pos = -bones[mats[0]].X * p * weights_in[0] + ...(up to 3 weights)`; per-vertex lighting `light_ambient + sum(max(dot-ish,0) * light_colN)`; fog `255 - clamp(-w + hvdf.w, fog_min, fog_max)`; same Step-5 NDC mapping.
* Models whose geometry the game modifies per frame ("mod vtx", blend shapes, `blerc`) are recomputed on CPU into dynamic vertex buffers (`model_mod_draws`, AVX `blerc_avx`).

### 4.2 Method B: mechanical, instruction-by-instruction VU1 -> C++ ("vu2c"), executed on the CPU (Ocean mid/near/texture, Shadow v1)

Files: `ocean/OceanMid_PS2.cpp` (4,991), `ocean/OceanNear_PS2.cpp` (4,232), `ocean/OceanTexture_PC.cpp` (734), `Shadow_PS2.cpp` (1,894), the VU register file in **`game/common/vu.h` (610 lines)**.
* `vu.h` defines `Vf` (16-byte aligned `float[4]`, `x()/y()/z()/w()`, `x_as_u32()`, `x_as_u16()` for integer-in-float tricks, per-op methods such as `mr32`, `mfir`, `maxi`), `Mask` (xyzw field masks), `Accumulator` (the VU ACC register with the madd/msub family), and `vu_max/vu_min` (VU float compare quirks). It uses SSE intrinsics through `common/util/simd_util.h`, which on ARM includes `third-party/sse2neon/sse2neon.h`.
* Each renderer class owns `Vf m_vu_data[1024]` (VU1 data memory, 16 KB) and declares its own register file (`OceanMid.h:182`): `struct Vu { const Vf vf00(0,0,0,1); Accumulator acc; Vf vf01..vf31; u16 vi01..vi15; float Q, P; } vu;` plus `lq_buffer/sq_buffer/ilw_buffer` helpers that read/write `m_vu_data` through a `Mask`. DMA `UNPACK`s are `memcpy`d into `m_vu_data[addr (+ TOPS buffer offset)]` exactly like the VIF would (`OceanMid::run`: `u16 addr = up.addr_qw + (up.use_tops_flag ? get_upload_buffer() : 0)`; toggles `m_buffer_toggle` on each `xtop()` to mimic double buffering).
* Each `MSCAL`/`MSCALF` immediate (`41, 43, 46, 73, 107, 275`) selects a generated function, e.g. `run_call73_vu2c()`. Each VU1 instruction pair becomes lines like (from `OceanMid_PS2.cpp`):
  ```cpp
  // lq.xyzw vf01, 733(vi00)    |  nop                            0
  lq_buffer(Mask::xyzw, vu.vf01, 733);
  ```
  with `clip`, `fcand`, `fcor` (clip flag registers), `erleng` (rsqrt), branches as `goto`, `bc` flags, etc. in the same file.
* `xgkick(addr)` is the output: it passes the GIF packet that the VU built *inside* `m_vu_data` to `CommonOceanRenderer::kick_from_mid((const u8*)&m_vu_data[addr])`, which decodes the GIF tags into vertex/index buffers (`CommonOceanRenderer`, 539 lines) and later draws them with `shaders/ocean_common.*`. I.e. **GIF output of a VU program -> same tag/register decoder family as the direct path**.
* **(inference)** The translator that produced `*_PS2.cpp` is not in this repo (the only related code is `decompiler/VuDisasm/` (disassembler) and `decompiler/analysis/mips2c.cpp` (EE MIPS -> C++) exist). The files' per-line comments match the disassembler's dual-issue format, so the pipeline was probably disassemble -> script -> hand fix. Reference disassemblies for all Jak VU programs are in `test/decompiler/vu_reference/{jak1,jak2,jak3,jakx}/*.txt` (raw `.word` dumps) and `*-result.txt` (disassembly), 43,831 lines for Jak 1, used as regression tests in `test/decompiler/test_VuDisasm.cpp`. `docs/progress-notes/jak2/{generic,emerc,etie,sprite_glow,blerc_asm}.md` and `docs/progress-notes/jak1/scratch/sprite_*.txt` contain the humans' annotated understanding of several VU programs and are a **good model for what documentation to produce while porting**.
* The ocean results later got faster paths: `OceanTexture_PC.cpp` is a hand-simplified rewrite of the ocean-texture program that renders to a texture with a GPU pass (`ocean_texture.*`), and `OceanMid.cpp` still runs the mechanical port. So the mechanical port is the "get it correct first" tool and the semantic rewrite is the "make it fast" tool.

### 4.3 Method C: semantic rewrite once the VU program is understood (Shadow2, sky blend GPU, OceanTexture_PC, TIE/TFRAG/merc offline)

`Shadow_PS2.cpp` (mechanical, 1,894) was followed by `Shadow2` (585 lines) which doesn't emulate VU1 at all. It reads the shadow data tables from the DMA (`kTopVertexDataAddr = 4, kBottomVertexDataAddr = 174, kCapIndexDataAddr = 344, kWallIndexDataAddr = 600` = VU1 data-memory offsets used by the program; `buffer_from_mscal2/4/6` correspond to MSCAL entry points), builds explicit front/back-face index buffers (`add_cap_tris`, `add_wall_quads`, `add_flippable_tris`) and draws the stencil volume with `shaders/shadow2.*` (two index buffers, `clear_mode` uniform). For comparison the VU1 sprite and generic programs were ported via method A. Tfrag/tie/shrub/merc skip the VU programs completely (strategy 2 in 3.3).

### 4.4 DirectRenderer in detail (the path our 2D menu/HUD/font code will resemble)

`DirectRenderer.{h,cpp}` 332 + 1,483 lines; shaders `direct_basic.*`, `direct_basic_textured.{vert,frag}`, `direct_basic_textured_multi_unit.*`, `debug_red.*`.

**Input entry points:** `render()` (as a bucket: loops DMA transfers until `next_bucket`, calls `render_vif(vif0, vif1, data, size)` for each, skipping the default-register CALL), `render_vif()` (parses a VIF stream with `NOP`/`FLUSHA`/`DIRECT n` and recurses into `render_gif` for each `DIRECT`'s `n*16` bytes), `render_gif()` (GIF tag loop, see 2.5), plus `flush_pending()` for use as a sub-component. Constructor argument is a batch size in triangles (`0x20000` for the debug bucket).

**Architecture:** the object is a **software model of the GS drawing registers + a primitive assembler**:
* State structs mirror GS registers: `TestState` (from `GsTest`), `BlendState` (from `GsAlpha`), `PrimGlState` (from `GsPrim`), `TextureState` (TEX0 + CLAMP + TEX1.mmag), `m_scissor` (static, shared across buckets), `PrimBuildState` (current RGBAQ, ST, Q, `building_idx`, up to 3 pending vertices, tri-strip/fan start-up counters).
* **Deferred GL state:** register writes only mark `m_*_needs_gl_update`. When a register write changes something that matters for GL (`handle_test1`, `handle_alpha1`, `handle_prim`, `handle_zbuf1`, `handle_texa`, `handle_tex0_1` ...), it first calls `flush_pending()` (draw what is buffered under the old state) and *then* records the new state. That is the entire batching strategy; counters `flush_from_tex_0/…/state_exhaust` are exposed in the debug window.
* **Vertex kick = `handle_xyzf2_common(x, y, z, f, ..., advance)`** (`advance` = !ADC). It snapshots RGBA, ST/Q, XYZ+fog into `building_*[idx]`, then by primitive kind appends triangles to `m_prim_buffer` (vector of 64-byte `Vertex{xyzf(float4), stq(float3), rgba(u8x4), tex_unit, tcc, decal, fog_enable, use_uv, scissor(float4)}`):
  * `TRI`: every 3rd vertex; `TRI_STRIP`: after 3 start-up vertices emit a triangle on every kick with the 3-vertex window (the `tri_strip_startup` counter; ADC=1 kicks update the window without emitting); `TRI_FAN`: same with the fan pivot; `SPRITE`: 2 corners -> **2 triangles** (corner3/4 synthesised from the other corners' x/y with the *last* vertex's z, colour of the second vertex; Gouraud sprites assert); `LINE` and `LINE_STRIP`: expanded to quads of width `1<<19` in GS fixed point using a z-axis cross product; `POINT` unsupported.
  * Everything becomes `GL_TRIANGLES` in one dynamic VBO (`glBufferData(..., GL_STREAM_DRAW)` each flush).
* **Vertex units:** vertices are stored as raw GS 12.4 fixed-point-scaled integers converted to float in `PrimitiveBuffer::push`: `v.xyzf[0] = (float)vert[0] / (float)UINT32_MAX` where the caller shifted the 16-bit GS x left by 16, similarly z `/ 0xffffff`; the shader (`direct_basic_textured.vert`) maps: `gl_Position = vec4((x - 0.5) * 16., -(y - 0.5) * 32 * HEIGHT_SCALE, z * 2 - 1., 1.0); gl_Position.y *= SCISSOR_ADJUST;`. (The `0.5` and the factors 16 and 32 are the GS 4096-pixel primitive space vs the 512 pixel screen.) Offscreen mode (`offscreen_mode == 1`) uses a different mapping for render-to-texture targets (the mini-map).
* **Texturing:** `handle_tex0_1` stores `texture_base_ptr = reg.tbp0()`, `tcc`, `decal` (TFX decal vs modulate; anything else asserts) and `using_mt4hh` (PSMT4HH). At draw time `update_gl_texture` looks the TBP up in `TexturePool::lookup(tbp)` (or `lookup_mt4hh`) and binds it to texture unit 20; missing = placeholder checkerboard. CLAMP bits 0 and 2 -> `GL_CLAMP_TO_EDGE` vs `GL_REPEAT`; `TEX1.mmag` -> linear vs nearest. The CLUT is *ignored* ("the only thing the direct renderer does with texture is font, which does no tricks with CLUT. The texture upload process will do all of the lookups with the default CLUT.").
* **Fragment shader** (`direct_basic_textured.frag`) emulates the GS combiner and tests:
  * `sample_tex(tex_coord.xy / tex_coord.z)` for ST/Q (perspective division per pixel as the GS does), or `sample_tex_px` for UV (FST=1: pixel coordinates `/16`, divided by texture size);
  * `tex_info.y` = TCC, `.z` = decal, `.w` = fog flag: modulate (`rgb = vertex * tex`), decal (`rgb = tex * 0.5`), TCC picks texture alpha or vertex alpha; **colours are in the PS2's 0x80 = 1.0 scale, so `color *= 2` at the end** (`fragment_color.w * 2.` in the vertex shader for alpha);
  * alpha test as uniforms `alpha_min`/`alpha_max` (`GEQUAL`: `alpha_min = aref/128`; `GREATER`: `(1+aref)/128`; the `greater` flag chooses `<=` vs `<` discard); `ta0` substitutes alpha when the texel alpha is 0;
  * scissor emulation: per-vertex `gs_scissor` (x0,x1,y0,y1) compared with `gl_FragCoord` scaled by `game_sizes` (512 x 448 virtual screen vs the real viewport);
  * fog: `mix(color.rgb, fog_color.rgb, clamp(fog_color.a * fog, 0, 1))` with `fog = 255 - xyzf.w`, `fog_color` from the default-regs buffer.
* **Blend** (`update_gl_blend`): the ALPHA register's (A,B,C,D,FIX) are matched against 6 known combos and mapped to `glBlendFuncSeparate`/`glBlendEquation` (the same table appears in `DrawMode::AlphaBlend`), e.g. `(Cs-Cd)*As+Cd` -> `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`; `(Cs-0)*As+Cd` -> `GL_SRC_ALPHA, GL_ONE`; `(0-Cs)*As+Cd` -> `GL_FUNC_REVERSE_SUBTRACT`; `(Cs-Cd)*FIX+Cd` -> `GL_CONSTANT_ALPHA` with `glBlendColor(0,0,0,fix/127)`; `(Cs-0)*Ad+Cd` -> `GL_DST_ALPHA, GL_ONE` with `color_mult = 0.5`; A=B=C=D=Cs is used as "blending off". Anything else logs "unsupported blend". PABE must be 0, DATE must be off (asserts).
* **Depth** (`update_gl_test`): ZTE must be on (assert: "you aren't supposed to turn off z test enable, the GS had some bugs"); ZTST -> `glDepthFunc` NEVER/ALWAYS/GEQUAL/GREATER; ZMSK from ZBUF -> `glDepthMask`; `FRAME.fbmsk` -> `glColorMaski`.
* **Alpha-fail modes** (`AFAIL=FB_ONLY/RGB_ONLY`, used with alpha test by sky/sprites): emulated by the **"double draw"**: draw once with `alpha >= aref` (colour + depth) and once with `alpha < aref` (colour only, `glDepthMask(GL_FALSE)`), by flipping `alpha_min/alpha_max` uniforms and issuing two `glDrawArrays`. ATEST=NEVER + AFAIL=FB_ONLY is recognised as "colour-only, no depth write" (`alpha_trick_to_disable`).
* **FRAME/XYOFFSET:** `handle_frame` is a virtual no-op (frame buffer is fixed); `ProgressRenderer` and `OceanEnvmap` override it / watch FRAME_1 to redirect to an off-screen FBO. `handle_xyoffset` (Jak 2+ only) shifts vertices by `(ofx - 0x7000) / ...`. Hard-coded: Jak 1 `ZBUF.zbp == 448`, `PSMZ24`.
* **Not supported (assert):** AA1, CTXT=1, FIX prim bit, DATE, texture transfers (BITBLTBUF/TRXDIR sequences assert), REGLIST registers other than PRIM/RGBAQ/XYZF2, point primitives.
* Memory: each DirectRenderer instance owns a CPU vertex array of `batch_size * 3` vertices (64 B each) and a GL buffer sized `batch_size * 3 * 2` vertices (`m_ogl.vertex_buffer_max_verts`); for the "debug" bucket (`batch_size = 0x20000`) that is about 25 MB of CPU array plus about 50 MB of GL buffer. They size generously because it only has to fit one frame of one bucket.

### 4.5 DirectRenderer2 (dead code in this checkout, but it documents the better batching design)

`DirectRenderer2.{h,cpp}` 150 + 821 lines + `direct2.vert/frag` (48+75). **Not instantiated anywhere** (`grep` finds only `Shader.cpp` creating the shader and `game/CMakeLists.txt`). **(inference)** `Generic2` copied its approach, so reading `DirectRenderer2` is the cheap way to understand `Generic2_OpenGL.cpp`.
* Same register handlers but with `DrawMode` (32-bit packed state) instead of separate state structs; **tri-strip only** (`ASSERT(reg.kind() == GsPrim::Kind::TRI_STRIP)`, `gouraud`, `tme`, no FST/CTXT/AA).
* Vertices: `Vertex{xyz(float3), rgba(u8x4), stq(float3), tex_unit, flags(tcc|decal|fog), fog, pad}` = 32 bytes. Indices: strips with `UINT32_MAX` as primitive restart (`glEnable(GL_PRIMITIVE_RESTART)`); a "start strip" marker is pushed when `next_vertex_starts_strip`, and an ADC=0 vertex pushes `[restart, v-1, v]` to keep the strip glued.
* **Draws are state records**: `Draw{DrawMode mode; start_index; tbp; fix; tex_unit}` opened only when state changed (`m_current_state_has_open_draw` flag invalidated by every state-changing register write). At flush: upload vertex+index buffers once; `draw_call_loop_grouped` walks draws and **merges up to `TEX_UNITS = 10` consecutive draws that differ only in texture** by giving each a distinct texture unit (`tex_unit = (prev + 1) % 10`; vertices carry `tex_unit`; the fragment shader `switch`es over `tex_T0..tex_T9`), then one `glDrawElements(GL_TRIANGLE_STRIP)` for the group. The stats `saved_draws` shows the gain.
* Coordinates: raw GS integers go to the GPU unconverted: `position_in.x - 0x8000) / 0x1000`, `y: -(y - 0x8000)/0x800`, `z: z / 0x800000 - 1` (vertex shader does the fixed-point decode).
* `use_ftoi_mod` variant (`handle_xyzf2_mod_packed`) accepts float x,y,z (`x*16`) instead of integer GS coordinates, for sources that never did the ftoi4 conversion (VU-generated packets).

---------------------------------------------------------------------------------------------------

## 5. GS state translation and textures

### 5.1 GS register -> GL state map (as implemented)

| GS register / bit | Where handled | GL / shader result |
|---|---|---|
| `PRIM.PRIM` kind (tri/strip/fan/sprite/line/linestrip) | `DirectRenderer::handle_xyzf2_common` (CPU primitive assembly), Sprite3 (4 verts + restart), Generic2/Direct2 (strips w/ restart) | everything drawn as triangle lists or strips; no `GL_TRIANGLE_FAN`/`GL_LINES` |
| `PRIM.TME` | `PrimGlState.texture_enable` | picks `DIRECT_BASIC_TEXTURED` vs `DIRECT_BASIC` program |
| `PRIM.FGE` | per-vertex `fog_enable` byte | fragment `mix(color, fog_color, ...)` |
| `PRIM.ABE` | `m_blend_state.alpha_blend_enable` | `glEnable(GL_BLEND)` |
| `PRIM.FST` | `use_uv` per vertex | UV (pixels/16) vs ST/Q sampling branch in shader |
| `PRIM.IIP` | always Gouraud (asserts for flat sprites) | varying colour |
| `PRIM.AA1/CTXT/FIX` | assert not used | n/a |
| `RGBAQ`, `ST`, `UV`, `XYZF2/XYZ2`, `FOG` | vertex assembler | VBO attributes (rgba u8, stq float3, xyzf) |
| `TEX0`: TBP0, PSM, TCC, TFX | `handle_tex0_1` / `AdgifHelper` | texture = `TexturePool::lookup(tbp)`; TCC/decal flags per vertex; PSM only distinguishes PSMT4HH (-> `lookup_mt4hh`); TW/TH/CBP/CPSM/CSM ignored ("assume they got it right") because textures are pre-converted RGBA8 |
| `TEX1`: MMAG (and MMIN) | `handle_tex1_1` | `GL_LINEAR` vs `GL_NEAREST`; mipmaps only for background (`mipmap` flag in `setup_opengl_from_draw_mode`) |
| `CLAMP` bit0/bit2 (WMS/WMT=CLAMP) | `handle_clamp1` | `GL_CLAMP_TO_EDGE` vs `GL_REPEAT`; region clamp unsupported (warp special-cased) |
| `ALPHA` (A,B,C,D,FIX) | `update_gl_blend`, `DrawMode::AlphaBlend` | blend presets above; unknown combos logged |
| `TEST`: ATE/ATST/AREF/AFAIL | `update_gl_prim/test`, `setup_opengl_from_draw_mode` | `alpha_min`/`alpha_max` uniforms (`aref/128` or `/127`), discard in shader; AFAIL FB_ONLY/RGB_ONLY -> two-pass "double draw" (`DoubleDrawKind::AFAIL_NO_DEPTH_WRITE`); ATST NEVER+FB_ONLY -> depth write off |
| `TEST`: ZTE/ZTST | `update_gl_test` | `glEnable(GL_DEPTH_TEST)`; `GEQUAL`/`GREATER`/`ALWAYS`/`NEVER` direct; **depth buffer cleared to 0.0 and compared with GEQUAL** (`glClearDepth(0.0)` in `setup_frame` and `BlitDisplays`) |
| `TEST`: DATE/DATM | assert off | n/a |
| `ZBUF`: ZMSK | `handle_zbuf1` | `glDepthMask`; ZBP/PSM only asserted (448 for Jak1, 304 Jak2) |
| `FRAME`: FBP/FBMSK | `handle_frame` (virtual) | default ignored; `ProgressRenderer`, `OceanEnvmap`, `EyeRenderer` use FBP to switch FBO; FBMSK -> `glColorMaski` |
| `SCISSOR` | `handle_scissor` | per-vertex vec4 + `gl_FragCoord` test in `direct_basic_textured.frag` (`scissor_enable`) |
| `XYOFFSET` | `handle_xyoffset` (Jak 2+) | CPU shift of vertices |
| `FOGCOL` | read from default regs buffer at byte 144 (`fog_color`) | `fog_color` uniform; `A+D FOGCOL` writes are skipped ("TODO") |
| `TEXA` | `handle_texa` | `ta0` uniform: texels with alpha 0 get alpha `ta0` (RGBA16 handling); `ta1` must be 0x80, `aem` false |
| `COLCLAMP` | assert == 1 | n/a |
| `MIPTBP1/2`, `TEXFLUSH`, `TEXCLUT` | ignored | n/a |
| `BITBLTBUF/TRXPOS/TRXREG/TRXDIR` (GS local transfers) | `DirectRenderer` asserts (or logs "GS TEXTURE COPY") | not supported in the direct path; handled by game-side `texture_relocate` / PC_PORT commands instead |
| `PMODE.ALP`, PCRTC | `set_pmode_alp` + `do_pcrtc_effects` | blackout alpha blended over the final image |

Shader-side conventions shared across all shaders: colours doubled at the end (0x80 = 1.0), alpha doubled in the vertex shader, `HEIGHT_SCALE`/`SCISSOR_ADJUST`/`SCISSOR_HEIGHT` placeholder tokens replaced by `std::regex_replace` in `Shader.cpp` (448 vs 416 line games) before compile; samplers must be named `tex_T<n>` and are bound to unit n by `Shader` automatically; `ub_bones` UBO bound to binding point 1; shader sources are **read from disk at run time** from `game/graphics/opengl_renderer/shaders/` (not embedded).

### 5.2 Textures: how PS2 VRAM becomes GL textures

**There is no VRAM emulation in the Jak 1 draw path.** The pipeline:

1. **Offline** (`decompiler/data/tpage.cpp` using `common/texture/texture_conversion.h` addressing): each game texture page's PSMT8/PSMT4/PSMCT16/PSMCT32 + CLUT data is de-swizzled (`psmct32_addr`, `psmt8_addr`...) and converted to RGBA8888, stored in the level `.fr3` files (`tfrag3::Level::textures`, `index_textures`) and in the "common" file. Each texture has a stable `PcTextureId{page, tex}` from a static tpage directory (`game/graphics/texture/jak1_tpage_dir.cpp`, 103 lines; jak2 274; jak3 1,720).
2. **Load time** (`Loader`, `LoaderStages`, 1,597 lines): a background thread reads `.fr3` (`file_util::read_binary_file` + `compression::decompress_zstd`) and deserialises; the render thread then uploads textures/meshes in time-budgeted stages (`TIE_LOAD_BUDGET = 1.5` ms, `SHARED_TEXTURE_LOAD_BUDGET = 3` ms per frame; GL objects freed through `m_garbage_*` lists). `upload_to_gpu` (TexturePool.cpp): `glTexImage2D(GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV)` + `glGenerateMipmap` + max anisotropy + linear filtering.
3. **Run time** (`game/graphics/texture/TexturePool.{h,cpp}`, 379+428 lines): `std::array<TextureVRAMReference, 1024*1024*4/256 = 16384> m_textures` indexed by **TBP** (VRAM word address / 64 = 256-byte block). A `TextureVRAMReference{GLuint gpu_texture; GpuTexture* source}`. `lookup(tbp)` is a single array read, lock-free. Three layers: VRAM slot -> `GpuTexture` (one per `PcTextureId`, holds every loaded copy because the same texture can come from two levels) -> `TextureData{gl, data}`.
   * The game "uploading" a texture page triggers `__pc-texture-upload-now` (GOAL `texture.gc:1476`) -> `TexturePool::handle_upload_now(tpage, mode, memory_base, s7)` which just parses the `GoalTexturePage`/`GoalTexture` headers and, for each mip, sets `m_textures[tex.dest[mip]]` to point at the pre-converted GpuTexture of that id (or a placeholder if the loader hasn't delivered it). The GS image DMA that would copy pixels is **dropped**. `mode` -1/0/2/-2 selects which of the 3 segments (mip groups) of the page to register.
   * VRAM-to-VRAM copies by the game (`texture_relocate(dest, src, format)`, GOAL `texture.gc:1555`) become table moves: `move_existing_to_vram(tex, slot)`; `format == 44` (PSMT4HH) goes to `m_mt4hh_textures` (the font stores two textures on the same VRAM words in different channels, which this scheme has to split).
   * A 16x16 checkerboard (`0xa0303030/0xa0e0e0e0`) is the **placeholder** used for slots whose data hasn't arrived; the code comment explains the original game referenced textures before they were loaded ("a 'bug' in the original game, but can't be seen most of the time").
   * `TextureUploadHandler` runs in each `*_TEX_*` bucket and executes `PC_PORT` upload packets `{vif0 = PC_PORT, vif1 = 3, 16 bytes: page address + mode}` added by the PC-port changes in `texture.gc:959,1069`, in bucket order, so a texture swap in the middle of the frame happens at the same point of the frame as on PS2.
   * Common textures (`GAME.CGO` fonts, HUD) are loaded at startup (`loader->load_common(texture_pool, "GAME")`).
4. `TextureConverter` (`game/graphics/texture/TextureConverter.cpp`, 211 lines) is a 4 MB fake VRAM with `upload`/`download_rgba8888` that covers PSMT8+CLUT32/16, PSMT4+CLUT32/16, PSMCT16 (including the CSM1 CLUT index scramble: `clut_chunk = value/16; clx = chunk&1 ? 8 : 0; cly = (chunk>>1)*2 ...`). It is **no longer used by `TexturePool`** (the include remains; only `TextureAnimator.cpp` refers to the same address helpers). It documents the algorithm you'd need if you ever decode swizzled GS-format textures at run time.
5. **Animated/generated textures** that the game builds in VRAM at run time: Jak 1 has only the sky-blend (`SkyBlendGPU`/`CPU`: two FBO textures blended with weights from the DMA, inserted into the pool at `SKY_TEXTURE_VRAM_ADDRS = {8064, 8096}`), eye textures, and the ocean texture. Jak 2/3/X have `TextureAnimator` (5.8k lines: `VramEntry` fake-VRAM with kinds `CLUT16_16_IN_PSM32 / GENERIC_PSM32 / GENERIC_PSMT8 / GENERIC_PSMT4 / GPU`, `ClutBlender` (blend two CLUTs on GPU -> new texture), `ShaderContext` (TEX0/TEX1/TEST/CLAMP/ALPHA), a pooled `OpenGLTexturePool` (allocate/free by size), `Psm32ToPsm8Scrambler`; the animation commands arrive as `PC_PORT` immediate 12 packets in the texture buckets).

### 5.3 Render-to-texture and framebuffer effects (the pattern used throughout)

All of these keep the rule "a TBP is an identifier": **the renderer redirects drawing to an FBO-owned GL texture and then registers that GL texture in the pool under the TBP the game will later sample from**.
* Helpers (`opengl_utils.{h,cpp}`, 436 lines): `FramebufferTexturePair(w, h, format, num_levels)` + RAII `FramebufferTexturePairContext` (bind FBO, set viewport, restore), `FramebufferCopier` (`glBlitFramebuffer` from the main render FBO to a texture, `copy_now`; `copy_back_now` reverses), `FullScreenDraw`, `FullScreenTexDraw`.
* `EyeRenderer`: a `GpuEyeTex{GpuTexture*, tbp, FramebufferTexturePair fb}` per eye (`NUM_EYE_PAIRS = 20`, enlarged from 32x32 to 128x128 for quality); draws iris/pupil into the FBO from the DMA'd sprite info and installs it with `TexturePool::give_texture_and_load_to_vram(in, tbp)`; Merc then finds eye textures through the normal pool path (`EYE_BASE_BLOCK_JAK1 = 8160`).
* `OceanEnvmap`/`OceanTexture`: watch FRAME_1/SCISSOR writes in the incoming GIF (`scan_gs_set`) to detect "start drawing the envmap page", set `m_offscreen_mode = true` and open an FBO context, draw with the Direct path, then `move_existing_to_vram(m_envmap_gpu_tex, page_tbp)`.
* `BlitDisplays` (Jak 2/3): `PC_PORT 0x10` = copy the framebuffer into a texture registered at `tbp 0x3300`; `0x11` = "copy back" at end of frame; `0x13` zoom blur; `0x14` colour filter; `0x15` slow-time. `Warp`: `FramebufferCopier` then Generic2 in WARP mode samples the copy. `Sprite3_Distort`: copies the framebuffer into `m_distort_ogl.fbo` and draws the distorter mesh sampling it (instanced variant `sprite_distort_instanced.*`). `DepthCue` draws in 16 slices (`TOTAL_DRAW_SLICES`), each time sampling the current framebuffer through `framebuffer_sample_tex` and an intermediate FBO (the GS "depth-cue-base-page" trick), `GlowRenderer` copies depth (`glow_depth_copy.*`) and probes occlusion with small draws read back by `glow_probe_read.*`/`glow_probe_downsample.*` (so glow is a multi-pass GPU algorithm replacing a GS z-buffer readback trick).
* End-of-frame: MSAA render FBO -> `resolve_buffer` (`glBlitFramebuffer GL_LINEAR`) -> window FBO with post shader.

---------------------------------------------------------------------------------------------------

## 6. Platform layer

### 6.1 Window, context, input

* **Library: SDL3**, vendored at `third-party/SDL` = tag `release-3.4.10` per `vendor.yaml`; zlib license. Dear ImGui `v1.92.6` (`third-party/imgui`, SDL3 + OpenGL3 backends) for all debug UI; GL loader **glad 0.1.34** generated for `gl=4.3`, *compatibility* profile, extensions `GL_ARB_bindless_texture, GL_ARB_texture_filter_anisotropic` (`third-party/glad/include/glad/glad.h` header); window icon via `stb_image`. No GLFW (stale comments in the code mention "glfw": `OpenGLRenderer.h:"provided by glfw"`, `opengl.h: #define GLFW_INCLUDE_NONE`).
* **Context creation** (`game/graphics/pipelines/opengl.cpp`): `gl_init` sets attributes *before* the window exists: `SDL_GL_DOUBLEBUFFER 1`, `PROFILE_CORE`, `CONTEXT_FLAGS = DEBUG_FLAG if settings.debug else 0`, `MAJOR_VERSION 4`, `MINOR_VERSION 3` (**1 on `__APPLE__`**), depth 24, stencil 8, alpha 8. `gl_make_display` then `SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY)`, `SDL_GL_CreateContext`, `SDL_GL_MakeCurrent`, `gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)`, then constructs `GraphicsData` (TexturePool, Loader, `OpenGLRenderer`) **after** GL is current, then ImGui init. Error dialogs say "OpenGOAL requires OpenGL 4.3" (not adjusted for the Mac 4.1 case).
* `GfxDisplay`/`GLDisplay`: one display per window (supports spectator windows in the design, only the main one is used); `DisplayManager` (`game/system/hid/display_manager.{h,cpp}`, 144+537): windowed/fullscreen/borderless via `SDL_SetWindowFullscreen`, `SDL_SetWindowFullscreenMode`, multi-monitor `SDL_GetDisplayBounds`, `SDL_SyncWindow`; settings persisted (`game/settings`). `InputManager` (760 lines + `input_bindings` 808 + devices 1,400): SDL keyboard/mouse/gamepad (incl. DualSense adaptive trigger and rumble) -> `PadData` that `scePadRead` copies into the game's `CPadInfo` (section 7).
* Hi-DPI: `SDL_GetWindowSizeInPixels` each frame for FBO sizing; `SDL_GetWindowDisplayScale` only for picking the icon.
* VSync: `SDL_GL_SetSwapInterval(vsync)` toggled lazily on the render thread.

### 6.2 GL version required per OS, and everywhere the code adapts to macOS 4.1

Requested version: Linux/Windows 4.3 core; macOS 4.1 core (`opengl.cpp:136-141`, `gfx_test.cpp:28`). **Every shader is `#version 410 core`** (92 of 92 files), so the same GLSL runs everywhere. A search of the renderer for GL 4.2+ features (compute, SSBOs, image load/store, `glTexStorage`, `layout(binding=)`, `glBufferStorage`, DSA, `glMemoryBarrier`, `glCopyImageSubData`, `glClipControl`): **none are used**. What it does use: UBOs (`ub_bones`, GL 3.1), `glMultiDrawElements`, primitive restart, `glBlitFramebuffer`, MSAA renderbuffers and `GL_TEXTURE_2D_MULTISAMPLE`, `glColorMaski`, `glBlendFuncSeparate`, `glPolygonMode` (debug only), instanced draws (distorter), `glDrawBuffers`, anisotropic filter. All GL 4.1 core. (The only 4.3-class call is `glDebugMessageCallback`, which is guarded.)

Every place the source adapts to macOS (complete list from `grep __APPLE__`, graphics-related first):

| Where | Adaptation |
|---|---|
| `game/graphics/pipelines/opengl.cpp:136` | `#ifndef __APPLE__ MINOR_VERSION 3 #else MINOR_VERSION 1` |
| `game/graphics/pipelines/opengl.cpp:303` | ImGui GLSL string: `"#version 410"` on Apple vs `"#version 430"` |
| `game/graphics/opengl_renderer/OpenGLRenderer.cpp:79` | `glEnable(GL_DEBUG_OUTPUT); glDebugMessageCallback(...)` only `#ifndef __APPLE__` (GL 4.3 / KHR_debug is absent on Apple's 4.1 driver) |
| `game/graphics/gfx_test.cpp:28` | the `--gpu-test` probe window requests 4.3, overridden to 4.1 on Apple |
| `CMakeLists.txt:40` | `enable_language(OBJC)` and `CMAKE_OSX_SYSROOT` from `xcrun --show-sdk-path` for SDL3 (comments reference SDL3 regressions: libsdl-org/SDL#6455, #12078) |
| (shaders) | all `#version 410 core`, deliberately a common baseline |
| (glad) | generated for 4.3 compat; on Apple the extra function pointers simply stay null **(inference: any accidental 4.2+ call would crash only on Mac; none found)** |

Other `__APPLE__` code is about the **GOAL-runtime**, not graphics, and doesn't apply to us: `runtime.cpp` (`MAP_32BIT`/no `MAP_POPULATE`; on Apple Silicon `mmap` RW then `mach_vm_remap` a second RX alias of EE RAM because W^X forbids RWX: `g_ee_main_mem_exec`), `main.cpp:187` (AVX required; on Intel-less Macs requires macOS >= 15 because Rosetta only emulates AVX since Sequoia), `kernel/*/kscheme.cpp`, `kernel/common/codegen.h`, `mips2c_private.h` (ABI differences in the generated-code trampolines), `SystemThread.cpp:102` (`pthread_setname_np` signature), `common/util/os.cpp`, `FileUtil.cpp` (paths), `xdbg.cpp` (ptrace debugger), `game/common/vu.h:30`.

### 6.3 Build system

* Root `CMakeLists.txt` (C++20; presets in `CMakePresets.json`: Windows MSVC/clang, Linux gcc/clang, `Release-macos-x86_64-clang`, `Release-macos-arm64-clang`, `-static` variants; `ASAN_BUILD`, `STATICALLY_LINK`). Helper `build_third_party_lib(dir target)` does `add_subdirectory(third-party/<dir> EXCLUDE_FROM_ALL)` and marks includes `SYSTEM` + `-w`. Order: curl, replxx, sqlite3, tree-sitter, tinyfiledialogs, fmt, **`common`**, cubeb, lsp, libco, zstd, SDL3, imgui, **`game`**, goalc, tools, decompiler, googletest, tests, lzokay, stb_image, draco/tiny_gltf (level export to glTF), xdelta3, discord-rpc.
* Targets: `common` (static lib: `common/CMakeLists.txt`, includes `dma/dma_copy.cpp, dma.cpp, gs.cpp, custom_data/*, texture/*, math/*, util/*`; links `fmt lzokay replxx libzstd_static tree-sitter sqlite3 libtinyfiledialogs tiny_gltf`), `runtime` (static lib of everything in `game/CMakeLists.txt` `RUNTIME_SOURCE` + `third-party/glad/src/glad.c`; links `common fmt SDL3::SDL3 imgui discord-rpc sound stb_image libco libcurl` + `pthread dl` or Windows `mman`), `gk` (the executable, `game/main.cpp`), `sound` (static lib linking `cubeb`), `goalc` (compiler), `decompiler`, tests.
* **What the renderer path actually needs** vs tools: the renderer itself (everything in `game/graphics/**`) needs only SDL3, glad, ImGui (debug UI; removable), stb_image (icon), fmt, zstd (to read `.fr3`), and the `common` utility sources it uses (`dma`, `custom_data`, `math/Vector.h`, `util/Serializer.h`, `util/FileUtil`, `util/Timer`, `util/FrameLimiter`, `log`, `Assert`). Everything else in the link line is the *rest of the runtime or tooling*: libcurl (HTTP update checks/launcher), discord-rpc, sqlite3 (Jak 2/3's in-game editor), tree-sitter + replxx (compiler REPL/formatter), tinyfiledialogs (error dialogs), lzokay (asset decompression), draco/tiny_gltf (decompiler glTF export), libco (IOP coroutines), cubeb (audio).
* Other flags: Clang default is `-mavx` (Apple Silicon non-x86: `-march=native -mcrc`); release `-O3`; Mac linker `-Wl,-stack_size -Wl,0x20000000`. `third-party/sse2neon` provides SSE intrinsics on ARM (used by `vu.h` and `merc blerc_avx`).
* `docs/setup/system/macos.md` (full text summary): needs Apple Silicon on **macOS Sequoia** or an Intel Mac; builds on macOS 15. Install Xcode CLT; on Apple Silicon install Rosetta 2 (`softwareupdate --install-rosetta`). **x86_64 build** (the supported one, runs under Rosetta on M-series): `brew install cmake nasm ninja go-task clang-format openssl@3`, `export OPENSSL_ROOT_DIR=$(brew --prefix openssl@3)`, `cmake -B build --preset=Release-macos-x86_64-clang`, `cmake --build build --parallel $(sysctl -n hw.logicalcpu)`. **ARM64 build: "experimental, unsupported"**, same without `nasm`, preset `Release-macos-arm64-clang`; may need `LIBRARY_PATH` to the CLT SDK `usr/lib`. `AGENTS.md` in that repo states: "At the moment we support x86_64 on Windows, Linux and macOS (via Rosetta translation)". Linux doc lists the SDL3 build dependencies (libxrandr, libxinerama, libxcursor, libxi, libpulse, libgl1-mesa-dev). Windows doc: VS 2022 + Scoop for llvm/nasm/cmake.

### 6.4 Apple-specific observations that matter for us

* OpenGL on macOS is frozen at 4.1 and deprecated (general knowledge, not from the repository). OpenGOAL demonstrates that a 4.1-core renderer can cover the whole PS2 feature set they needed (including render-to-texture, MSAA, multi-draw). For a long-lived Mac port you would probably put a thin abstraction over the GL calls and add a Metal backend later; the `GfxRendererModule` struct in `gfx.h` is the seam they left for that.
* Window/GL context must be created and driven on the main thread on macOS; OpenGOAL already runs the game on a secondary thread for that reason (inference).

---------------------------------------------------------------------------------------------------

## 7. The rest of the PS2 that had to be replaced

Our C code will call the real Sony APIs by name; OpenGOAL's game code (GOAL) calls the same libraries through C++ glue registered as GOAL function symbols (`make_function_symbol_from_c("syncv", ...)`, `kernel/jak1/kmachine.cpp:544-610`; ~114 registrations across `kernel/common/kmachine.cpp` (1,235 lines) and `kernel/jak1/kmachine.cpp` (651)). Names/typedefs follow Sony's ABI (`ee::scePadRead`, `sceMcOpen`...) but the implementations were written from observed behaviour (SPDX headers in the sound code credit individual authors under ISC); I did not check whether any header text derives from Sony material, which is relevant to your project rule about the Sony SDK.

| PS2 facility | OpenGOAL implementation (files) | Approach | Size |
|---|---|---|---|
| **GS sync/vsync/path** | `game/graphics/sceGraphicsInterface.{h,cpp}`, `game/sce/libgraph.cpp`, `stubs.cpp` | `sceGsSyncV` / `sceGsSyncPath` -> condition variables in `opengl.cpp` (section 1.4); `sceGsResetGraph/PutIMR/GetIMR/ExecStoreImage` are `ASSERT(false)` stubs; Jak 1 `PutDisplayEnv` (`kernel/jak1/kmachine.cpp:413`) is not a no-op: it reads byte 1 of the display env and calls `set_pmode_alp(alp / 255.f)` (the blackout/fade amount); the rest of the env is ignored | ~70 |
| **DMA** | `game/sce/libdma.{h,cpp}` | `sceDmaSync` returns 0 immediately ("successful transfer finish, without timeout"); the DMA *kick* is `__send-gfx-dma-chain`; GOAL `dma-send*` functions "won't work properly on PC" (`goal_src/jak1/engine/dma/dma.gc` comment) so game code was changed at the call sites. Scratchpad (SPR) DMA is still used on the game thread for EE-side work (`dma-send-to-spr`); on PC the scratchpad is plain RAM. | 15 |
| **EE timers / clock** | `kernel/common/kmachine.cpp` `read_ee_timer()` | `Timer ee_clock_timer` (std::chrono) scaled `ns * 3 / 10` = 300 MHz ticks (`TICKS_PER_SECOND = 300` in `goal_constants.h`); vsync-driven counters come from the render loop | 10 |
| **Controller (libpad)** | `game/sce/libpad.{h,cpp}` (154+64); `game/system/hid/*` | `scePadPortOpen` returns `port+1`; `scePadGetState` always "stable"; `scePadInfoMode` advertises DualShock 2; `scePadRead(port, slot, rdata)` fills a 32-byte `CPadInfo`: `button0` bits, analog sticks, pressure array from `InputManager::get_current_data(port)` (a `shared_ptr<PadData>` from a per-port map; keyboard + any SDL gamepad, remappable, per-port binding); `scePadSetActDirect` -> rumble through SDL (incl. DualSense trigger effects), queued as `EEInputEvent`s that the render thread executes (`process_ee_events`) because SDL calls must stay on that thread. Input is polled on the render thread (`GLDisplay::render`); I saw a mutex on the event queue (`m_event_queue_mtx`) but none in the `get_current_data` accessor **(inference: pad data is handed over without an explicit lock; benign torn reads)**. | libpad 218 + hid 4,087 |
| **Memory card (libmc + game save layer)** | `game/kernel/common/kmemcard.cpp` (716) = the game's save/load state machine; `game/sce/sif_ee_memcard.{h,cpp}` (64+308) = a `sceMc*` emulation | Two layers. (1) `kmemcard.cpp` does not rely on a card at all: "instead of two memory cards we just simulate the 4 save files (8 banks)" (`MemoryCardFile mc_files[4]`); `MC_run`, `MC_makefile`, `MC_get_status`, `MC_check_result`, `pc_update_card`, `pc_game_save_synch`, `pc_game_load_synch` do plain `fopen`/`file_util::read_binary_file` on `<user memcard dir>/<serial>AYBABTU!/bankN.bin` (`file_util::get_user_memcard_dir(version)`; file name tables per game), using the original 0x400-byte `McHeader{save_count, checksum, magic 0x12345678, preview_data[64], data[944], save_count2}`; operations complete synchronously and the PS2-style callback is invoked; the memory card "handle" is a made-up constant (`PC_MEM_CARD_HANDLE = 0x6C616F67`). (2) `sif_ee_memcard.cpp` is a full in-memory libmc emulation: `CardData{unordered_map<string, File{vector<u8>, is_directory}>, is_formatted}`, calls `sceMcInit/GetInfo/Open/Close/Read/Write/Seek/Mkdir/GetDir/Delete/Format/Unformat/Sync` all complete instantly and are reported through `sceMcSync` (`current_function`, `current_function_result`); `sceMcInit` loads `user/memcard.bin` (via `common/util/Serializer.h`) and `flush_memory_card_to_file()` can write it, but **in this checkout nothing calls the flush** (grep), so this layer is effectively volatile **(inference: the real save path is layer 1)**. | ~1,100 |
| **CD/DVD (libcdvd)** | `game/sce/libcdvd_ee.{h,cpp}` (63+33) | `sceCdInit` ok, `sceCdMmode` records CD/DVD, `sceCdDiskReady` always ready, `sceCdGetDiskType` returns PS2CD/PS2DVD from the mode. No sector reads on the EE side. | 100 |
| **File I/O + sound/stream RPCs via IOP** | `game/sce/sif_ee.{h,cpp}` (76+193): `sceSifLoadModule`, `sceSifBindRpc`, `sceSifCallRpc`, `sceSifCheckStatRpc`, `sceOpen/sceClose/sceRead/sceWrite/sceLseek`; `kernel/common/fileio.cpp`, `kernel/jak1/fileio.cpp`; `kernel/common/kdgo.cpp` + `kernel/jak1/kdgo.cpp` (DGO archive loading via IOP RPC); `kernel/common/ksound.cpp` | `sceOpen` etc. use `fopen` on `file_util::get_file_path(...)` under the project dir; `kernel` code sets `isodrv = fakeiso` ("fakeiso is the only one that works in opengoal"): game files come from extracted directories (`out/<game>/...`), not a disc. `sceSifLoadModule("cdrom0:\\DRIVERS\\OVERLORD.IRX;1")` is special-cased: it starts the C++ overlord with the module args. RPC calls are queued to the IOP thread (`IOP_Kernel::sif_rpc`) and polled with `sceSifCheckStatRpc`. | ~900 |
| **IOP kernel & overlord** | `game/system/IOP_Kernel.{h,cpp}` (243+545), `iop_thread.*`, `game/sce/iop.{h,cpp}` (176+274), `game/overlord/{common,jak1}/*` (~5,400 lines for Jak 1: ISO/stream/queue/ramdisk/ssound/srpc), `third-party/libco` | The IOP's multithreading kernel is reimplemented as **cooperative coroutines (`co_create(0x300000 stack)`) on a single OS thread**: `CreateThread/StartThread/SleepThread/WakeupThread/DelayThread/YieldThread`, semaphores (`WaitSema/SignalSema/PollSema`), event flags, mailboxes, RPC server loop (`rpc_loop`, `sif_rpc`), priority scheduler (`schedNext`, `dispatch`) with time-based wakeups. The "overlord" IOP module (stream/file/sound command server, decompiled from the retail IRX and ported to C++) runs on top of it with `fake_iso` (file lookups in extracted dirs, `FS_Open/FS_BeginRead/FS_SyncRead/FS_LoadMusic/FS_LoadSoundBank`; `DMA_SendToEE` = memcpy into EE RAM). Timings of disc reads are instant (`overlord/todo.txt` lists known simplifications). | ~7,000 |
| **Sound (SPU2 + 989snd)** | `game/sound/` (sndshim 306, sdshim 131 + `989snd/*` ~4,400 + `common/*` ~560 = ~6,300 lines); `third-party/cubeb` | The Sony SPU2 `sceSd*` API is a **shim over a software mixer**: `spu_memory[0x15160*10]`, 48 `snd::Voice`s with ADSR envelope, ADPCM (VAG) decoding, pitch/volume/reverb-less mix (`common/voice.cpp`, `synth.cpp`, `envelope.cpp`); `sdshim.cpp` implements `sceSdSetSwitch(KON/KOFF)`, `sceSdGetAddr`, DMA callbacks etc. On top, `989snd/` re-implements the sound-bank player (SFX blocks, MIDI sequences, AME scripting, LFO, `loader`, `blocksound_handler`, `midi_handler`, `ame_handler`) that the Jak IOP module embedded ("Naughty Dog used a third party library for sound called `989SND`", `docs/project-overview.md`). Output through **cubeb** (CoreAudio/WASAPI/PulseAudio/ALSA...) callback `Player::sound_callback`. | ~6,300 |
| **Threads/process model for the game** | GOAL's own scheduler (`kernel/common/kscheme.cpp`, `kernel/jak1/kscheme.cpp` 1,887, asm stack switching in `kernel/asm_funcs_x86_64.asm`/`_arm64.s`) | GOAL processes and "suspend" are done with explicit stack switching, not PS2 kernel threads; Sony `CreateThread` isn't used on EE. **Not applicable to a C game** unless your C game uses EE threads, in which case you'd map them to host threads or coroutines (compare IOP_Kernel). | n/a |
| **Language/region, clock, aspect** | `game/sce/libscf.{h,cpp}` (45+105) | `sceScfGetLanguage` from the OS locale (Win32 `GetUserDefaultUILanguage`; POSIX branch uses `clocale`), `sceScfGetAspect` = 4:3, `sceCdReadClock` fills a `sceCdCLOCK` from `localtime()` in BCD (`60 -> 0x60`), used by `DecodeTime` | 150 |
| **Debug/DECI2** | `game/sce/deci2.*`, `game/system/Deci2Server.*` | TCP bridge to the REPL; irrelevant for us | 340 |
| **Host services** | `game/system/SystemThread.*`, `background_worker.*`; `kernel/common/kmalloc.cpp` (215, allocator working inside the 128 MB EE buffer (the GOAL heaps); not otherwise inspected) | thread helpers with CPU-usage stats; bump/level allocator on the EE RAM | ~500 |

Also relevant: EE hot assembly (`draw-string`, collision, bones, sparticle, sky, ripple, ocean_vu0 ...) was converted to C++ by `decompiler/analysis/mips2c.cpp` ("Mips2C") into 32,611 lines under `game/mips2c/jak1_functions` (+ similar for jak2/3/x), each line a literal one-instruction call (`c->lqc2(vf26, 732, v1); c->vadd_bc(DEST::xy, BC::w, vf26, vf26, vf0);` ... from `game/mips2c/readme.md`), using `game/mips2c/mips2c_private.h` (1,769 lines: `ExecutionContext` with GPRs/VU0 macro-mode registers). **Not applicable to us** because our decompiled sources are already C for the whole game, but the VU0 macro-mode (COP2) instruction semantics in `mips2c_private.h` are a tested reference if our C code leaves any inline VU0 assembly.

---------------------------------------------------------------------------------------------------

## 8. Reuse verdict

ISC is a "do what you want, keep the notice" license, so reuse is fine legally for everything first-party here. Technical fit is the real question. "C game" = plain C compiled by a host compiler (no GOAL, no goalc).

| Component | Files | Lines | Verdict | Reason |
|---|---|---|---|---|
| DMA chain reader, tags, VIF codes | `common/dma/dma.h`, `dma.cpp`, `dma_chain_read.h` | 437 | **Reuse nearly as is** | PS2-exact, header-only core, tiny dependency set (fmt, Assert, common_types). Add support for `spr` if needed; REFS/PCE/TTE not modelled; CALL depth 2. Check R&C's use of tag kinds. |
| GS/GIF register wrappers, `DrawMode`, `AdGifData` | `common/dma/gs.h`, `gs.cpp` | 1,055 | **Reuse nearly as is** | pure bitfield accessors, no dependencies beyond dma.h. `AdGifData` is Jak's shader record layout; R&C may differ (reuse the idea). `DrawMode` is a good batching key. |
| DMA copier/verify/serialize | `common/dma/dma_copy.*` | 254 | **Reuse the idea (or as is)** | chunk-copy + fixup is a neat way to snapshot a frame for capture/replay/unit tests; makes the frame immune to the game thread. Needs `Serializer.h`. |
| DMA helper asserts | `opengl_renderer/dma_helpers.*` | 189 | **Reuse as is** | convenience for "expect exactly this unpack" |
| GS memory addressing + CLUT scramble | `common/texture/texture_conversion.h`, `game/graphics/texture/TextureConverter.*` | 225 + 235 | **Reuse nearly as is** (only if you decode swizzled textures at run time or write your own extractor) | self-contained tables for PSMCT32/16, PSMT8, PSMT4. Offline conversion is what they did. |
| VU1 register file | `game/common/vu.h` | 610 | **Reuse if you port VU programs mechanically** | SSE (use sse2neon on ARM; present in vendored tree); includes VU float quirks. |
| Mechanical VU1->C++ ports | `OceanMid_PS2.cpp`, `OceanNear_PS2.cpp`, `Shadow_PS2.cpp` | 11,117 | **Not directly reusable** (specific to Jak microcode) | but they show the *method*, and the translator is not in the repo. If R&C's VU1 programs are similar in style you'd write your own translator (disasm in `decompiler/VuDisasm/` is ~5 files; ISC; reusable as a starting point for a VU1 disassembler). |
| Frame hand-off & sync | `opengl.cpp` (`gl_send_chain/gl_vsync/gl_sync_path`, `GraphicsData`), `gfx.*` | ~400 relevant | **Reuse the idea, rewrite** | the mutex/condvar protocol is simple and proven; their module-of-lambdas `GfxRendererModule` is a good seam for multiple backends. |
| Frame limiter | `common/util/FrameLimiter.*` | 81 | **Reuse as is** | small; Win32 branch uses `timeBeginPeriod`. |
| Bucket dispatch & `SharedRenderState` | `BucketRenderer.*`, `OpenGLRenderer.cpp` `dispatch_buckets_*`, `buckets.h` | ~600 | **Reuse the idea, rewrite** | relies on Jak's exact bucket linked-list layout; R&C's display-list structure must be examined first (**see [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md)**). |
| `DirectRenderer` | `DirectRenderer.*`, `direct_basic*.glsl`, `debug_red.*` | ~2,000 | **Reuse nearly as is** for the 2D/HUD/font/debug path (best first reuse target) | a faithful GS drawing-register + primitive-assembly model that turns arbitrary GIF packets into GL triangles; known gaps are listed in 4.4. Strip Jak-specific constants (zbp 448/304, 448 vs 416 height, texture unit 20, per-game scissor/size constants). Needs rewriting only the texture lookup (TexturePool) and `SharedRenderState` coupling. |
| `DirectRenderer2` | `DirectRenderer2.*`, `direct2.*` | ~1,040 | **Reuse the idea** | better batching (draw records + 10 texture units + strip restart), but untested in this checkout (unused). |
| Sprite3 | `sprite/Sprite3*.cpp`, shaders | 2,792 | **Reuse the idea, rewrite** | demonstrates how to port a VU1 sprite program to a vertex shader; the data layouts are Jak's. |
| Generic2 | `foreground/Generic2*` | 2,007 | **Reuse the idea, rewrite** | same; the generic VU1 program is Jak-specific, but the "re-derive GS state per adgif and bucket by DrawMode" machinery is general. |
| Merc2 + merc2.vert | | 1,983 | **Reuse the idea, rewrite** | GPU skinning with UBO bones and LOD/effect masks; fed by Jak's offline `MercModel`. |
| TFrag/Tie3/Shrub/Hfrag/background_common | `background/*` | ~3,900 | **Reuse the idea, rewrite** | the offline-mesh approach is Jak-specific (needs an extractor), but BVH culling + time-of-day lookup + multi-draw list building are generic techniques. |
| Ocean, Shadow, Sky, Eye, DepthCue, Glow, Distort, TextureAnimator, BlitDisplays, Warp | | ~25,000 | **Not applicable** (Jak-specific effects) | use as worked examples when we meet the equivalent R&C effect. |
| TexturePool (VRAM-slot -> GL texture table, placeholder, mt4hh, relocate) | `texture/TexturePool.*` | ~810 | **Reuse the idea** | the "TBP is an identifier, not an address" model plus pre-converted RGBA8 textures is the key simplification. The `PcTextureId` machinery is tied to Jak's tpage directory. |
| Loader (background file I/O, staged GPU upload with time budgets, deferred deletes) | `loader/*` | 1,597 | **Reuse the idea** | good pattern; data format is Jak's `tfrag3`. |
| `tfrag3` data format + serializer | `common/custom_data/*` | 1,600 | **Reuse the idea** | needs an extractor to produce it; version-stamped (`TFRAG3_VERSION = 43`) and zstd'd. |
| Level/asset extractor | `decompiler/level_extractor/*` | 18,216 | **Not applicable** | consumes GOAL object files/BSP structures. |
| FBO helpers | `opengl_utils.*`, `Fbo.h` | 487 | **Reuse nearly as is** | small, generic (FramebufferTexturePair, FramebufferCopier, FullScreenDraw). |
| Shader loader / token substitution | `Shader.*` | 226 | **Reuse nearly as is** | trivial; embed sources instead of reading from disk. |
| SDL3 window/GL context + display manager + input | `pipelines/opengl.cpp` (window part), `system/hid/*` | ~5,000 | **Reuse nearly as is** (display_manager, input_manager); context bits are ~100 lines | very complete (fullscreen modes, multi-monitor, gamepad+DualSense, bindings). Depends on `game_settings` and `lg::`; decouple. |
| ImGui debug UI | `Profiler.*`, `debug_gui.*`, per-renderer `draw_debug_window` | ~750 | **Reuse the idea** | per-bucket enable/disable, per-renderer stats, texture pool browser are what made their bring-up possible. |
| libpad / libmc / libcdvd / libscf / libdma stubs | `game/sce/*` | ~1,000 (without iop) | **Reuse the idea, rewrite** | ours call the real Sony function names with our own types; the behaviour (instant completion, one in-memory memcard saved to `user/memcard.bin`, always-ready CD, pad mapping) is exactly what we need. |
| IOP kernel emulation, overlord, fake_iso | `system/IOP_Kernel.*`, `sce/iop.*`, `overlord/*` | ~7,000 | **Reuse the idea** | if R&C also runs an IOP-side file/stream server, a coroutine IOP kernel + C++ module is the way; the overlord code itself is Jak's. |
| Sound (SPU2 shim + 989snd) | `game/sound/*` | ~6,300 | **Reuse the idea / possibly parts as is** | if R&C uses the same 989snd bank formats (**see [RAC1_PAL_SURVEY.md](RAC1_PAL_SURVEY.md)**), `sdshim` + `common/{voice,synth,envelope}` + cubeb output are directly valuable; sound banks are format-specific. |
| GOAL compiler, runtime kernel (kscheme/klink/kdgo/symbol table), `goalc`, `goal_src`, mips2c, REPL/DECI2, debugger | `goalc/`, `game/kernel/**` GOAL parts, `goal_src/`, `game/mips2c/**` | huge | **Not applicable to a C game** | everything tied to GOAL types, symbols, linking, JIT-like loading. |
| Build system pieces | root `CMakeLists.txt`, `CMakePresets.json`, `game/CMakeLists.txt` | | **Reuse the idea** | `build_third_party_lib` pattern; presets; `-mavx` vs `-march=native -mcrc` split; SDL3 on Mac needs `enable_language(OBJC)`. |

### 8.1 Licenses

**Project license** (`LICENSE`): ISC, "Copyright (c) 2020-2026 OpenGOAL Team". Obligation: *"Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies."* So any copied or substantially-derived file must carry (or be accompanied by) that copyright line and permission notice; no attribution-in-UI or source-disclosure duty; warranty disclaimer. Some files carry extra SPDX headers (`// Copyright: 2021 - 2024, Ziemas  // SPDX-License-Identifier: ISC` in `game/sound/989snd/*`): keep those headers when reusing that code. The README states the project ships **no game assets** and requires users to supply their own legally obtained game copy (`README.md:25`); the repo contains disassembled VU reference listings and game-derived constants/layouts in source form, which is the project's own interpretation, not a statement I can verify legally.

**Third-party libraries on the renderer/runtime path** (as vendored in `third-party/`; license text read from the vendored files unless marked):

| Library | Version/ref | License | Notes |
|---|---|---|---|
| SDL3 | `release-3.4.10` | zlib (`third-party/SDL/LICENSE.txt`) | altered versions must be marked; notice must not be removed; attribution in docs "appreciated, not required" |
| Dear ImGui | `v1.92.6` | MIT (`third-party/imgui/LICENSE.txt`, (c) Omar Cornut) | keep copyright + permission notice; debug UI only |
| glad (GL loader) | 0.1.34 generated, gl 4.3 compat | generator output with no license header in `glad.c`/`glad.h`; includes Khronos `KHR/khrplatform.h` (MIT-style, "Copyright (c) 2008-2018 The Khronos Group Inc."; keep the notice) | glad generated output is widely treated as public-domain/MIT-like (**general knowledge, not stated in the vendored files**); you can regenerate your own loader for core 4.1 |
| stb_image / stb_image_write | v2.21 / v1.11 | dual MIT **or** public domain (Unlicense) per the footer of `stb_image.h`; `stb_image_write.h` header says "public domain" | |
| {fmt} | 11.1.4 | MIT (`third-party/fmt/LICENSE`) | needed by `common/dma` and logging |
| zstd | vendored lib | BSD-3-style **or** GPLv2 at your option (header of `zstd/lib/zstd.h`; no LICENSE file vendored) | choose BSD; needed for `.fr3` reading |
| libco | in-tree | ISC (`third-party/libco/LICENSE`, "ares team, Near et al") | IOP coroutines only |
| cubeb | in-tree | ISC-style (`cubeb/LICENSE`: "Copyright (c) 2011 Mozilla Foundation", permission to use/copy/modify/distribute with notice) | audio output |
| lzokay | in-tree | MIT (`third-party/lzokay/LICENSE`, "Copyright (c) 2018 Jack Andersen") | asset decompression |
| sqlite3 | amalgamation 3.42 | public domain (`sqlite3/LICENSE`) | not renderer |
| libtinyfiledialogs | v2.9.3 | zlib (header of `tinyfiledialogs.c`) | error dialogs |
| discord-rpc | | MIT ("Copyright 2017 Discord, Inc.") | optional |
| libcurl | 8.21.0 | curl license (MIT-like) | HTTP; not needed for rendering |
| sse2neon | v1.9.1 | MIT (per `vendor.yaml`) | SSE-on-ARM for `vu.h` |
| mman-win32 | in-tree | license not checked (header says "mman-win32") | Windows mmap shim for the EE-RAM buffer; not needed unless you mimic their memory model |
| fpng | | Unlicense/public domain (`fpng.cpp` footer) | screenshots |
| googletest, tree-sitter, replxx, draco, tiny_gltf, xdelta3, zydis, capstone, CLI11, inja, json.hpp, magic_enum | | various permissive (MIT/BSD/Apache-2/ MPL for capstone/zydis variants) **not checked in detail** | tools only |

Practical consequence: the whole runtime/renderer dependency set is permissive (zlib/MIT/ISC/BSD/public-domain). The only "choose" license is zstd (take BSD). Nothing on the renderer path is GPL/LGPL.

---------------------------------------------------------------------------------------------------

## 9. Design consequences for the R&C renderer (all inference, from the above)

1. **Decide the interface first: "chain interpreter" vs "command list".** Jak's renderer interprets a faithful PS2 chain, and had to alter the game (PC_PORT packets, skipped MPG, skipped geometry DMA) to make fast paths possible. We compile our own C, so we can emit either (a) the original PS2 chain (maximum fidelity, like their direct/generic/sprite paths), or (b) a higher-level command stream next to it (like their PC_PORT packets), or replace sub-systems completely. The cleanest split for us is probably: keep PS2 chain semantics for 2D/HUD/menus/fonts (DirectRenderer-style), and replace per-object VU1 draws with explicit "draw this model/level chunk with these matrices/colors" calls.
2. **Do not emulate GS VRAM.** Their texture design (VRAM address -> opaque texture id, pre-converted RGBA8 textures registered when the game "uploads", placeholders when late, FBO textures registered at the TBP the game will sample) avoids GS swizzling and CLUT work at run time. Converting at asset-load time needs a texture-page parser for R&C's formats; `texture_conversion.h` is the reference for the address math.
3. **VU1 programs: three tools in this order** (their experience): identify the program by MSCAL address and mirror its VU memory map in C++ structs; port the math into a vertex shader keeping the asm in comments (sprite, generic, merc); for output that is not trivially shader-shaped (ocean, shadow) first do a mechanical CPU port (needs a VU1 interpreter or translator that this repo doesn't ship), then replace it by a semantic rewrite once understood. Keep reference disassemblies and annotated notes in the repo.
4. **Depth/space conventions:** reverse-Z style (clear 0, `GEQUAL`), GS-pixel-space outputs mapped in the vertex shader (`-2048`, `/256`, `/-128`, `z/2^23 - 1`, then `*w`), colours in 0x80 = 1.0 (double at the end), alpha test in `aref/128`, `ADC` bit carried per vertex, STQ divided per pixel, FST UV in pixels. Copy these exactly; they are the hard-won parts.
5. **Threading:** render on the main thread; hand-off with mutex + two condvars; `sync_path` = wait for render done, `syncv` = wait for swap. Prefer to snapshot the frame (their optional copier) over reading live game memory.
6. **Limits to expect in a GL 4.1 core baseline (macOS):** no compute, no SSBO, no debug callback; they live with it (everything in `#version 410 core`). Keep shaders in that subset and the Mac port comes for free on GL. Consider a thin graphics API wrapper from day one.
7. **Test strategy worth copying:** the frame-dump format (`FixedChunkDmaCopier::serialize_last_result`), per-bucket enable checkboxes in ImGui, per-renderer ASSERT on every structural assumption, and VU disassembly regression tests (`test/decompiler/test_VuDisasm.cpp`).
8. **What this repository cannot answer:** whether R&C's display lists use bucket linked lists like Naughty Dog's engine (Jak's renderer depends on that shape: `next_bucket`, the default-regs CALL); whether R&C reuses 989snd and the same sound-bank formats; how R&C stores texture pages and CLUTs. Those decide how much of sections 3-5 carries over structurally.
