/* sceDmaPutEnv 0x01E9CEF0..0x01E9D0D0 (interval 480 bytes; the function is
 * 0x01E9CEF0..0x01E9D0C8 including the unreachable word after the return).
 *
 * Writes a DMA environment back into the EE DMA controller: D_CTRL is
 * rebuilt field by field from three 10-entry translation tables (the four
 * low bytes of the environment), then D_STAT, D_PCR, D_RBOR and D_RBSR are
 * programmed from the remaining fields, and finally the whole 20-byte
 * environment is copied to the shadow copy the getter reads back.
 *
 * The five control registers are read first, unconditionally and in retail
 * order D_CTRL, D_STAT, D_PCR, D_RBOR, D_RBSR: the four reads other than
 * D_CTRL are volatile accesses kept for their side effect only, which is why
 * they survive as dead loads above the four range checks.
 *
 * Research leads: Lombyte's GPL-3.0 sceDmaReset
 * (https://github.com/lombyte-project/Lombyte/blob/main/src/sdk/dma/sce_dma_reset.c)
 * and the matched RAC2 body
 * (https://github.com/OpenRAC/rac2-gc-decomp/blob/master/src/core/00127040.c).
 * The RAC5 body here was adapted and checked against this game's own bytes;
 * the RAC2 routine's
 * sceDmaEnv layout and instruction stream this reproduces:
 *   sts/std/mfd/rcyc are the four bytes the tables translate,
 *   express + notify pack D_STAT, tqwc + sqwc pack D_PCR,
 *   rbadr is D_RBOR and rbmsk is D_RBSR. */

typedef struct {
    unsigned char sts;
    unsigned char std;
    unsigned char mfd;
    unsigned char rcyc;
    unsigned short express;
    unsigned short notify;
    unsigned short sqwc;
    unsigned short tqwc;
    void *rbadr;
    unsigned int rbmsk;
} sceDmaEnv;

extern unsigned char dma_ctrl_sts_tab[];   /* 0x01F20D38 */
extern unsigned char dma_ctrl_std_tab[];   /* 0x01F20D48 */
extern unsigned char dma_ctrl_mfd_tab[];   /* 0x01F20D58 */
extern sceDmaEnv sceDmaEnvData;            /* 0x01FD35E0 */

int sceDmaPutEnv(sceDmaEnv *env)
{
    unsigned int ctrl;
    unsigned int pcr;
    unsigned int sqwc;
    unsigned int rbor;
    unsigned int rbsr;

    ctrl = *(volatile unsigned int *)0x1000E000;
    pcr = *(volatile unsigned int *)0x1000E020;
    sqwc = *(volatile unsigned int *)0x1000E030;
    rbor = *(volatile unsigned int *)0x1000E050;
    rbsr = *(volatile unsigned int *)0x1000E040;
    if (env->sts >= 10) {
        return -1;
    }
    if (env->std >= 10) {
        return -2;
    }
    if (env->mfd >= 10) {
        return -3;
    }
    if (env->rcyc >= 7) {
        return -4;
    }
    ctrl = (ctrl & ~0x30) | (dma_ctrl_sts_tab[env->sts] << 4);
    ctrl = (ctrl & ~0xC0) | (dma_ctrl_std_tab[env->std] << 6);
    ctrl = (ctrl & ~0xC) | (dma_ctrl_mfd_tab[env->mfd] << 2);
    if (env->rcyc != 0) {
        ctrl |= 2;
        ctrl = (ctrl & ~0x300) | ((env->rcyc - 1) << 8);
    } else {
        ctrl &= ~2;
    }
    pcr = (env->express << 16) | env->notify;
    sqwc = (env->tqwc << 16) | env->sqwc;
    rbor = (unsigned int)env->rbadr;
    rbsr = env->rbmsk;
    *(volatile unsigned int *)0x1000E000 = ctrl;
    *(volatile unsigned int *)0x1000E020 = pcr;
    *(volatile unsigned int *)0x1000E030 = sqwc;
    *(volatile unsigned int *)0x1000E050 = rbor;
    *(volatile unsigned int *)0x1000E040 = rbsr;
    sceDmaEnvData = *env;
    return 0;
}
