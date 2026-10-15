#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * boot.cpp (0x12DA38-0x12DB68): ParseBin and main. crt0 calls func_0012DB18
 * as main. From 0x12DB18 on, retail's game compiler spills s-registers with
 * sq, as v1.14 does (docs/TOOLCHAIN.md). Name from the NTSC split.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);
extern int func_00114060(int, void *);
extern void func_00113AE0(void *);
extern void func_00117118(void *, void *, int, int);
extern int func_00119008();
extern int D_0012FCF0 NOT_SDA;
extern void func_00118E90(int arg0, void *arg1);
extern void *D_00154A40 NOT_SDA;
extern int D_00155080[];
extern void func_001193F8(int);
extern void func_00118AD0(int, int);
extern int D_00154F54;
extern int D_0012FD04;
extern int D_00154F64 NOT_SDA;
extern int D_00154F6C NOT_SDA;
extern void func_0011AA90(int, int, int, int, int, int, int);
extern void func_0011AA00(void);
extern int D_0012FD08 NOT_SDA;
extern int func_0011D960(void);
extern void func_0011D9A8(void);
extern int func_00118C70(void *);
extern int D_0012FDA0;
extern int D_0012FDA4;
extern char D_00157E80[];
extern int D_0012FD9C;
extern void func_0011BBF0(void);
extern int D_0012FD9C NOT_SDA;
extern char D_0012FCEC[];
extern char D_001580A8[];
extern int D_0012FDA8;
extern void func_001153FC(void *, int, int);
extern int D_0012FD94;
extern int D_0012FDAC;
extern char D_00158140[];
extern int D_00158180;
extern int D_001581C0;
extern char D_00158528[];
extern int D_0012FDB4;
extern int func_0011CE70(int arg0, int arg1, int arg2, void *arg3);
extern void func_00118EC0(void);
extern void func_0011D4E0(void);
extern void func_00118EB0(void);
extern int D_00130420;
extern int D_00130424;
extern int D_00130BD0[];
extern char D_00130428[];
extern int func_0011DC50(void);
extern void func_00118D80(int);
extern int func_00120F30(int);
extern void *D_00159840;
extern int D_001313E0;
extern int D_001313E8;
extern int D_001313EC;
extern int D_001313F0;
extern int D_001313E4;
extern int D_001313FC;
extern int func_0011B4C8();
extern int func_00120D28(int);
extern void func_00118C90(int);
extern char D_00132590[];
extern int D_00131440;
extern void func_0011A6C8();
extern int func_0011B6B8(void *);
extern char D_00153000[];
extern char D_00132E08[];
extern int D_001313D0;
extern int func_00121040(int);
extern int D_001325C0;
extern char D_00132E40[];
extern void func_00123650(void *);
extern char D_001534E0[];
extern int D_00132E70[];
extern int func_001238B0(int, int, int, int);
extern int D_00132EA8;
extern int *D_00159B28;
extern int *D_00159B2C;
extern int *D_00159B30;
extern char D_0015B108[];
extern int D_0015B180;
extern void func_00124B60(void *);
extern char D_00153658[];
extern int func_00124920(int);
typedef struct {
    char unk_00[4];
    int  unk_04;
    int  unk_08;
    char *unk_0C;
    char unk_10[0x320];
} Ent330;
extern Ent330 D_0015B640[];
extern void func_00119288(void *a, void *b);
extern void func_001286E8(int, int);
extern int D_00132F70[];
extern void func_0012BCC8(int);
extern void func_00128590(void *);
extern void func_00128968(void *, int);
extern int func_00128860(void *, int);
extern int func_00128A58(void *, int);
extern void func_00129180(void *);
extern char D_00153A80[];
extern void func_00116248_4(void *, char *, int, int) __asm__("func_00116248");
extern void func_0012C468_a(void *, void *) __asm__("func_0012C468");
extern void func_0012AAC8(void *, int);
extern void func_0012C468(void *, void *);
extern char D_00153B38[];
extern void func_0012C0A0(void *);
extern void func_0012BF40(void *);
extern void func_0012C278(void *);
extern void func_00129E30(void *, int, int, int);
extern char D_00153BB8[];
extern char D_00153BD8[];
extern void func_00116248(void *);
extern void func_0012C420(void *);
extern char D_00153C48[];
extern char D_00153C78[];
extern char D_00153C90[];
extern char D_00153CC8[];
extern int func_0012CE48(void *);
extern void func_0012D2A0(void);
extern signed char D_001331D8[];
extern int func_0012D4E0(int);
extern int func_0012D4B0(int);
extern void func_0011DDA0(int);

extern int *D_0015EF4C MACRO_ADDR;

typedef void (*BinFn)(void);

struct Chunk {
    unsigned char *dst;
    int size;
    int pad8;
    BinFn id;
};

/* ParseBin: copy each boot-binary chunk to its destination while chunk ids agree; return the id. Adapted from Lombyte (MIT) for PAL: gameplay/state/parse_bin.c, parse_bin. */
BinFn func_0012DA38(void) {
    unsigned char *p;
    struct Chunk *c;
    unsigned char *dst;
    unsigned char *src;
    BinFn id;

    id = 0;
    p = (unsigned char *)(D_0015EF4C[0] + (int)D_0015EF4C);
    while (c = (struct Chunk *)p, p = (unsigned char *)(c + 1), dst = c->dst, src = p,
           (id == 0) ? (id = c->id, 1) : (id == c->id)) {
        if (((c->size & 7) == 0) && (((unsigned int)p & 7) == 0) && (((unsigned int)dst & 7) == 0)) {
            unsigned char *d = dst;
            unsigned char *s = p;
            unsigned char *e = dst + c->size;
            while (d != e) {
                *(long *)d = *(long *)s;
                d += 8;
                s += 8;
            }
        } else {
            unsigned char *e = dst + c->size;
            while (dst != e) {
                *(int *)dst = *(int *)src;
                dst += 4;
                src += 4;
            }
        }
        p += c->size;
    }
    return id;
}


extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");




extern int D_0012FD9C;


extern void func_00116248_4(void *, char *, int, int) __asm__("func_00116248");
extern void func_0012C468_a(void *, void *) __asm__("func_0012C468");
extern void func_0012C468(void *, void *);
extern void func_00116248(void *);

typedef void (*EntryFn)(void);
extern void func_0011DFC8(void);
extern EntryFn func_0012DA38(void); /* ParseBin(void) */
extern void func_001E99D8(void);

void func_0012DB18(void) { /* main */
    EntryFn entry;

    func_0011DFC8();
    entry = func_001E99D8;
    do {
        entry();
        entry = func_0012DA38();
        func_00118D80(0);
        func_00118D80(2);
    } while (1);
}
