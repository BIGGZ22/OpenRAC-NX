#include "include_asm.h"

/* Qwen draft (branch qwen/autonomy, 2e797515), checked by tools/check.py */
void func_0028A180(int, int);

extern void func_0028B650(char *, int);

/* Update the measured fields of the first matching key among13records. */
typedef struct { unsigned int fields[25]; int key; unsigned int busy; unsigned int tail[9]; } UpdateRecord;

extern UpdateRecord D_L00_0027DD00[13];

typedef struct {
    char pad0[0x8];
    int unk08;
    int *unk0C;
    char pad10[0x38];
    short unk48;
    short unk4A;
    char pad4C[0x4];
    int unk50;
    int unk54;
    int w;
    int h;
    int flags;
    char pad64[0x8];
    int unk6C;
    unsigned char cnt[4];
    int unk74;
    int unk78;
    int unk7C;
    void *unk80;
} HudElem;

typedef int s32;

typedef unsigned char u8;

typedef unsigned int u32;

void func_00288D88(void)
{
}

INCLUDE_ASM("asm/game", func_00288D90);

typedef struct { int active; char reserved04[0x20]; int key; int selection; char reserved2C[0x14]; int value40; int value44; } SelectorState;
typedef struct { char reserved00[0x24]; int key; char reserved28[0x40]; int active;
                short unk6C; char reserved6E[4]; unsigned short unk72; } SelectorOwner;
