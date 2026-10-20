#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef int s32;

extern void func_31B920(void *entity, int a, int b);
extern int func_31B9F0(int id);
extern void func_320318(void *entity, void *target);
extern void func_2EEC20(void *target);
extern void func_326730(void *entity, void *target, int a, int b, float scale);
extern void func_312550(void *entity, int id, void *target);
extern void func_2EFD00(void *target, int a, float f);
extern float func_2F0048(float f0, float f1);
extern void func_324210(void *entity, void *target, int a, float f0, float f1);
extern u32 func_31CFF8(void *a, void *b, u32 c);
extern int func_3437D8(int index, void *a, void *out, float f);
extern void func_3A2F48(void *entry);
extern int func_31BA30(int a, int b);
extern float func_2F0148(int a);
extern void func_2FD088(int a, int b, float f);
extern void func_2E77F0(void *fn, void *entity);

/* The entity's per-frame sub-states; several are still INCLUDE_ASM markers
 * further down this file, hence the prototypes that repeat their definition. */
extern void func_L03_003F5B70(u8 *entity);
extern void func_L03_003F6158(u8 *entity);
extern void func_L03_003F64E8(u8 *entity);
extern void func_L03_003F6610(u8 *entity);
extern void func_L03_003F6748(u8 *entity);
extern void func_L03_003F6948(u8 *entity);
extern void func_L03_003F6BA8(u8 *entity);

/* `lui $v0,%hi ; lw $v0,%lo($v0)` in the retail: one register, no HIGH/LO_SUM
 * split, which is what the sda attribute (SYMBOL_REF_FLAG) gives in -G0. */
extern u8 *D_L03_001B2A1C __attribute__((sda));
extern u8 *D_L03_001FB5A0[];

/* One word of a stream: bits 0..14 name a record, bit 15 ends the walk. */
typedef struct {
    u16 raw;
} StreamEntry;

/* Measured partial layouts; the original types and game roles are unknown. */
typedef struct {
    u8 pad0[0x340];
    u32 entries[30];      /* 0x340 */
    u32 unknown3B8;       /* 0x3B8 */
    s32 count;            /* 0x3BC */
} LevelState;

typedef struct {
    u8 pad0[0x2DC];
    s32 current;          /* 0x2DC */
    u8 pad1[0x34];
    s32 table[1];         /* 0x314 */
} PickerState;

typedef struct {
    u32 field00;
    u8 pad04[0x0C];
    u8 field10[1];
} Entry41D1D8;

extern Entry41D1D8 *D_L03_001FE700[];

INCLUDE_ASM("asm/levels", func_L03_003F09C0);

INCLUDE_ASM("asm/levels", func_L03_003F11A0);

INCLUDE_ASM("asm/levels", func_L03_003F1470);

INCLUDE_ASM("asm/levels", func_L03_003F4638);

INCLUDE_ASM("asm/levels", func_L03_003F5590);

INCLUDE_ASM("asm/levels", func_L03_003F5B70);

/* ticks the entity's sub-states, feeds the 0x95 counter to the engine and
 * re-arms its 200-second timer */
void func_L03_003F6080(u8 *entity) {
    u8 *state = *(u8 **)(entity + 0x68);
    u32 timer;

    func_320318(entity, state + 0x70);
    func_L03_003F5B70(entity);
    func_L03_003F64E8(entity);
    func_L03_003F6610(entity);
    func_L03_003F6748(entity);
    func_L03_003F6158(entity);
    if (*(u8 *)(entity + 0x95) >= 2)
        func_2FD088(5, 0, *(float *)(state + 0x20) / func_2F0148(*(s16 *)(state + 0x24)));
    if (*(s32 *)(state + 0x3BC) != 0)
        func_2E77F0(func_L03_003F6948, entity);
    if (*(u8 *)(entity + 0x31) != 0)
        func_2E77F0(func_L03_003F6BA8, entity);
    timer = *(u16 *)(entity + 0x96);
    if (timer <= 0xFFFE) *(u16 *)(entity + 0x96) = (u16)(timer + 1);
}

INCLUDE_ASM("asm/levels", func_L03_003F6158);

/* func_L03_003F62C8 lives in L03_003F09C0-g8.c (it reaches D_L03_001B108C
 * through $gp). */

/* draws a value distinct from the state's current one and stores the table word it names */
void func_L03_003F6458(u8 *entity, s32 selector) {
    PickerState *state = *(PickerState **)(entity + 0x68);
    s32 value;

    do {
        if (selector == 1)
            value = func_31B9F0(3) * 2 + 1;
        else if (selector == 0)
            value = func_31B9F0(4) * 2;
        else
            value = func_31B9F0(7);
    } while (value == state->current);
    state->current = state->table[value];
}

INCLUDE_ASM("asm/levels", func_L03_003F64E8);

/* steps the entity's two blend channels and its rate */
void func_L03_003F6610(u8 *entity) {
    u8 *state = *(u8 **)(entity + 0x68);

    if (*(u8 *)(state + 0xD1) == 0) func_312550(entity, 7, state + 0xD0);
    func_2EFD00(state + 0xE0, 2, *(float *)(state + 0x338));
    *(float *)(state + 0x338) = func_2F0048(*(float *)(state + 0x338), 0.41597018f);
    func_324210(entity, state + 0x110, 0, 0.03f, 0.3f);
    func_324210(entity, state + 0x190, 1, 0.03f, 0.3f);
}

