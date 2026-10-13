/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The contract between a game's translated C and the port's runtime.
 *
 * The port keeps the game's memory where the game expects it: one 4 GB
 * reservation whose offsets are the PlayStation 2's own addresses. A pointer
 * in the game is a 32-bit offset into it (gaddr), the way OpenGOAL keeps
 * Jak's pointers ("OpenGOAL offsets", docs/port/DESIGN.md). Structures keep
 * their console layout, globals stay at their retail addresses, and an
 * address the game computes (an absolute address, a global plus an offset,
 * an uncached alias) means what it meant on the console.
 *
 * port/tools/hostgen writes C against this header: every pointer becomes a
 * gaddr, every access through one goes through G(), and a call through a
 * function pointer goes through GFN(). Hand-written host code uses the same
 * macros. This header is C (C11 with the GNU extensions both GCC and Clang
 * have), so the generated code is compiled as C, and C++ includes it too.
 */
#ifndef OPENRAC_GUEST_H
#define OPENRAC_GUEST_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* An address in game memory: what a pointer is in the game. */
typedef uint32_t gaddr;

/* The game's 128-bit integer (gcc's mode(TI)): quadword copies and MMI. */
__extension__ typedef __int128 openrac_s128;
__extension__ typedef unsigned __int128 openrac_u128;

/* ---- The memory map, in game addresses ----
 *
 * Main RAM and its two uncached aliases share one set of pages, as on the
 * console. The scratchpad is the EE's 16 KB of fast memory. The register
 * pages are plain memory: the port never models the hardware behind them;
 * where the game hands work to the hardware (a DMA send, a vsync wait), the
 * runtime's replacement of that library call takes over. The port's own
 * region holds what translated code needs that the console kept elsewhere
 * (locals whose address is taken, string literals). Everything else in the
 * reservation is inaccessible, so a stray access stops with its address. */
#define GUEST_RAM 0x00000000u
#define GUEST_RAM_SIZE 0x02000000u /* 32 MB */
#define GUEST_RAM_UNCACHED 0x20000000u
#define GUEST_RAM_UNCACHED_ACCEL 0x30000000u
#define GUEST_EE_REGS 0x10000000u
#define GUEST_EE_REGS_SIZE 0x00010000u
#define GUEST_VU_MEM 0x11000000u
#define GUEST_VU_MEM_SIZE 0x00010000u
#define GUEST_GS_REGS 0x12000000u
#define GUEST_GS_REGS_SIZE 0x00002000u
#define GUEST_SCRATCHPAD 0x70000000u
#define GUEST_SCRATCHPAD_SIZE 0x00004000u /* 16 KB */
#define GUEST_PORT 0xE0000000u
#define GUEST_PORT_SIZE 0x01000000u /* 16 MB */
#define GUEST_STACK_TOP (GUEST_PORT + 0x00100000u)
#define GUEST_STACK_BOTTOM GUEST_PORT
#define GUEST_STRINGS (GUEST_PORT + 0x00100000u)
#define GUEST_STRINGS_SIZE 0x00100000u
#define GUEST_STATICS (GUEST_PORT + 0x00200000u)
#define GUEST_STATICS_SIZE 0x00100000u

/* ---- Addresses ---- */

/* Where game memory starts in the host's address space. Set once by the
 * runtime before any game code runs. */
extern uint8_t* openrac_guest_base;

/* The host pointer for a game address, and an lvalue of type T there. */
#define G(a) ((void*)(openrac_guest_base + (gaddr)(a)))
#define GREF(T, a) (*(T*)G(a))

/* The game address of a host pointer into game memory. */
#define GADDR(p) ((gaddr)((const uint8_t*)(p) - openrac_guest_base))

/* The host compiler does not know that RAM and its uncached aliases are the
 * same memory: a write through one alias and a read through another in the
 * same function may be reordered or folded. Code that does that needs
 * GBARRIER() between the two (the game itself uses the aliases for what it
 * hands to the hardware, which the port reads after the call). */
#define GBARRIER() __asm__ __volatile__("" : : : "memory")

/* ---- Locals whose address is taken ----
 *
 * On the console such a local lives on the stack in game memory, and the
 * game passes its address around like any other pointer. Translated code
 * gives the function a frame on the game stack (GUEST_STACK_*): GFRAME(n)
 * reserves n bytes, aligned to 16, and gives them back when the function
 * returns, by whatever path. */
extern gaddr openrac_guest_sp;
gaddr openrac_guest_frame(uint32_t size);

static inline void openrac_guest_frame_end(gaddr* saved) {
    openrac_guest_sp = *saved;
}

#define GFRAME(name, size)                                                                         \
    gaddr name##_saved_sp __attribute__((cleanup(openrac_guest_frame_end))) = openrac_guest_sp;    \
    const gaddr name = openrac_guest_frame(size)

