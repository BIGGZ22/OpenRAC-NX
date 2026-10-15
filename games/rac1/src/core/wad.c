#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * wad.cpp (0x12F348-0x12F580), the last object in core_text. Name from
 * the NTSC split.
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
extern short D_0015ED84;
extern short D_0015ED80;
extern short D_0015EDC4;
extern void func_0012DDC0(void);

extern unsigned char D_0015EE58[4] MACRO_ADDR;
extern int func_00121750(int, int, void *, void *);
extern void func_00122598(int);
extern int func_00121930(void);

/* wad_GetSectors_FiiPv(lba, sectors, buf): sceCdRead with the usual
 * request/poll/retry shape (see func_0012F3F8/func_0012F4A8 below),
 * returning the byte count read (sectors << 11 = sectors * 0x800). */
int func_0012F348(int lba, int sectors, void *buf) {
    char hdr[4];

    hdr[0] = 0x20;
    hdr[1] = D_0015EE58[0];
    hdr[2] = 0;
    hdr[3] = 0;

    do {
        func_00121750(lba, sectors, buf, &hdr);
        while (func_00120F30(1) != 0) {
            func_00122598(0);
        }
    } while (func_00121930() != 0);

    return sectors << 11;
}

typedef struct { char b[4]; } StreamHdr;
extern int D_00137C80[];

/* Same request/poll/retry shape as func_0012F348 above, reading 6 fixed
   sectors into a stack buffer and copying 0x2960 bytes of it into
   D_00137C80. The nop after `jal func_00120F30` in the poll loop is
   ps2eeas's short-loop padding (tools/ps2eeas_nops.py). */
int func_0012F3F8(void) {
    char buf[0x3000];
    StreamHdr hdr;
    unsigned int i;

    hdr.b[0] = 0x20;
    hdr.b[1] = D_0015EE58[0];
    hdr.b[2] = 0;
    hdr.b[3] = 0;

    do {
        func_00121750(0x5DC, 6, buf, &hdr);
        while (func_00120F30(1) != 0) {
            func_00122598(0);
        }
    } while (func_00121930() != 0);

    for (i = 0; i < 0x2960; i++) {
        ((char *)D_00137C80)[i] = buf[i];
    }
    return 1;
}

extern int D_0013A548[];
extern int D_0013A5E0[];

/* As func_0012F3F8 but the sector number comes from an 8-byte-stride
   table indexed by arg0 and re-read on every retry, and 0x2434 bytes are
   copied into D_0013A5E0. The entry address is formed after the header
   stores, which orders the two constant-address computations as retail
   does. */
int func_0012F4A8(int arg0) {
    int *entry;
    char buf[0x2800];
    StreamHdr hdr;
    unsigned int i;

    hdr.b[0] = 0x20;
    hdr.b[1] = D_0015EE58[0];
    hdr.b[2] = 0;
    hdr.b[3] = 0;
    entry = &D_0013A548[arg0 * 2];

    do {
        func_00121750(*entry, 5, buf, &hdr);
        while (func_00120F30(1) != 0) {
            func_00122598(0);
        }
    } while (func_00121930() != 0);

    for (i = 0; i < 0x2434; i++) {
        ((char *)D_0013A5E0)[i] = buf[i];
    }
    return 1;
}
