#include "common.h"
#include "structs.h"

#define IPU_CTRL   ((volatile unsigned int *)0x10002010)

#define IPU_BP     ((volatile unsigned int *)0x10002020)

#define IPU_TOP    ((unsigned long *)0x10002030)

/* func_0012C508 returns a short: its epilogue sign-extends $v0. */
extern short func_0012C508(int, int);

extern int r1_D_00132F70[];

extern void r1_func_0012BCC8(int);

typedef struct {
    unsigned char pad0[0x11C];
    int unk11C;
    unsigned char pad120[0x6F8];
    int unk818;
    unsigned char pad81C[0x1C];
    int unk838;
    int unk83C;
    unsigned char pad840[0x18];
    void *cbData;
} MpegVdec;

#define IPU_CMD  ((volatile unsigned long *)0x10002000)

extern int func_0012C680(void *, int);

extern void func_0012C788(void *, int);

extern int func_0012C878(void *, int);

/* rac1-decomp src/core/00125630.c: func_001286E8 */
/* _ipuVdec (libmpeg). Adapted from Lombyte (MIT) for PAL. */
short func_0012C508(int arg0, int tbl) {
    MpegVdec *d = (MpegVdec *)arg0;
    int i;
    int j;
    unsigned int cmd;
    long r;
    long top;
    unsigned int bp;

    i = 0;
    j = 0;
    while ((*IPU_CTRL & 0x80004000) == 0x80000000) {
        if (j++ > 5000) {
            r1_func_0012BCC8((int)d->cbData);
            j = 0;
        }
    }
    cmd = (tbl << 26) | 0x30000000;
    *(volatile unsigned int *)IPU_CMD = cmd;
    d->unk818 = r1_D_00132F70[(int)cmd >> 28];
    r = *IPU_CMD;
    while (r < 0) {
        if (i++ > 5000) {
            r1_func_0012BCC8((int)d->cbData);
            i = 0;
        }
        r = *IPU_CMD;
    }
    bp = *IPU_BP;
    top = *IPU_TOP;
    d->unk838 = top;
    if (top < 0) {
        d->unk83C = -(bp & 0x1F) & 0x1F;
    } else {
        d->unk83C = 32;
    }
    d->unk11C = (int)r == 0;
    return (short)r;
}
extern int func_0012C680(void *arg, int count);
INCLUDE_ASM("asm/core", func_0012C680);

extern void func_0012C788(void *arg0, int arg1);
INCLUDE_ASM("asm/core", func_0012C788);

extern int func_0012C878(void *arg0, int n);
INCLUDE_ASM("asm/core", func_0012C878);

