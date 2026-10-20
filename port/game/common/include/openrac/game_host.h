/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Between a game's program (main.cpp), the library replacements every game
 * shares (lib/) and what hostgen generates for each game (game_info.c,
 * libraries.c, port/tools/hostgen).
 *
 * The replacements are C, written against guest.h like the translated game:
 * their arguments are the game's values and addresses. They implement the
 * libraries' APIs as the games use them (libraries.tsv lists each one), from
 * what the game's own code shows and what is publicly known of each API;
 * nothing of Sony's is in them. Below them, the port never models the
 * console: a read is a read of the extracted disc image, a memory card is a
 * folder, a frame ends where the game waits for vsync, and the game's display
 * list goes to the port's renderer as data.
 */
#ifndef OPENRAC_GAME_HOST_H
#define OPENRAC_GAME_HOST_H

#include "openrac/guest.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- The game, from its hostgen.json (generated game_info.c) ---- */

typedef struct openrac_game_info {
    const char* id;     /* "rac1-pal": port/game/<id>, the program openrac-<id> */
    const char* title;  /* "Ratchet & Clank (PAL)" */
    const char* game;   /* "rac1": the extractor's and the launcher's name for it */
    const char* serial; /* "SCES_509.16": its executable on the disc */
    int frame_rate;     /* 50 (PAL) or 60 */
    const char* entry;  /* the game's main, or NULL while it is not known */
} openrac_game_info;

extern const openrac_game_info openrac_game;

/* Runs the game's entry (its main). Returns false if the game has none yet. */
int openrac_game_run(void);

/* The level program that is loaded, or OPENRAC_OVERLAY_EXE: which overlay a
 * code address means. Generated as "the executable" unless the game's
 * hostgen.json says the game provides it ("overlay_hook", in host/). */
int openrac_game_loaded_overlay(void);

/* Every function by code address (generated functions.c). */
void openrac_game_register_functions(void);

/* ---- Set by the program before the game starts ---- */

/* The player's disc image, as the extractor kept it (iso_data/<game>/disc.iso).
 * The game reads it by sector. */
extern const char* openrac_game_disc_image;

/* A folder of edited levels (--mods), or NULL: its patches.tsv lists the disc
 * sectors that are read from files beside it instead (editor/pack.py writes
 * both). The disc image itself is never changed. */
extern const char* openrac_game_mods_dir;

/* The first memory card's folder (the second is beside it, <folder>.slot2). */
extern const char* openrac_game_card_dir;

/* The console's language setting (sceScfGetLanguage): 1 English, 2 French,
 * 3 Spanish, 4 German, 5 Italian. */
extern int openrac_game_language;

/* ---- Provided by the program, called by the replacements ---- */

/* The game finished a frame and waits for vsync (sceGsSyncV): present what
 * the renderer has, pump input, keep the frame rate. Returns the field the
 * console would report (0 or 1, alternating). */
int openrac_game_vsync(void);

/* Called by the window's frame loop with the frame number, after each vertical blank, when a game
 * installs it (rac1: the direct start into a level, boot.c). */
extern void (*openrac_game_on_frame)(unsigned frame);

/* The player's bolts, when a game installs it (rac1/pal: boot.c): logged while a level plays for
 * the launcher's Discord presence. */
extern int (*openrac_game_bolts)(void);

/* The buttons a pressed Start also presses while Skip cutscenes is on (settings skip_cutscenes), when
 * a game installs it: what the game's own skip of the scene playing now takes, as the window's pad
 * bits (0x4000 Cross, 0x0F00 the four shoulders), or 0 while nothing skippable plays (rac1: boot.c,
 * the shoulders in a cutscene, Cross in the ship's landing and take-off). */
extern int (*openrac_game_skip_buttons)(void);

/* For crash reports (port/common/crash.h), sampled once a frame, when a game installs them (rac1:
 * boot.c): the game's mode word, where the hero is and where the camera is. Each returns 0 when it
 * has no answer now (no level loaded). */
extern int (*openrac_game_mode)(uint32_t* mode);
extern int (*openrac_game_hero_position)(float xyz[3]);
extern int (*openrac_game_camera_position)(float xyz[3]);

/* For crash reports: the moby whose update was running when the program stopped. */
typedef struct openrac_game_moby {
    uint32_t address; /* in game memory */
    int32_t oclass;   /* its class */
    float position[3];
    int32_t uid; /* its id in the level's placements, or -1 */
} openrac_game_moby;