typedef struct { unsigned short key; char reserved02[10]; } SelectorRow;
extern SelectorState ref_L00_00288E18_00;
extern SelectorOwner D_001A63A8;
extern SelectorRow D_0018C958[];
int func_00288E18(int key, int selection)
{
    SelectorState *state = &ref_L00_00288E18_00;
    if (state->active == 0 && state->key == -1 && D_001A63A8.active == 0 &&
        D_001A63A8.key == -1 && D_0018C958[selection].key != 65535) {
        state->key = key;
        state->selection = selection;
        state->value40 = 0;
        state->value44 = 0;
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_00288E98);

typedef struct {
    int active;
    int timer;
    char pad08[0x18];
    int entryIndex;
    int unk24;
} SubtitleState;

typedef struct { int unk00; int unk04; int id; int unk0C; } TextEntry;

typedef struct { char pad00[0x2A20]; int unk2A20[1]; } DiscToc;

extern SubtitleState D_00254E78;
extern TextEntry *D_001B1840 __attribute__((sda));
extern DiscToc D_0014B540;

void func_00288EA0(void)
{
    int idx;

    if (D_00254E78.active == 0)
        return;
    if (D_00254E78.entryIndex != -1) {
        idx = D_001B1840[D_00254E78.entryIndex].id;
        if (idx != -1 && D_0014B540.unk2A20[idx] != 0 &&
            idx == D_001A63A8.unk6C - 6000) {
            if (D_001A63A8.unk72 != 6 && D_001A63A8.unk72 != 7)
                D_001A63A8.unk72 = 5;
        }
    }
    D_00254E78.active = 0;
    D_00254E78.unk24 = -1;
    D_00254E78.entryIndex = -1;
    D_00254E78.timer = 0;
}

INCLUDE_ASM("asm/game", func_00288F48);


INCLUDE_ASM("asm/game", func_00289018);

INCLUDE_ASM("asm/game", func_00289020);

INCLUDE_ASM("asm/game", func_00289178);

void func_00289AA0(void)
{
}

INCLUDE_ASM("asm/game", func_00289AA8);

extern short D_00254EC8[][2];

int func_00289FC8(short key, int field, unsigned short *out)
{
    int i;

    for (i = 0; i < 170; i++) {
        if (D_00254EC8[i][field] == key) {
            if (out != 0)
                *out = (unsigned short)D_00254EC8[i][field ? 0 : 1];
            return i;
        }
    }
    return -1;
}

INCLUDE_ASM("asm/game", func_0028A030);

void
func_0028A130(int param_1)
{
  if (param_1 == 0x17)
    func_0028A180(0x9f3, 0x4a);
  if (param_1 == 0x19)
    func_0028A180(0xa35, 0x8c);
  return;
}

INCLUDE_ASM("asm/game", func_0028A180);

INCLUDE_ASM("asm/game", func_0028A270);

INCLUDE_ASM("asm/game", func_0028A278);

INCLUDE_ASM("asm/game", func_0028A2F8);

INCLUDE_ASM("asm/game", func_0028A670);

INCLUDE_ASM("asm/game", func_0028AB18);

INCLUDE_ASM("asm/game", func_0028AB20);

INCLUDE_ASM("asm/game", func_0028AB70);

INCLUDE_ASM("asm/game", func_0028ACB0);

INCLUDE_ASM("asm/game", func_0028AE88);

INCLUDE_ASM("asm/game", func_0028AFE8);

INCLUDE_ASM("asm/game", func_0028B160);

extern char *D_001BAF3C;
extern char * volatile D_001B1880 __attribute__((sda));
extern char * volatile D_001B1884 __attribute__((sda));

void func_0028B288(void)
{
    char *p = D_001BAF3C;
    D_001B1880 = p;
    D_001B1884 = p + 0x64000;
}

char *func_0028B2B8(int size)
{
    char *end;
    char *cursor;

    if (D_001B1880 == 0)
        func_0028B288();
    end = D_001B1884;
    cursor = D_001B1880;
    if (end - cursor < size)
        return 0;
    size = (size + 15) & 0xFFFFFFF0u;
    D_001B1880 = cursor + size;
    return cursor;
}

/* func_0028B338 is in 00288D88-g8.c. */

INCLUDE_ASM("asm/game", func_0028B3D0);

/* Calls a callee with (this, word at +32), copies six words from +36..+56 down to +4..+24, calls the function pointer at +48 with this when non-zero, then clears +104. */
void func_0028B4D8(char *p)
{
    int fp;

    func_0028B650(p, *(int *)(p + 32));
    fp = *(int *)(p + 48);
    *(int *)(p + 4) = *(int *)(p + 36);
    *(int *)(p + 20) = *(int *)(p + 52);
    *(int *)(p + 24) = *(int *)(p + 56);
    *(int *)(p + 12) = *(int *)(p + 44);
    *(int *)(p + 8) = *(int *)(p + 40);
    *(int *)(p + 16) = fp;
    if (fp != 0)
        ((void (*)(char *))fp)(p);
    *(int *)(p + 104) = 0;
}

typedef struct { unsigned char pad00[0x64]; int key; unsigned char pad68[0x28]; } SearchRow_28B5D0;
extern SearchRow_28B5D0 D_00255330[];
extern void func_0028B3D0(int, int, int, int, int, int, int);

void func_0028B540(void)
{
    char *row = (char *)D_00255330;
    int *field = (int *)(row + 0x6c);
    int i = 0;
    do {
        func_0028B3D0(i, 0xffff, 0, 0, 0, 0, 1);
        i++;
        field[4] = 0;
        field[0] = -6;
        func_0028B4D8(row);
        row += 0x90;
        field = (int *)((char *)field + 0x90);
    } while (i < 13);
}

int func_0028B5D0(int key)
{
    int i;
    for (i = 0; i < 13; i++) {
        if (D_00255330[i].key == key)
            break;
    }
    if (i < 13) {
        func_0028B3D0(i, 0xffff, 0, 0, 0, 0, 0);
        return 1;
    }
    return 0;
}


INCLUDE_ASM("asm/game", func_0028B6C0);

/* findfirstmatchingkey among13records and writefield24 plusfield04 whenword68clear */
void func_0028B6C8(int key, unsigned int value)
{
    int i;
    for (i = 0; i < 13; i++) {
        if (D_L00_0027DD00[i].key == key) break;
    }
    if (i < 13) {
        D_L00_0027DD00[i].fields[9] = value;
        if (D_L00_0027DD00[i].busy == 0)
            D_L00_0027DD00[i].fields[1] = value;
    }
}

int func_0028B740(HudElem *rec, int *x, int *y) {
    int w = rec->w;
    int h = rec->h;
    int flags = rec->flags;

    if ((flags ^ 1) & 1) {
        if (!(flags & 2)) {
            *y -= h >> 1;
        }
    }
    if (!(rec->flags & 4)) {
        if (rec->flags & 8) {
            *x -= w;
        } else {
            *x -= w >> 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_0028B7A8);

void func_0028B950(u8 *a0) {
    s32 *value=*(s32 **)(a0+0xc);
    s32 digits;
    s32 n;
    if (value!=0 && ((u32)value&3)==0) {
        s32 initial=*value;
        s32 maximum=*(s32 *)(a0+8);
        *(volatile s32 *)(a0+0x78)=initial;
        if (maximum<initial) *(volatile s32 *)(a0+0x78)=maximum;
        *(s32 *)(a0+0x74)=*(volatile s32 *)(a0+0x78);
    } else {
        *(volatile s32 *)(a0+0x74)=99999;
        *(volatile s32 *)(a0+0x78)=99999;
    }
    digits=0;
    for (n=*(volatile s32 *)(a0+8);n>9;n/=10) digits++;
    if ((*(u32 *)(a0+0x60)&3)==0 && (*(u32 *)(a0+0x60)&12)!=0) {
        *(s32 *)(a0+0x5c)+=(digits+1)*12;
        if (*(volatile s32 *)(a0+0x58)<14) { *(s32 *)(a0+0x58)=14; return; }
    } else {
        if (*(s32 *)(a0+0x5c)<12) *(s32 *)(a0+0x5c)=12;
        *(s32 *)(a0+0x58)=*(volatile s32 *)(a0+0x58)+(digits+1)*14;
    }
}

void func_0028BA50(HudElem *a0) { a0->unk7C = 0xD2; a0->unk48 = 0; a0->unk4A = 0; func_0028B950((u8 *)a0); }

INCLUDE_ASM("asm/game", func_0028BA88);

INCLUDE_ASM("asm/game", func_0028BC58);

extern void func_0029CCD8(int, int);
extern void func_002B0CE0(int, int, int);

void func_0028BCA0(int arg) {
    func_0029CCD8(arg, 1);
    func_002B0CE0(13, 0, 1);
}

INCLUDE_ASM("asm/game", func_0028BCD0);

INCLUDE_ASM("asm/game", func_0028BCE8);