/* A static local whose address the game takes: storage in game memory for
 * the life of the program, zeroed (the port's statics region). */
gaddr openrac_guest_static(uint32_t size);

/* ---- String literals ----
 *
 * A literal in the game's C is data in game memory on the console. GSTR
 * copies it into the port's string region once, at its first use, and gives
 * its address. */
gaddr openrac_guest_string(const char* s, size_t size);
#define GSTR(lit)                                                                                  \
    __extension__({                                                                                \
        static gaddr gstr_cached_;                                                                 \
        if (gstr_cached_ == 0) {                                                                   \
            gstr_cached_ = openrac_guest_string((lit), sizeof(lit));                               \
        }                                                                                          \
        gstr_cached_;                                                                              \
    })

/* ---- Functions ----
 *
 * Function pointers in the game are code addresses: in its data (tables of
 * update functions, vtables) and in what it computes. GFN looks the address
 * up in the functions the build registered (port/runtime/guest.cpp) and
 * gives a host function pointer of type T. A level's program is loaded at
 * the same addresses as every other level's, so a code address means the
 * function of the level that is loaded (openrac_guest_set_overlay). */
typedef void (*openrac_host_fn)(void);

typedef struct openrac_fn_entry {
    gaddr address;
    openrac_host_fn fn;
    const char* name;
} openrac_fn_entry;

/* Overlay -1 is the executable; 0..n are the level programs. */
#define OPENRAC_OVERLAY_EXE (-1)
void openrac_guest_register(int overlay, const openrac_fn_entry* entries, size_t count);
void openrac_guest_set_overlay(int overlay);
int openrac_guest_overlay(void);
openrac_host_fn openrac_guest_function(gaddr address);
const char* openrac_guest_function_name(gaddr address);
#define GFN(T, a) ((T)openrac_guest_function(a))

/* Which overlay is loaded, asked at each call through a code address when
 * set: the game knows (rac1 keeps its current level in a global). */
void openrac_guest_set_overlay_source(int (*current)(void));

/* ---- Per-level relocation ----
 *
 * A level's program carries its own copy of much of the executable's code,
 * whose globals sit at other addresses (rac1: each level's copy of the engine).
 * The port has one host function for both, written with the executable's
 * addresses, so an executable function's global address goes through
 * OPENRAC_DATA and a function address it takes through OPENRAC_CODE: with a
 * level loaded, an address in the relocated range becomes that level's. The
 * tables are built when a level's code is first used, by pairing the address
 * operands of each function's executable code with its copy in the level's
 * code (the two differ only in addresses), so nothing of the game is stored. */
void openrac_guest_set_relocation(gaddr low, gaddr high, gaddr gp, const uint8_t* exe_code,
                                  gaddr exe_base, uint32_t exe_size);
extern gaddr openrac_relocate_low, openrac_relocate_high;
gaddr openrac_relocate_data(gaddr address);
gaddr openrac_relocate_code(gaddr address);
static inline gaddr OPENRAC_DATA(gaddr a) {
    return a - openrac_relocate_low < openrac_relocate_high - openrac_relocate_low
               ? openrac_relocate_data(a)
               : a;
}
static inline gaddr OPENRAC_CODE(gaddr a) {
    return openrac_relocate_high != 0 ? openrac_relocate_code(a) : a;
}

/* Of a function the catalogue folded (one name for tiny functions at several addresses of one
 * program), the loaded program's place nearest `caller`; places are (overlay + 1) << 24 |
 * address. A call through that place goes where its code calls (openrac_guest_function). */
gaddr openrac_guest_nearest(const uint32_t* places, int count, gaddr caller);
/* The place of a folded function a call goes to: the k-th call in the caller's code (at `caller`,
 * `size` bytes, in the program loaded) to any of `places`; openrac_guest_nearest's when the caller's
 * code has no such call (or its size is not known). */
gaddr openrac_folded_call(const uint32_t* places, int count, gaddr caller, uint32_t size, int k);

/* Level to level: a function several levels' programs carry is written with the addresses of the
 * level its name gives (rac1: func_L05_... has level 5's). OPENRAC_LDATA and OPENRAC_LCODE give
 * the loaded level's address for one of that level's; the tables come from pairing the function's
 * code in that level's program with its copy in the loaded one. The game says where a level's
 * program is (openrac_guest_set_level_programs: called with a level, it hands each piece of the
 * program over with openrac_guest_add_level_program, at the address the piece is loaded to). */
void openrac_guest_set_level_programs(void (*load)(int level));
void openrac_guest_add_level_program(int level, gaddr base, const uint8_t* bytes, uint32_t size);
gaddr openrac_relocate_level_data(int from, gaddr address);
gaddr openrac_relocate_level_code(int from, gaddr address);
#define OPENRAC_LDATA(from, a) openrac_relocate_level_data((from), (a))
#define OPENRAC_LCODE(from, a) openrac_relocate_level_code((from), (a))

