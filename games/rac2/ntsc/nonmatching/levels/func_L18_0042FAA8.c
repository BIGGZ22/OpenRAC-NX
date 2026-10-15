/* Near miss from src/levels/18_damosel/L18_003F80A0-g8.c: the build refuses this body at its
   place — 60 bytes, expected 56. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_003F80A0-g8.c. */
typedef unsigned char u8;

extern char D_L18_001B1480[] __attribute__((sda));

extern char D_L18_001B1490[] __attribute__((nosda));

extern void func_L18_0042FAE0(u8 *a0, void *a1);

void func_L18_0042FAA8(u8 *a0) {
    func_L18_0042FAE0(a0, D_L18_001B1480);
    func_L18_0042FAE0(a0, D_L18_001B1490);
}
