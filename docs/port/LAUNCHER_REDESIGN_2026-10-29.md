# Switch launcher redesign (2026-10-29): not built yet

## Codex build follow-up

The historical not-built statements below describe Claude's session. On user
request, the actual Switch Deko3D target has now built successfully (inner exit0).
Copied NRO/ELF hashes match the build outputs; the complete heading TTF is embedded
in the ELF and DekoSurface is present. Both shader packs match staged RomFS.
Identity is recorded in build/deko3d-test/last-build.json, hardware_tested=false.
Previous candidate is preserved in diagnostics/2026-10-29-before-launcher-redesign/.
No launcher/game source changes were needed for this build. Console navigation,
touch, presentation and launcher-to-game handoff still need hardware testing.

---

This implements the mockup the user approved, in the real launcher.

**Not built yet.** The Claude session has no devkitA64. Only rendering, gameplay-independent menu
logic and the launcher's presentation changed. The game renderer, shaders and settings format are
unchanged.

## What changed

### Look
`port/platform/switch/launcher_canvas.cpp` is rewritten. It is still drawn on the CPU into one
1280x720 RGBA buffer.
- **Top bar:** the brand, the sections (L / R) and the disc ID.
- **Bottom bar:** button hints, drawn as round A/B/X/Y and L/R/D-pad keys.
- **Anti-aliasing:** shapes, check marks and the icons are anti-aliased.
- **Play page:**
  - a hero banner (your art, the included logo, or a drawn planet when neither is chosen);
  - a readable panel that fades into the art;
  - Launch and Diagnostic boot buttons;
  - three readiness cards.
- **Settings / Customize:**
  - a page list with icons;
  - rows with ◀ value ▶ on the selected row;
  - toggles for On/Off;
  - colour swatches next to hex colours;
  - a scroll bar;
  - a side panel explaining the selected row, which also shows the last notice.
- **Saves, Setup, About, first-launch progress and the button rebind screen:** restyled.

### Fonts
The system font is still used for body text. Titles use Chakra Petch Bold:
- SIL OFL 1.1;
- stored at `port/platform/switch/fonts/`;
- embedded by `cmake/Window.cmake` as a generated header (the same mechanism as the launcher's icon);
- served by `artwork::heading_font()`.

If the face fails to load, titles fall back to the system font.

### Colours
- `launcher_appearance.cpp` `dark()` is now navy/amber. Saved "Dark" users get it automatically.
- Custom and Light themes still work.
- Customize > Appearance still edits every colour. The row label "Sidebar color" is now "Bar color";
  it sets the top and bottom bars.

### Navigation (`launcher.cpp`)
- L / R moves between six sections (`launcher_ui.h` `section_of` / `section_pages`):
  - Play;
  - Settings: Display, Audio, Gameplay, Controller, Game fixes;
  - Saves;
  - Customize: Artwork, Appearance;
  - Setup;
  - About.
- Settings and Customize open on their page list. Up/Down picks a page; A or Right opens it.
- In a page, B returns to the page list; B again returns to Play.
- The internal tab numbers, the rows and every settings action are unchanged.

### Touch
- Taps follow the new layout: sections, page list, rows (the value side acts as A), Launch,
  Diagnostic boot, the readiness cards (Recheck), the Setup button and + Exit.
- All the coordinates are constants shared through `launcher_ui.h`.
- Rebind Cancel uses `kCancel*`.

### Notices
- A notice clears after 6 seconds.
- It shows in the bottom bar, or in the side panel on Settings/Customize pages.

### Drawn with deko3d in the deko3d build
In the deko3d build, `launcher_graphics.cpp` `DekoSurface` replaces SDL + OpenGL.

How it works:
- It creates its own deko3d device, a two-image swapchain on the default `NWindow` (1280x720), and
  two staging buffers with fences.
- Each frame is copied by the copy engine (`dkCmdBufCopyBufferToImage`), with Full barriers around
  the copy, then presented.
- No shaders are involved.
- Its destructor waits for the queue to go idle, then destroys everything before the game opens
  dkgl on the same window.

Other builds:
- The OpenGL and Vulkan builds keep the OpenGL surface (`OPENRAC_SWITCH_DEKO3D_BUILD` selects which).
- `cmake/Games.cmake` adds the deko3d include path and library to the game target for the deko3d
  build.

## Verified locally
- **Host preview:** `launcher_canvas.cpp`, compiled on a PC, renders every page to PNG.
  - The PNGs and `preview.cpp` are in `build/claude-scratch/launcher-preview/`. Body text there is
    DejaVu Sans.
  - Checked: dark and light themes; with no art, with the included logo, and with a custom banner;
    the warning state; settings with the page list focused, a row focused, and scrolled; appearance
    swatches; saves; setup; about; progress; rebind.
- **Warnings:** `launcher_canvas.cpp` builds with no warnings (`-Wall -Wextra`).
- **Syntax checks** with libnx headers (host compiler with stubs):
  - `launcher_graphics.cpp`, with and without `OPENRAC_SWITCH_DEKO3D_BUILD`;
  - `launcher.cpp`, against the project headers.
- **Heading font:** the CMake step that embeds it was run with `cmake -P`; the bytes match the TTF.

## Needs the Switch
1. The build itself:
   - `build\deko3d-test\build-deko3d.bat`;
   - check `=== exit code 0`, the NRO target, and that the NRO and ELF copies match the build outputs;
   - add the ELF hash to `last-build.json`.
2. On the console, the launcher opens with the new look (deko3d build).
3. Then check, docked and handheld:
   - L / R, the page lists, B back;
   - touch;
   - a rebind;
   - Customize > Appearance colours;
   - an artwork download;
   - Launch into the game: the game must still start on deko3d after the launcher releases the
     window.
4. The first-launch preparation screen, if a fresh install is available.

## Revert
The previous launcher sources, `Window.cmake`, `Games.cmake` and `THIRD_PARTY_NOTICES.md` are in
`build/claude-scratch/backup-pre-launcher-redesign-2026-10-29/`. Restoring them, and removing
`port/platform/switch/fonts/`, returns the old launcher.