/* One C function for copies of the same code at several places, each copy taking the address of
 * its own neighbour (rac1: a moby's update, the same code in each class, passing that class's
 * draw callback). The function asks at entry which place it was called at (openrac_guest_entry:
 * the place of the last call through a code address, when that call came to it; 0 otherwise), and
 * a code address it takes is read from that copy's own instructions: openrac_code_in_copy finds
 * where the copy at `canon` (in level `from`'s program, `size` bytes) forms `target`, and reads
 * the same instructions at `self`. Without a place, or one that does not form it, it is
 * OPENRAC_LCODE's. */
extern gaddr openrac_guest_last_entry;
extern openrac_host_fn openrac_guest_last_fn;
static inline gaddr openrac_guest_entry(openrac_host_fn self) {
    return openrac_guest_last_fn == self ? openrac_guest_last_entry : 0;
}
gaddr openrac_code_in_copy(int from, gaddr canon, uint32_t size, gaddr self, gaddr target);
/* The same for a call: the function the copy at `self` calls where the copy at `canon` calls
 * `target` (a small wrapper the catalogue folded, whose copies call different functions); `from`
 * -1 is the executable. Without a place, `target` (OPENRAC_CODE's or OPENRAC_LCODE's). */
gaddr openrac_call_in_copy(int from, gaddr canon, uint32_t size, gaddr self, gaddr target);

/* A call from the executable's own engine code to one of its empty hook stubs (stub.c: functions
 * that only return), which every level's program fills with a function of its own. The executable's
 * copy of the caller (`exe_fn`, `size` bytes) calls the stub at its k-th such call; the copy the
 * loaded level carries (the place of the caller in level `home`, `home_fn`, relocated to the loaded
 * program) calls that level's function at the same instruction. In the executable's own program,
 * or where the copy does not call at that place, it is the stub (OPENRAC_CODE's). */
gaddr openrac_hook_in_copy(gaddr exe_fn, uint32_t size, int home, gaddr home_fn, gaddr stub, uint32_t k);

/* The EE FPU's divide, for the game's float `/`: no infinities or NaNs. A divisor of zero (or a
 * denormal, which the EE reads as zero) gives +-FLT_MAX with the operands' signs XORed, and a
 * quotient too large saturates the same way, where the host would give infinity (and NaN from
 * there on: an animation loop that never ends). */
static inline float openrac_fdiv(float a, float b) {
    uint32_t ua, ub;
    memcpy(&ua, &a, 4);
    memcpy(&ub, &b, 4);
    if ((ub & 0x7F800000u) == 0) {
        return ((ua ^ ub) & 0x80000000u) != 0 ? -3.40282347e38f : 3.40282347e38f;
    }
    const float q = a / b;
    if (__builtin_isinf(q)) {
        return q < 0.0f ? -3.40282347e38f : 3.40282347e38f;
    }
    return q;
}

/* A function the game calls that has no C in the port yet: still assembly in
 * the decompilation, or a library the port has not replaced. Logged once per
 * function and counted (openrac_guest_report_missing at exit). While the
 * port is being brought up the program stops at the first one instead
 * (openrac_guest_stop_on_missing), so the log ends with what to do next. */
void openrac_guest_missing(const char* name);
void openrac_guest_report_missing(void);
void openrac_guest_stop_on_missing(int stop);

/* For the debugging tools: every call so far to a function without C (the
 * return value) and how many such functions there are; then each of them, in
 * the order of their names, with its calls (null past the last). The names
 * stay valid for the life of the program. */
unsigned long openrac_guest_missing_calls(unsigned* functions);
const char* openrac_guest_missing_at(unsigned index, unsigned long* calls);

/* ---- The host's stack, for the debugging aids ----
 *
 * Host code is built on Linux, macOS and Windows, and only the first two have
 * backtrace (execinfo.h): a debugging aid that names who called it asks here
 * instead. `text` gets the function `depth` calls above the caller of this one
 * (0: who called the caller) as the host can name it: its symbol on Linux and
 * macOS, the module and the offset in it on Windows (which a symbolizer
 * resolves with the program's PDB), "?" when the stack does not reach it. */
void openrac_host_caller(int depth, char* text, size_t size);

/* ---- What the game's own inline assembly did ----
 *
 * The decompilation keeps three quadword helpers as inline assembly
 * (games/rac1/pal/include/common.h): qcopy, qcopy_nc and qzero. */
static inline void openrac_qcopy(gaddr dst, gaddr src) {
    memmove(G(dst), G(src), 16);
}

static inline void openrac_qzero(gaddr p) {
    memset(G(p), 0, 16);
}

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_GUEST_H */
