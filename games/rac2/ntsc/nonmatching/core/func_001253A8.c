/* sceCdRead, libcdvd's cdvd005.o. 480 bytes, one place (boot).

 * Below: the instruction-level map from the first run, then the complete C
 * written on 2026-10-15 and the measurement of where it lands.

 * src/core/001253A8.c line 54. sceCdRead, named by its own two debug strings
 * "call cdread cmd" (D_0013B2E0) and "cdread end" (D_0013B2F8) and by RPC #1
 * on the N-command client D_00137550. Frame 0x80; s3 = arg0 (bsize),
 * s1 = arg1 (lsn), s4 = arg2 (mode), s0 = arg3 (the 3-byte command data).
 *
 *   1. if ((D_001363B4 & 1) == 0 && func_00124AF0() == 6) return 0;
 *   2. if (func_00124980(4) == 0) return 0;      /* N-command guard *\/
 *   3. build the command at D_00136480: +0 bsize, +4 lsn, +8 mode,
 *      +0xC/+0xD/+0xE = buff[0..2], +0x10 = D_00137480 (send buffer),
 *      +0x14 = D_00137540;
 *   4. select the read command from buff[2]: 1 -> 0x918, 2 -> 0x924,
 *      and `mult` it against lsn -- note the R5900 three-operand form, rd set;
 *   5. if ((D_001363B4 & 2) == 0) { D_00137540 = 0; func_0011CEC8(mode, ...); }
 *      then func_0011CEC8 (sceSifWriteBackDCache) over 0x90 bytes at
 *      D_00137480, 0x18 at D_00136480 and 4 at D_00137540;
 *   6. if (D_00136390 > 0) func_0011C820 (Kprintf) with "call cdread cmd";
 *   7. D_001363D4 = D_001363B0 = 1; sceSifCallRpc(func_0011D620) with
 *      a0 = 0x137550, a1 = 1, a2 = 1, a3 = 0x136480, t0 = 0x18, t1 = 0,
 *      t2 = 0, t3 = 0x1246E0, 0(sp) = 0x137480;
 *   8. on failure clear the two flags and func_0011AC40(D_001363A8), return 0;
 *      on success, if (D_00136390 > 0) Kprintf("cdread end"), return 1.
 *
 * rac1's src/core/00121750.c is the same object (sceCdRead) one RAC earlier
 * and carries two matched siblings, func_00121930 (sceCdGetError) and
 * func_001219C8 (sceCdBreak), which give the S-command / RPC / SignalSema
 * idiom and the "failure path first" layout the source needs.

 * Measured 2026-10-15 (src/core/001253A8.c, sdk -O2 -G2): the C below is the
 * whole function and its structure reproduces retail's control flow, but the
 * allocation rotates and needs one register too many -- our frame is 0x90
 * against 0x80 and s6 is saved, retail runs s0-s5 only. Retail: s3 = arg0
 * (later 0x137480), s2 = &D_00136480, s1 = arg1, s4 = mode (later 0x130000),
 * s0 = arg3 (later D_00137540), s5 = 0x130000 (flag base). Ours: s2 = arg0,
 * s5 = &D_00136480 copied to s4 for the writeback call (an extra daddu retail
 * does not need), s6 = flag base, and s5 again for the D_00136390 read, so a
 * seventh callee-saved register is live. Same wall class as func_0011DE08 /
 * func_001286C8; one shape tried, more are cheap from here. */

typedef struct {
    int arg0;                  /* 0x00 */
    int arg1;                  /* 0x04 */
    int mode;                  /* 0x08 */
    unsigned char cmd[3];      /* 0x0C */
    char pad_0F;
    char *sendbuf;             /* 0x10 */
    int *unk14;                /* 0x14 */
} CdReadCmd_1253A8;

extern int D_001363B4;
extern int D_00136390;
extern int D_001363D4;
extern int D_001363B0;
extern int D_001363A8;
extern CdReadCmd_1253A8 D_00136480;
extern char D_00137480[];
extern int D_00137540;
extern char D_00137550[];
extern char D_0013B2E0[];
extern char D_0013B2F8[];
extern int func_00124AF0(void);
extern int func_00124980(int);
extern int func_0011CEC8();
extern int func_0011D620();
extern void func_0011C820(char *);
extern void func_001246E0(void);
extern void func_0011AC40(int);

int func_001253A8(int arg0, int arg1, int arg2, unsigned char *buff) {
    int size;

    if ((D_001363B4 & 1) == 0 && func_00124AF0() == 6) {
        return 0;
    }
    if (func_00124980(4) == 0) {
        return 0;
    }
    D_00136480.arg0 = arg0;
    D_00136480.arg1 = arg1;
    D_00136480.mode = arg2;
    D_00136480.cmd[0] = buff[0];
    D_00136480.cmd[1] = buff[1];
    D_00136480.cmd[2] = buff[2];
    D_00136480.sendbuf = D_00137480;
    D_00136480.unk14 = &D_00137540;
    if (buff[2] == 1) {
        size = arg1 * 0x918;
    } else if (buff[2] == 2) {
        size = arg1 * 0x924;
    } else {
        size = arg1 << 11;
    }
    if ((D_001363B4 & 2) == 0) {
        D_00137540 = 0;
        func_0011CEC8(arg2, size);
    }
    func_0011CEC8(D_00137480, 0x90);
    func_0011CEC8(&D_00136480, 0x18);
    func_0011CEC8(&D_00137540, 4);
    if (D_00136390 > 0) {
        func_0011C820(D_0013B2E0);
    }
    D_001363D4 = 1;
    D_001363B0 = 1;
    if (func_0011D620(D_00137550, 1, 1, &D_00136480, 0x18, 0, 0, func_001246E0, D_00137480) >= 0) {
        if (D_00136390 > 0) {
            func_0011C820(D_0013B2F8);
        }
        return 1;
    }
    D_001363D4 = 0;
    D_001363B0 = 0;
    func_0011AC40(D_001363A8);
    return 0;
}
