/* from src/levels/shared/L00_002F4400.c: arms the 0x3C-tick timer once (f38 = 1, f3C =
 * 0x3C, func_00288D90), then raises f0 to 8. Best of 4 compiles (2 profiles): retail's
 * two guards are `bnel v0,$zero,<epilogue>` with `ld s0,0(sp)` in the annulled slot -
 * the branch-likely form that lets the s0 restore sit there and keeps s0 = the %hi of
 * D_L00_0027D848 live across the call. This cc1 emits a plain `bnez` and fills the slot
 * from the fall-through (an extra `lw v0,%lo(s0)`), 92 bytes against retail's 88.
 * `-mbranch-likely` is not an option of the pinned cc1 (`Invalid option`), and the sdk
 * profile gives the right 88 bytes but a 0x20 frame against retail's 0x10. */
typedef struct {
    int f0;
    char pad_04[0x34];
    short f38;
    char pad_3A[2];
    int f3C;
} Entry_r002F46C8;

extern Entry_r002F46C8 D_L00_0027D848[];
extern void func_00288D90(void);

void func_L00_002F46C8(void) {
    if (D_L00_0027D848[0].f38 == 0) {
        D_L00_0027D848[0].f38 = 1;
        D_L00_0027D848[0].f3C = 0x3C;
        func_00288D90();
    }
    if (D_L00_0027D848[0].f0 == 0)
        D_L00_0027D848[0].f0 = 8;
}
