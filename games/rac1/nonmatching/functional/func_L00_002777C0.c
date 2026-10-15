/* func_L00_002777C0 -- src/overlays/shared/pause_00277208.c (functional C for the port, not a match)
 * Page-menu close, run from the menu tick once the menu state (+0x110) reaches 10: syncs the VU1
 * chain, runs each of the current page's 14 objects' close handlers (+0xC, (obj, 0)) and clears the
 * page, writes back the four changed equipped items (menu +0x30 vs D_0014171B + 0x45, 0 becomes 0x26)
 * into the hero block (+0x20B8), restores the stream base (D_0015EF78) and the HUD panels, frees the
 * 14 preview mobys, starts the two close fades (func_L00_002367A8), sets the menu to state 0x14,
 * clears the counter slots the menu added, re-enables the camera and resumes the game.
 * From the staged near miss (nonmatching/shared/func_L00_002777C0.c), made self-contained.
 * equiv: DIFFERENT by shape only: retail rebuilds D_L00_001BA220's address after a label (so the
 * free loop's base, load and store show as unknown) where this keeps its high half in a saved
 * register; the slot-clearing loop is rotated with one more `sll`; one branch-likely naming. Every
 * call, argument, store and loop bound is the same. */
typedef struct {
    int state;
    int *page;
    int *next;
    int unkC;
    int unk10;
    int unk14;
    int unk18;
    unsigned char pad1C[0x30 - 0x1C];
    int items[4];
    unsigned char pad40[0xCC - 0x40];
    int unkCC;
    unsigned char padD0[0x110 - 0xD0];
    int closing;
} Menu_2777C0;
typedef struct {
    unsigned char pad0[0xC];
    void (*close)(void *, int);
} Obj_2777C0;
typedef struct {
    unsigned char pad0[0x74];
    int f74;
    unsigned char pad78[0x80 - 0x78];
    int f80;
    int f84;
} Hud_2777C0;
typedef struct {
    unsigned char pad0[0xC];
    unsigned char count;
    unsigned char padD[0x48 - 0xD];
    int slots[1];
} Cnt_2777C0;

extern Menu_2777C0 M_2777C0 __asm__("D_L00_001BA070");
extern char D_L00_0017E5D8_2777C0[] __asm__("D_L00_0017E5D8");
extern Cnt_2777C0 *D_L00_00197680_2777C0 __asm__("D_L00_00197680");
extern int D_L00_00173F40_2777C0[] __asm__("D_L00_00173F40");
extern char D_00141760_2777C0[] __asm__("D_00141760");
extern char D_0013F450_2777C0[] __asm__("D_0013F450");
extern int D_0015EE98_2777C0 __asm__("D_0015EE98");
extern int D_0015EF78_2777C0 SDATA(D_0015EF78);
extern int D_L00_0015F6BC_2777C0 __asm__("D_L00_0015F6BC") MACRO_ADDR;
extern void *D_L00_001BA220_2777C0[] __asm__("D_L00_001BA220");

extern void func_00234AC8_2777C0(int) __asm__("func_00234AC8");
extern void func_002348E8_2777C0(void) __asm__("func_002348E8");
extern void func_001FF958_2777C0(int, int, int) __asm__("func_001FF958");
extern void func_L00_00235CA0_2777C0(int) __asm__("func_L00_00235CA0");
extern void *func_002267C0_2777C0(void *) __asm__("func_002267C0");
extern int func_L00_00235790_2777C0(void) __asm__("func_L00_00235790");
extern int func_001F9850_2777C0(int) __asm__("func_001F9850");
extern void func_L00_002367A8_2777C0(int, int) __asm__("func_L00_002367A8");
extern int func_001FFB38_2777C0(int, int, void *, void *, void *, void *, int) __asm__("func_001FFB38");
extern void func_002349B8_2777C0(void) __asm__("func_002349B8");
extern void func_L00_0023A658_2777C0(void) __asm__("func_L00_0023A658");
extern void func_L00_0023A690_2777C0(void) __asm__("func_L00_0023A690");
extern void func_L00_0023A788_2777C0(void) __asm__("func_L00_0023A788");

void func_L00_002777C0(void) {
    int i, x, o;
    int *dst, *src, *cur;
    Hud_2777C0 *h;

    if (M_2777C0.closing < 10) {
        return;
    }
    func_00234AC8_2777C0(1);
    func_002348E8_2777C0();
    if (M_2777C0.page != 0) {
        for (i = 0; i < 14; i++) {
            Obj_2777C0 *obj = ((Obj_2777C0 **)((char *)M_2777C0.page + 0x44))[i];
            if (obj != 0 && obj->close != 0) {
                obj->close(obj, 0);
            }
        }
        M_2777C0.page = 0;
    }
    /* D_0013E633 + 0xE1D + 0x20B8: the hero block's equipped items */
    {
        char *gp_ = D_0013F450_2777C0;
        gp_ += 0x20B8;
        dst = (int *)gp_;
    }
    src = (int *)((char *)&M_2777C0 + 0x30);
    cur = (int *)D_00141760_2777C0;
    for (i = 3; i >= 0; i--) {
        x = *src;
        if (*cur != x) {
            *dst = x ? x : 0x26;
        }
        dst++;
        src++;
        cur++;
    }
    h = *(Hud_2777C0 **)(D_L00_0017E5D8_2777C0 + 0x18);
    D_0015EF78_2777C0 = M_2777C0.unk18;
    func_001FF958_2777C0(0, h->f74, 0);
    func_L00_00235CA0_2777C0(2);
    if ((*(Hud_2777C0 **)(D_L00_0017E5D8_2777C0 + 0x18))->f80 != 0) {
        func_L00_00235CA0_2777C0(3);
    }
    if ((*(Hud_2777C0 **)(D_L00_0017E5D8_2777C0 + 0x18))->f84 != 0) {
        func_L00_00235CA0_2777C0(4);
    }
    {
        void **p = D_L00_001BA220_2777C0;
        for (i = 13; i >= 0; i--) {
            *p = func_002267C0_2777C0(*p);
            p++;
        }
    }
    o = func_L00_00235790_2777C0();
    func_L00_002367A8_2777C0(o, func_001F9850_2777C0(0xB4));
    o = func_001FFB38_2777C0(2, 0x754E, func_L00_0023A658_2777C0, func_L00_0023A690_2777C0,
                             func_L00_0023A788_2777C0, &D_0015EE98_2777C0, 9999999);
    func_L00_002367A8_2777C0(o, func_001F9850_2777C0(0xB4));
    M_2777C0.state = 0x14;
    M_2777C0.unk14 = 2;
    for (i = M_2777C0.unkCC; i < D_L00_00197680_2777C0->count; i++) {
        *(int *)((char *)D_L00_00197680_2777C0 + i * 4 + 0x48) = 0;
    }
    D_L00_00197680_2777C0->count = M_2777C0.unkCC;
    D_L00_0015F6BC_2777C0 = 1;
    D_L00_00173F40_2777C0[4] &= 0x7FFFFFFF;
    func_002349B8_2777C0();
    func_00234AC8_2777C0(1);
}
