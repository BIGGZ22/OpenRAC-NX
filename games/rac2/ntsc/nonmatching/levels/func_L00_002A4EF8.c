/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x30 differs
   The body is complete and the right length (60 words, 240 bytes) and every retail
   instruction is present. What differs is where this cc1 schedules the two index-clamp
   ops (`movz $s0,$a2,$v1` and `slt $v0,$v0,$s0`): retail keeps both right after the
   `slti`, before the 20-byte literal copy, this cc1 puts them after the copy. All 14
   differing words follow from that one placement (the `lh` base gets `$v1` instead of
   reusing `$s0`, the `blez` offset, and the copy/table-address order shift with it).
   Falsified levers, all giving the same 14 words: ternary against if-statement clamps,
   the clamped index inline in the subscript, nested ternary, `s16`/`unsigned` index,
   a second index variable, a pointer-dereference copy, `__asm__` aliases of
   D_001A7A18 and D_001A7F30, and the flag sets -fno-schedule-insns2 (13 words, first
   difference moves to +0x34), -fno-schedule-insns, -fno-delayed-branch and -O1.
   What would unblock it: a way to keep the clamp ops inside the `slti` block, or a
   scheduler-visible dependency from the literal copy to the clamp result. */

typedef unsigned char u8;
typedef short s16;
typedef int s32;
typedef float f32;

/* The resident player block: +0x22B4 is the control mode, +0x24CC the health. */
typedef struct {
    u8 pad00[0x22B4];
    u8 mode;
    u8 pad01[0x24CC - 0x22B5];
    s32 health;
} D00189E20;
extern D00189E20 D_00189E20;
/* The player's deferred nanotech XP, the nanotech level. */
extern s16 D_001A7A18;
/* Damage scale of each nanotech level, a resident literal. */
typedef struct {
    f32 scale[5];
} NanotechScale;
extern NanotechScale D_001A7F30;

extern f32 func_00283CE0(int value);
extern int func_00283CF0(f32 value);
extern void func_L00_0030C7F8(int argument);

/* Take AMOUNT off the player's health, scaled by his nanotech level. */
void func_L00_002A4EF8(int amount)
{
    u8 mode;
    int level;
    int damage;
    NanotechScale scale;

    if (amount == 0)
        return;
    damage = amount;
    mode = D_00189E20.mode;
    if (mode == 0 && amount > 0) {
        level = D_001A7A18;
        level = level < 5 ? level : 4;
        level = level > -1 ? level : 0;
        scale = D_001A7F30;
        damage = func_00283CF0(func_00283CE0(amount) * scale.scale[level] + 0.5f);
        damage = damage > mode ? damage : 1;
    }
    D_00189E20.health -= damage;
    if (D_00189E20.health < 0)
        D_00189E20.health = 0;
    func_L00_0030C7F8(360);
}
