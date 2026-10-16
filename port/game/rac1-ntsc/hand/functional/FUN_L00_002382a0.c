/* Carried from the PAL decompilation's src/overlays/shared/hud_00235960.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
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
} HudElem_c2382a0;
int func_L00_00235A70(HudElem_c2382a0 *rec, int *x, int *y);
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
typedef struct {
    char pad_00[0x38];
    unsigned short x38;
    char pad_3A[0x4C - 0x3A];
} SrcElem_c2382a0;
extern int D_L00_0015FA90 MACRO_ADDR;
extern void *D_L00_0015FAC0 MACRO_ADDR;
extern SrcElem_c2382a0 D_L00_00179AC0[];
extern short D_L00_0015F718;
extern short D_L00_0015F71C;
extern short D_L00_0015F72C;
extern int D_0014171B_i[] __asm__("D_0014161B");
extern short D_L00_0015F780;
typedef struct { int id; char pad[0x14]; int item; } E_238C30_c2382a0;
typedef struct { char pad[0x46]; short h46, h48; char pad4A[2]; } TI_238C30_c2382a0;
extern int func_001FF960(int, int);
extern void func_001FFC30(int, int, int, int, int, int);
extern void func_L00_0023B120(char *, int, int, int, int, int);
extern int func_00116248(char *, const char *, ...);
extern int func_001FA6E0(int, int, float);
extern float func_001FA580(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern int func_001F6AF0(int, int, long, void *, int);
extern void * func_001FDD10(int);
extern int func_001166CC(char *);
extern unsigned char D_0013E05A[];
extern short D_L00_0015F7B0;
extern short D_L00_0015F78C;
extern short D_L00_0015F7C4;
extern short D_L00_0015F7C0;
extern short D_L00_0015F720;
extern short D_L00_0015F724;
extern short D_L00_0015F790;
extern short D_L00_0015F794;
extern short D_L00_0015F798;
extern short D_L00_0015F79C;
extern short D_L00_0015F7A0;
extern short D_L00_0015F7A4;
extern short D_L00_0015F7A8;
extern short D_L00_0015F7AC;
extern short D_L00_0015F7B4;
extern short D_L00_0015F7B8;
extern short D_L00_0015F7BC;
extern int D_L00_0015F5CC MACRO_ADDR;
extern char D_L00_0015F7C8[];
extern char D_0013D407[];
#define GI(x) (*(int *)&(x))
#define GF(x) (*(float *)&(x))


int FUN_L00_002382a0(HudElem_c2382a0 *rec) {
    extern char D_L00_001C40B0[];
    char *wr;
    int x, y, half_w, half_h, alpha;
    int marker_alpha;
    int i, icon_x, icon_y, selected;
    int entry_offset, marker_offset, normal_offset;
    int color, shadow, text_y, id, item, prefix_length, text_length, j;
    float open, fade, angle;
    const char *text, *suffix;
    char *entry;
    unsigned char *q = rec->cnt;
    char formatted[80];

    open = func_001FA6C0(q[0]) / func_001FA6C0(GI(D_L00_0015F780));
    if (1.0f < open) open = 1.0f;
    else if (open < 0.0f) open = 0.0f;
    fade = func_001FA6C0(q[1]) / func_001FA6C0(GI(D_L00_0015F780));
    if (1.0f < fade) fade = 1.0f;
    else if (fade < 0.0f) fade = 0.0f;
    {
        float phase, angle, radius;
        phase = func_001FA6C0(D_L00_0015F5CC % GI(D_L00_0015F7B0));
        phase /= func_001FA6C0(GI(D_L00_0015F7B0));
        phase *= 6.28318f;
        angle = func_001F9DE0(phase - 3.14159f);
        radius = open * 128.0f;
        alpha = func_001FA6D0(radius);
        angle *= 0.125f;
        angle += 0.875f;
        marker_alpha = func_001FA6D0(radius * angle);
    }
    x = rec->unk50;
    y = rec->unk54;
    func_L00_00235A70(rec, &x, &y);
    half_w = GI(D_L00_0015F718) / 2;
    half_h = GI(D_L00_0015F71C) / 2;
    func_001FFC30(func_001FF960(0xe934, 0), x, y, half_w, half_h, alpha);
    func_001FFC30(func_001FF960(0xe934, 0), x + GI(D_L00_0015F718),
                  y + GI(D_L00_0015F71C), -half_w, -half_h, alpha);
    func_001FFC30(func_001FF960(0xe934, 0), x + GI(D_L00_0015F718),
                  y, -half_w, half_h, alpha);
    func_001FFC30(func_001FF960(0xe934, 0), x,
                  y + GI(D_L00_0015F71C), half_w, -half_h, alpha);
    for (i = 0; i < D_L00_0015FA90; i++) {
        angle = func_001FA580(((float)i + (float)i) * 3.1415927f /
                              (float)D_L00_0015FA90 - 3.1415927f, 1.5707964f);
        icon_x = x + GI(D_L00_0015F720) +
                 (int)(GF(D_L00_0015F78C) * func_001F9DC8(angle) * GF(D_L00_0015F7C4));
        icon_y = y + GI(D_L00_0015F724) +
                 (int)(GF(D_L00_0015F78C) * func_001F9DE0(angle));
        if (rec->unk74 == i && ((E_238C30_c2382a0 **)D_L00_0015FAC0)[GI(D_L00_0015F72C)][i].id != 0)
            func_001FFC30(func_001FF960(0xe99c, 0), icon_x - 0x13,
                          icon_y - 0x13, 0x26, 0x26, marker_alpha);
        if (((E_238C30_c2382a0 **)D_L00_0015FAC0)[GI(D_L00_0015F72C)][i].id != 0) {
            int icon_alpha;
            if (i != rec->unk74)
                icon_alpha = func_001FA6D0((float)alpha * GF(D_L00_0015F7C0));
            else
                icon_alpha = alpha;
            func_L00_0023B120((char *)rec, func_001FF960(((E_238C30_c2382a0 **)D_L00_0015FAC0)[GI(D_L00_0015F72C)][i].id,
                          ((unsigned char *)D_0013E05A + 0x4C6)[((E_238C30_c2382a0 **)D_L00_0015FAC0)[GI(D_L00_0015F72C)][i].item] ? 4 : 0), icon_x, icon_y, 1, icon_alpha);
        }
    }
    selected = rec->unk74;
    if (selected >= 0 && *(int *)(((char **)D_L00_0015FAC0)[GI(D_L00_0015F72C)] + selected * 0x1C) != 0) {
        color = func_001FA6E0(GI(D_L00_0015F798), GI(D_L00_0015F79C), open * fade);
        {
        int id;
        id = *(int *)((char *)D_0014171B_i + 0x885 + selected * 4);
        wr = D_L00_001C40B0 + id * 24;
        if (*(unsigned short *)(wr + 8) != 0) {
            int shadow, item;
            item = *(int *)(D_0013D407 + 0x21 + id * 4);
            func_00116248(formatted, D_L00_0015F7C8, item, *(unsigned short *)(wr + 0xE));
            if (item != 0)
                shadow = func_001FA6E0(GI(D_L00_0015F7A8), GI(D_L00_0015F7AC), open * fade);
            else
                shadow = func_001FA6E0(GI(D_L00_0015F7A0), GI(D_L00_0015F7A4), open * fade);
            func_001F6AF0(x + GI(D_L00_0015F720) + 1, y + GI(D_L00_0015F724) + 13, color, formatted, -1);
            func_001F6AF0(x + GI(D_L00_0015F720), y + GI(D_L00_0015F724) + 12, shadow, formatted, -1);
        }
        }
        prefix_length = 0;
        shadow = func_001FA6E0(GI(D_L00_0015F790), GI(D_L00_0015F794), open * fade);
        {
            int item, id;
            item = *(int *)(((char **)D_L00_0015FAC0)[GI(D_L00_0015F72C)] + selected * 0x1C + 0x18);
            if (((unsigned char *)D_0013E05A + 0x4C6)[item])
                id = ((TI_238C30_c2382a0 *)D_L00_00179AC0)[item].h48;
            else
                id = ((TI_238C30_c2382a0 *)D_L00_00179AC0)[item].h46;
            text = func_001FDD10(id);
        }
        if (text == 0) return rec->w;
        if (func_001166CC((char *)text) == 0) return rec->w;
        text_length = func_001166CC((char *)text);
        suffix = text;
        for (j = 0; j < text_length; j++) {
            suffix++;
            if (*(const unsigned char *)suffix == '-') {
                suffix++;
                prefix_length = j + 2;
                break;
            }
        }
        if (prefix_length == 0) {
            suffix = text;
            for (j = 0; j < text_length; j++) {
                suffix++;
                if (*(const unsigned char *)suffix == ' ') {
                    suffix++;
                    prefix_length = j + 1;
                    break;
                }
            }
        }
        text_y = y + GI(D_L00_0015F724);
        {
        int id;
        char *wr;
        id = *(int *)((char *)D_0014171B_i + 0x885 + selected * 4);
        wr = D_L00_001C40B0 + id * 24;
        if (*(unsigned short *)(wr + 8) != 0)
            text_y += GI(D_L00_0015F7B4);
        else if (prefix_length != 0)
            text_y += GI(D_L00_0015F7B8);
        else
            text_y += GI(D_L00_0015F7BC);
        }
        if (prefix_length != 0) {
            func_001F6AF0(x + GI(D_L00_0015F720) + 1, text_y + 1, color, (void *)text, prefix_length);
            func_001F6AF0(x + GI(D_L00_0015F720), text_y, shadow, (void *)text, prefix_length);
            text_y += 16;
            func_001F6AF0(x + GI(D_L00_0015F720) + 1, text_y + 1, color, (void *)suffix, -1);
            func_001F6AF0(x + GI(D_L00_0015F720), text_y, shadow, (void *)suffix, -1);
        } else {
            func_001F6AF0(x + GI(D_L00_0015F720) + 1, text_y + 1, color, (void *)text, -1);
            func_001F6AF0(x + GI(D_L00_0015F720), text_y, shadow, (void *)text, -1);
        }
    }
    return rec->w;
}
