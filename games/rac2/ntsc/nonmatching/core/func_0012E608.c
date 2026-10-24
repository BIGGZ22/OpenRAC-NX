/* _csc_storeRefImage, from Lombyte (GPL-3.0) src/assembly/sdk/dma/csc_store_ref_image.c
   (its NON_MATCHING body is the recovered source; see the header comment in the
   body for the name mapping). Every instruction matches retail - the same frame,
   the same six saves in 16-byte slots, the same `mult rd,rs,rt` for
   `total = img->w * img->h`, both IPU_CTRL spins, both D3 write blocks, the
   split/short branches and the trailing callback - except ONE delay slot.
   Retail fills `jal enable_dmac(4)`'s slot with the preceding
   `*(volatile u32 *)0x1000E010 = 0x10;` (`sw $v1,0($v0)`), this cc1 emits a
   `nop` there instead and grows the function to 656 bytes. The refusal is the
   `volatile`: drop it and this cc1 fills the slot but also folds the address to
   `lui $at,0x1001 / sw $v0,-0x1FF0($at)`, where retail keeps the split
   `lui $v0,0x1000 / ori $v0,$v0,0xE010`; that non-volatile form matches
   retail's 648-byte extent exactly but then differs in the address form and the
   branch distances. Tested and rejected: declaring the enable_dmac store
   through a local `volatile u32 *`, swapping `enable_dmac(4)` and the store,
   unprototyped callee declarations, and the `sdk -O2 -G2` vs
   `sdk -O2 -g2 -gstabs` profiles (both 656). Unblocked by a cc1 that puts a
   volatile store in a call's delay slot while keeping the split address.

   WAVE 4: it is TWO delay slots, not one. Compiled as it stands (656 against
   retail's 648) the extra words are
     retail : jal enable_dmac(4) ; sw $v1,0($v0)          (the 0x1000E010 = 0x10 store)
     here   : sw $v1,0($v0) ; jal ; nop
     retail : beqz $v0,L ; jal func_0011F628              (the DIntr test)
     here   : beqz $v0,L ; nop ; jal func_0011F628 ; nop
   - one store before the call that retail has IN the call's slot (the backward
   direction, the same wall func_001307B0 hits), and one call that retail puts in
   the branch's slot. Measured and rejected in wave 4: SDK-plausible prototypes
   (AddDmacHandler/DIntr returning int, enable_dmac/EnableInterrupts/disable_dmac/
   RemoveDmacHandler/sendIpuCommand/_doCSC2 returning void, with their argument
   types) - they change the top of the function (the IPU_CTRL read moves from $v1
   to $v0) and leave both slots at 656/648. */
#include "common.h"

/* _csc_storeRefImage: adapted from Lombyte (GPL-3.0)
   src/assembly/sdk/dma/csc_store_ref_image.c, whose NON_MATCHING body is the
   recovered source. Queues the IPU's colour-space conversion for one reference
   image and runs the CSC while D3 (fromIPU) drains: if the picture needs more
   than 0xFFFF qwords the transfer is split, a channel-4 handler is installed
   and the remainder is handled by it. Lombyte's names map to RAC2 as:
   _dispatchMpegCallback = func_0012FA98, _sendIpuCommand = func_0012C380,
   AddDmacHandlerSecondary = func_0011A940 (the handler itself is
   func_0012E538), enable_dmac = func_0011B6C0, disable_dmac = func_0011B658,
   RemoveDmacHandler = func_0011A950, DIntr/EnableInterrupts =
   func_0011F5E0/func_0011F628, _doCSC = func_0012E110, _doCSC2 = func_0012E378.
   `total = img->w * img->h` is the R5900's three-operand `mult rd, rs, rt`,
   which rabbitizer prints with two operands and marks INVALID. */

typedef struct {
    u32 count;
    s32 addr;
} CscDma;

typedef struct {
    u8 pad0[0xD8];
    u32 data;
    u8 padDC[0x77C];
    void *cbData;
} CscDec;

typedef struct {
    s32 dest;
    u8 pad4[0x8];
    s32 w;
    s32 h;
} CscImage;

extern void func_0012E538(void);

/* Unprototyped on purpose, as the recovered source declares them: the
   prototype changes which delay slots this compiler fills. */
extern s32 func_0011A940();
extern s32 func_0011F5E0();
extern s32 func_0011F628();
extern s32 func_0011A950();
extern s32 func_0012FA98();
extern s32 func_0012E110();
extern s32 func_0012E378();
extern s32 func_0012C380();
extern s32 func_0011B658();
extern s32 func_0011B6C0();

void func_0012E608(CscDec *d, CscImage *img) {
    s32 cbarg[8];
    CscDma dma;
    s32 total;
    s32 small;
    s32 handle;
    s32 intr;

    total = img->w * img->h;
    cbarg[0] = 2;
    func_0012FA98(d->cbData, cbarg);
    if (*(volatile u32 *)0x10002010 & 0x4000) {
        *(volatile u32 *)0x10002010 = 0x40000000;
    }
    small = total < 0x400;
    while ((s32) * (volatile u32 *)0x10002010 < 0) {
    }
    func_0012C380(d, 0);
    while ((s32) * (volatile u32 *)0x10002010 < 0) {
    }
    dma.count = total * 0x18;
    dma.addr = img->dest & 0x0FFFFFFF;
    if (dma.count > 0xFFFF) {
        /* The handler receives the stack DMA state; the first transfer uses 0xFFFF QWC. */
        handle = func_0011A940(4, func_0012E538, 0, &dma);
        *(volatile u32 *)0x1000E010 = 0x10;
        func_0011B6C0(4);
        intr = func_0011F5E0();
        *(volatile u32 *)0x1000B410 = dma.addr;
        *(volatile u32 *)0x1000B420 = 0xFFFF;
        *(volatile u32 *)0x1000B400 = 0x101;
        if (intr) {
            func_0011F628();
        }
        dma.addr = (dma.addr + 0xFFFF0) & 0x0FFFFFFF;
        dma.count = dma.count - 0xFFFF;
        if (small) {
            func_0012E110(d, d->data, total);
        } else {
            func_0012E378(d, d->data, total);
        }
        func_0011B658(4);
        func_0011A950(4, handle);
    } else {
        /* Even the short transfer clears the stack count before running CSC. */
        intr = func_0011F5E0();
        *(volatile u32 *)0x1000B410 = img->dest & 0x0FFFFFFF;
        *(volatile u32 *)0x1000B420 = dma.count;
        *(volatile u32 *)0x1000B400 = 0x101;
        if (intr) {
            func_0011F628();
        }
        dma.count = 0;
        if (small) {
            func_0012E110(d, d->data, total);
        } else {
            func_0012E378(d, d->data, total);
        }
    }
    cbarg[0] = 3;
    func_0012FA98(d->cbData, cbarg);
}