/* Asked while the report is written, not each frame: from a signal handler, so it only reads game
 * memory it has checked is there, and allocates nothing. 1 and `out` filled when a moby's update
 * was running (rac1: boot.c's func_00213C78, the moby update loop), 0 otherwise. */
extern int (*openrac_game_updating_moby)(openrac_game_moby* out);

/* For OPENRAC_TRACE (frontend.cpp), when a game installs it (rac1: boot.c): the hero's position,
 * rotation (game axes; z the yaw) and state id. 0 when there is none now (no level loaded). */
extern int (*openrac_game_hero)(float position[3], float rotation[3], int32_t* state);

/* A trap the game raised on purpose, or another reason the game cannot go on: writes a crash
 * report (<data>/crashes) and ends the program. */
#ifdef __cplusplus
[[noreturn]]
#else
_Noreturn
#endif
void openrac_game_crash(const char* reason);

/* A warning from the game's host C, as one line of the port's log (and so of the session's file a
 * player sends): something the port did instead of what the console would have (a crash the
 * console shares, skipped). */
void openrac_game_warn(const char* message);

/* Called while Start is held with skip_cutscenes on, when a game installs it: skips what the game
 * lets the port skip besides its own cutscenes (rac1/pal: the flight between planets, boot.c). */
extern void (*openrac_game_skip)(void);

/* What the window's own menus and drawing need of a game's front end, when a game installs it
 * (rac1: port/game/rac1/host/front_end.c, with each disc's addresses). NULL: the window adds
 * nothing to the game's menus. */
typedef struct openrac_front_end {
    /* The developer menu (debug_menu.cpp): the levels, by the game's own numbers, and a new game
     * started straight in one of them, as --level N with OPENRAC_DIRECT does. */
    int levels;
    const char* const* level_names;
    void (*start_level)(int level);
    /* The menus' sound for a widget (rac1: the confirm sound, 0x11, from the widget's moby). */
    void (*menu_sound)(int sound, gaddr moby);
    /* The front end's page menu, data in the boot program (rac1: debug_menu.cpp describes it). */
    gaddr page_menu;      /* +4 the page shown */
    gaddr options_list;   /* the Options list widget: +0x34 its items, 12 bytes each */
    gaddr options_items;  /* the items the game gives it, ended by one without a label */
    gaddr video_item;     /* the item that opens the Video page, added where the game's lack it */
    gaddr language_page, language_title, language_list; /* what the Planets page is copied from */
    int16_t planets_text;                                /* "Planets" */
    const int16_t* planet_texts;                         /* each level's planet name, by level */
    gaddr video_page, video_list, video_rows; /* the Video options page, its list, its own rows */
    gaddr sound_hint;                         /* the Sound options page's hint widget */
    int32_t sound_list_seq, sound_hint_seq;   /* that page's frame animations */
    gaddr pad;        /* the first pad: +0x1C4 the buttons pressed this tick, +0x100 the sticks */
    gaddr video_mode; /* a byte: the game's own 50/60 Hz choice, kept at 60 Hz; 0 for none */
    /* The window (frontend.cpp): words of the boot program and the executable. */
    gaddr game_mode;  /* the game mode; 6 in the boot program is the flight between planets */
    gaddr light_bank; /* the directional light bank (relocated to the loaded program's copy) */
    gaddr level;      /* the level being played, or -1 */
    gaddr dialog;     /* the front end's dialog: +0 its kind, +0x1C its step (logged) */
    gaddr title_exit; /* the title loop's exit (logged) */
    gaddr ship_state; /* the ship's sequence word: 4 the flight through space (the flight world),
                         others drawn on the planet (0 take-off, 8 landing, 3 and 7 leaving it);
                         0 for a game without one */
} openrac_front_end;

extern const openrac_front_end* openrac_game_front_end;

/* The game sends a DMA chain (sceDmaSend): channel is the game address of
 * the channel's registers (0x10009000 is VIF1, the display list; 0x1000A000
 * is the GIF), tag the address of the chain's first tag. The renderer reads
 * it as data, the way OpenGOAL reads Jak's DMA chains. */
void openrac_game_dma_send(gaddr channel, gaddr tag);

/* The display and drawing environment, decoded from the GS register values
 * the game puts (sceGsPutDispEnv, sceGsPutDrawEnv). */
typedef struct openrac_game_display {
    int width, height; /* the frame buffer shown, in pixels */
    int frame_base;    /* its address in the GS's memory, in 2048-word pages */
    int frame_width;   /* in 64-pixel units */
    int psm;           /* its pixel format */
    int x, y;          /* where the console put it on the screen */
} openrac_game_display;

