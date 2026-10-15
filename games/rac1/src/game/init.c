#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * init.cpp in the original source; text 0x201D58-0x201E10.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern long D_00152178 NOT_SDA;
extern int func_001FE4D0(void);
extern char D_00199A68[];
extern short D_0015F780;
extern int D_001941CC NOT_SDA;
extern int D_0019A4E8 NOT_SDA;
extern int func_001FF668(int);
typedef struct {
    char b[0x13];
} Cfg13;
extern Cfg13 D_0019A540 NOT_SDA;
extern Cfg13 D_001E7DD8 NOT_SDA;
extern int func_00116810(void);
extern void func_001166FC(Cfg13 *, void *);
extern short D_0015F9D0;
extern void func_00201960(int, int, int, int, int);

extern int func_0011CBC8(int arg0);
extern int func_00118E20(void *buf, int flag);
extern int func_00118E10(int modid);
extern int func_0011D078(int a, int b, int c);
extern void func_0011CCB0();

/* LoadIRXModule(name, args): starts an IOP module load through an RPC
   helper, hands func_00118E20 a 4-word tagged buffer, polls the returned
   id until it is ready, then starts the module and reports the result.
   The nops before the poll loop's branch are the short-loop padding
   (tools/fix_short_loops.py); the result test comes before the
   func_0011CCB0 call, where retail's movz sits in its delay slot. */
int func_00201D58(void *arg0, void *arg1) {
    int buf[4];
    int result;
    int modid;
    int status;
    int ret = 1;

    result = func_0011CBC8((int)arg1);
    buf[0] = (int)arg0;
    buf[1] = result;
    buf[2] = (int)arg1;
    buf[3] = 0;
    modid = func_00118E20(buf, 1);
    if (modid != 0) {
        while (func_00118E10(modid) >= 0) {
        }
        status = func_0011D078(result, 0, 0);
        if (status < 0) {
            ret = 0;
        }
        func_0011CCB0(result);
    }
    return ret;
}
