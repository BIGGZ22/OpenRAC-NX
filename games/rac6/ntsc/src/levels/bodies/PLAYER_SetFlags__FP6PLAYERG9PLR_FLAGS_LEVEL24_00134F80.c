/* PLAYER_SetFlags__FP6PLAYERG9PLR_FLAGS_LEVEL24_00134F80 -- LEVEL24.REL 0x00134F80, 76 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   PLAYER_SetFlags__FP6PLAYERG9PLR_FLAGS -- LEVEL24.REL 0x00134F80, 76 bytes.
 *   The retail copies a 16-byte PLR_FLAGS into the player at 0x264.
 *   The retail body moves the two 64-bit halves of the flags with the
 *   unaligned pairs ldl/ldr and sdl/sdr (the source at 0x7/0x0 and 0xF/0x8
 *   of the flags pointer, the destination at 0x26B/0x264 and 0x273/0x26C
 *   of the player): the compiler does not know the alignment of either
 *   side, so the flags type is a packed 16-byte structure of two 64-bit
 *   fields.
 *   The value arrives by copy: the caller PLAYER_SetFlags__FG9PLR_FLAGS
 *   (LEVEL24.REL 0x00134FD0) copies the flags to its own frame and passes
 *   the address in $a1, which is how this ABI hands a 16-byte structure by
 *   value.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to
 *   the retail bytes of that placement, byte for byte.  The program-level
 *   gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct __attribute__((packed)) {
    long long low;
    long long high;
} PLR_FLAGS;

typedef struct __attribute__((packed)) {
    char pad[0x264];
    PLR_FLAGS flags;
} PLAYER;

void PLAYER_SetFlags__FP6PLAYERG9PLR_FLAGS(PLAYER *player, PLR_FLAGS flags)
{
    player->flags = flags;
}