void openrac_game_set_display(const openrac_game_display* display);
void openrac_game_set_video_mode(int interlace, int mode, int field_mode);

/* An image the game uploads to the GS's memory (sceGsExecLoadImage): its
 * destination and the game address of its pixels. The renderer turns it
 * into a texture (port/renderer, TexturePool); nothing models GS memory. */
typedef struct openrac_game_image {
    int base, width_units,
        psm; /* destination: base (64-word blocks), width (64-pixel units), format */
    int x, y, width, height;
    gaddr pixels;
} openrac_game_image;

void openrac_game_load_image(const openrac_game_image* image);

/* The game reading a rectangle of the GS's memory back (sceGsExecStoreImage):
 * the source, and the game address the pixels go to. The renderer answers
 * reads of the frame buffer with the frame it drew. */
void openrac_game_store_image(const openrac_game_image* image);

/* The game's own renderers were asked to draw these layers this frame (bits: 1 sky, 2 terrain,
 * 4 ties, 8 shrubs, 16 mobys; renderer::Bucket's order). Its renderer entry points call this in
 * the port, where they hand over to the port's renderers: the window draws a layer only in a
 * frame the game drew it (no world behind a loading card or a menu over black). A game that never
 * calls it gets its world drawn whenever it has a camera. */
#define OPENRAC_DRAW_SKY 1u
#define OPENRAC_DRAW_TERRAIN 2u
#define OPENRAC_DRAW_TIES 4u
#define OPENRAC_DRAW_SHRUBS 8u
#define OPENRAC_DRAW_MOBYS 16u
void openrac_game_draw(unsigned layers);

/* The moby at `moby`'s current pose, for the game's own use: each joint marked in `marks` (128
 * bytes: nonzero for a joint to evaluate, byte 0x7F the joint count) gets its pose matrix, four rows
 * of four floats, in the scratchpad at 0x70000000 + 0x40 * joint, as the game's hand-written chain
 * evaluator leaves it. Nothing without the window's evaluator. */
void openrac_game_moby_chain(gaddr moby, gaddr marks);

/* The game's moby renderer was asked to draw `count` mobys from `first` (0x100 bytes each; count
 * -1: the whole list, the world's draw). A list drawn on its own (the page menu's frame objects)
 * may be drawn with a camera of its own: the window draws those mobys as that camera saw them.
 * Nonzero: the caller puts a moby marker in the 2D list where the game's packets would have gone
 * (the renderer's kMobyMarker), so that mobys drawn into a texture are drawn there in order. */
int openrac_game_mobys_drawn(gaddr first, int count);

/* A textured quad the game draws in the world (rac1: func_001F7EF8, the draw callbacks' quad
 * routine): its corners in world space (x, y, z, w), the strip 0 1 2, 1 2 3; RGBA bytes and
 * normalised ST per corner; the GS registers CLAMP_1, TEX0_1, TEX1_1 and ALPHA_1 it draws with;
 * and, for an effect texture, where its pixels are.
 * The window draws the frame's quads after the mobys (renderer/effects.h). */
typedef struct openrac_game_quad {
    float corner[4][4];
    uint32_t rgba[4];
    float st[4][2];
    uint64_t clamp, tex0, tex1, alpha;
    /* Where the texture's pixels (PSMT8) and CLUT (16 x 16 PSMCT32) are in the game's memory, as
     * its effect texture paging sends them to TEX0's blocks; 0 when the texture is already in the
     * GS's memory some other way. */
    uint32_t pixels, clut;
} openrac_game_quad;

void openrac_game_effect_quad(const openrac_game_quad* quad);

/* A sky shell the game drew this frame (rac1: SkyDrawShell) and the matrix it was drawn with: 16
 * floats, the game's rows (x, y, z axes, then the translation, in the sky's own units), each row
 * one column of the window's column-major matrix. */
void openrac_game_sky_shell(int shell, const float* matrix);

/* A textured quad of the 2D layer (rac1: DrawTexturedQuad, the HUD's sprite routines): four corners
 * in strip order (0-1-2, 1-2-3) in the chip's 12.4 screen coordinates as the game computes them,
 * their texel coordinates (12.4 both ways), the depth, the colour (RGBAQ's low word) and the
 * texture (TEX0). The window draws it natively: the index to put in the display list's marker, or
 * -1 to send the game's own packet (no window, or the 2D layer replayed). */