/* hands every record of the stream at slot 0xAC to func_3A2F48 */
void func_L03_003F66D8(u8 *entity) {
    u8 *state = *(u8 **)(entity + 0x68);
    s32 index = *(s32 *)(state + 0xAC);
    StreamEntry *entry;

    if (index == -1) return;
    entry = (StreamEntry *)D_L03_001FB5A0[index];
    if (entry == 0) return;
    do {
        func_3A2F48(D_L03_001B2A1C + ((entry->raw & 0x7FFF) << 8));
        if ((s16)entry->raw < 0) break;
        entry++;
    } while (1);
}

INCLUDE_ASM("asm/levels", func_L03_003F6748);

/* appends one word to the level state's 30-entry table */
void func_L03_003F68E8(u8 *entity, u32 value) {
    LevelState *state = *(LevelState **)(entity + 0x68);

    if (state->count < 30) {
        state->entries[state->count] = value;
        state->count = state->count + 1;
    }
}

void func_L03_003F6918(u8 *entity, s32 index) {
    LevelState *state = *(LevelState **)(entity + 0x68);

    state->entries[index] = state->entries[--state->count];
}

INCLUDE_ASM("asm/levels", func_L03_003F6948);

INCLUDE_ASM("asm/levels", func_L03_003F6BA8);

INCLUDE_ASM("asm/levels", func_L03_003F6E88);

INCLUDE_ASM("asm/levels", func_L03_003F71E0);

INCLUDE_ASM("asm/levels", func_L03_004198B0);

INCLUDE_ASM("asm/levels", func_L03_00419CA0);

INCLUDE_ASM("asm/levels", func_L03_0041A090);

INCLUDE_ASM("asm/levels", func_L03_0041A570);

INCLUDE_ASM("asm/levels", func_L03_0041BE50);

INCLUDE_ASM("asm/levels", func_L03_0041CA78);

INCLUDE_ASM("asm/levels", func_L03_0041CEB0);

/* func_L03_0041D0A8 lives in L03_003F09C0-g8.c (it reaches D_L03_001B2A1C
 * through $gp). */

/* func_L03_003F6918 has a near miss in drafts/L03_003F09C0-w29-nearmiss.c: the
 * body is byte-identical except that the two `addu` come out (index, base)
 * instead of the retail's (base, index). */

/* walks entity's stream, asking func_31BA30 for the value it stores at +0x2D8
 * of every record whose halfword at +0xAA is 0xFF3 */
void func_L03_0041D128(u8 *entity) {
    StreamEntry *entry;
    StreamEntry *p;
    u8 *record;

    if (entity[0x21] == 0xFF) return;
    entry = (StreamEntry *)D_L03_001FB5A0[entity[0x21]];
    if (entry == 0) return;
    p = entry;
    do {
        record = D_L03_001B2A1C + ((p->raw & 0x7FFF) << 8);
        if (*(signed char *)(record + 0x20) >= 0) {
            if (*(short *)(record + 0xAA) == 0xFF3) {
                u8 *owner = *(u8 **)(record + 0x68);

                *(int *)(owner + 0x2D8) = func_31BA30(0x28, 0x32);
            }
        }
        if ((s16)p->raw < 0) break;
        p++;
    } while (1);
}

/* asks the current entry to resolve; 1 when it consumed the query */
u32 func_L03_0041D1D8(u8 *entity) {
    u8 *state = *(u8 **)(entity + 0x68);
    u8 *sub = entity + 0x10;
    Entry41D1D8 *entry = D_L03_001FE700[*(s32 *)(state + 0x290)];
    u32 local[4];

    if (func_31CFF8(sub, entry->field10, entry->field00)) {
        if (func_3437D8(*(s32 *)(state + 0x290), sub, local,
                        *(float *)(state + 0x50)) == 0)
            return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L03_0041D258);

INCLUDE_ASM("asm/levels", func_L03_0041DAD0);

INCLUDE_ASM("asm/levels", func_L03_0041DCE0);

INCLUDE_ASM("asm/levels", func_L03_0041DFA8);

/* func_L03_0041E240 lives in L03_003F09C0-g8.c (same $gp access). */

INCLUDE_ASM("asm/levels", func_L03_0041E2D8);

INCLUDE_ASM("asm/levels", func_L03_0041EAE0);

/* re-arms the entity's two channel counters and its 200-second timer */
void func_L03_0041EF90(u8 *entity) {
    u8 *state = *(u8 **)(entity + 0x68);
    u32 timer;

    func_320318(entity, state + 0x50);
    func_2EEC20(state + 0x320);
    func_2EEC20(state + 0x322);
    timer = *(u16 *)(entity + 0x96);
    if (timer <= 0xFFFE) *(u16 *)(entity + 0x96) = (u16)(timer + 1);
    func_326730(entity, state + 0x200, 0, 1, 2.0f);
}

void func_L03_0041F008(u8 *entity, u32 value) {
    u8 *state = *(u8 **)(entity + 0x68);

    func_31B920(entity, 6, 2);
    *(u32 *)(state + 0x338) = value;
}

INCLUDE_ASM("asm/levels", func_L03_0041F048);

INCLUDE_ASM("asm/levels", func_L03_0041F090);

INCLUDE_ASM("asm/levels", func_L03_0041F178);

INCLUDE_ASM("asm/levels", func_L03_0041F410);