typedef struct openrac_game_ui_rect {
    uint32_t x[4], y[4];
    uint32_t u[4], v[4];
    uint32_t z;
    uint32_t rgba;
    uint64_t tex0;
    uint32_t flags; /* 1: a glyph of text; 2: sent as a sprite (corners 0 and 3) */
} openrac_game_ui_rect;

int openrac_game_ui_quad(const openrac_game_ui_rect* quad);

/* How far, in the game's screen pixels (512 across its 4:3 picture), the HUD's elements anchored to
 * a side of the screen are moved out past that side: the width a wider picture gained there when
 * the player's HUD setting is "Edges", else 0. */
int openrac_game_hud_side(void);

/* How many times wider than the game's 4:3 the picture is (the player's aspect setting): its
 * horizontal tangent is the game's times this. 1 for a 4:3 picture, and without a window. */
float openrac_game_picture_wider(void);

/* How the player strafes on foot (settings strafe; rac1: host/strafe.c): 0 the game's own turning,
 * 1 hold L2 or R2: the camera swings behind him once, then he faces the camera's way, 2 that and,
 * with a forward-firing weapon in hand, always facing the camera's way. 0 without a window (no
 * settings). */
int openrac_game_strafe_mode(void);

/* A live particle the game's particle renderer would draw (rac1: PartProc): its 0x40-byte record
 * (+0x01 bits 0-1 the render kind: 0 camera-facing sprite, 1 flat quad, 2 line, 3 ribbon; +0x03
 * ALPHA_1's low byte; +0x04 RGBA; +0x08 rotation, 256 steps a turn; +0x09 near (low nibble, 1/4
 * units) and far (high nibble, 32 units); +0x0C size in 1/210000 units, or the second colour of a
 * line or ribbon; +0x10 position, +0x1C a ribbon's half width; +0x20 a line or ribbon's other end,
 * +0x2C its width factor) and where its texture is in the game's memory (32 x 32 PSMT8 pixels,
 * 16 x 16 PSMCT32 CLUT; 0 for none). The window draws the frame's particles after the effect quads,
 * back to front (renderer/effects.h). */
void openrac_game_particle(const uint8_t* record, uint32_t pixels, uint32_t clut, int log2_side);

/* A sky sprite (rac1: SkySpriteProc, the stars and glows between the sky shells): its 0x20-byte
 * record (+0x03 ALPHA_1's low byte, +0x04 RGBA, +0x08 rotation in radians, +0x10 position around
 * the eye, +0x1C size) and the TEX0 of its sky texture. The window draws them behind the world. */
void openrac_game_sky_sprite(const uint8_t* record, uint64_t tex0);

/* Plays a PSS movie from the disc in the window, blocking as the game's own player does: `bytes`
 * bytes at sector `lsn`, the ADPCM channel `channel` (the language; channel 0 when the file has no
 * such channel). Start skips it when `start_skips` (the console's readMpeg rule for the caller).
 * Returns 1 if skipped, 0 if played to the end or when there is no window to play it in. */
int openrac_game_play_movie(uint32_t lsn, uint32_t bytes, int channel, int start_skips);

/* The buttons of a pad, in the console's bit order (active low, as the pad
 * library reports them) and its analog values; false if none is connected. */
/* Ratchet & Clank's blend snapshot: moby `moby`'s current pose (its keys A and B and the blend
 * between them) written at `dst` as a keyframe, as the game's VU0 routine leaves it in a blend slot.
 * Nonzero when written. */
int openrac_game_moby_snapshot(gaddr moby, gaddr dst);

/* Nonzero: no memory card is inserted in either port (--no-card). */
extern int openrac_game_no_card;

/* --level N: the level a new game starts in (the game's own number), or -1 for the game's own
 * first level. For getting to a level directly, as the behaviour checker's level write does. */
extern int openrac_game_start_level;

int openrac_game_pad(int port, uint16_t* buttons, uint8_t analog[4]);

/* The game's text lookup, asked for a text id: the game address of a string of the port's own (the
 * rows it adds to the game's options), or 0 when the id is the game's. */
gaddr openrac_game_text(int id);

/* ---- Inside the replacements ---- */

/* Interrupt handlers the game installed (AddIntcHandler, sceGsSyncVCallback,
 * AddDmacHandler): called by the replacements where the console would raise
 * the interrupt. */
void openrac_game_run_vsync_handlers(void);
void openrac_game_run_dmac_handlers(int channel);

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_GAME_HOST_H */
