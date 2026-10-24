#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * stash.cpp in the original source; text 0x233FF8-0x234380.
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
extern void func_002023E0(int);
extern void func_002027C0(int);
extern void func_00204FC0(void *);
extern int D_0018CC20 NOT_SDA;
extern int D_001941C8 NOT_SDA;
extern int D_0016100C;
extern int D_001A0468[];
extern void func_00205830(int a, int b);
extern PadSlots D_001A01F0_slots __asm__("D_001A01F0");
extern int D_001A01F0[];
extern int *D_001602E0;
extern unsigned char D_0013D49C NOT_SDA;
extern unsigned char D_0013D49D NOT_SDA;
extern unsigned char D_0013D4A5 NOT_SDA;
extern short D_0015FE24;
extern unsigned char D_0013D4AC NOT_SDA;
extern unsigned char D_0013D4AD NOT_SDA;
extern unsigned char D_0013D4AE NOT_SDA;
extern unsigned char D_0013D4AF NOT_SDA;
extern unsigned char D_0013D4B5 NOT_SDA;
extern int D_001A04B4 NOT_SDA;
extern unsigned char D_0013D4C5 NOT_SDA;
extern int D_001414DC NOT_SDA;
extern unsigned char D_0013D4C0 NOT_SDA;
extern unsigned char D_0013D4C1 NOT_SDA;
extern unsigned char D_0013D4C2 NOT_SDA;
extern unsigned char D_0013D4D3 NOT_SDA;
extern unsigned char D_0013D4D4 NOT_SDA;
extern unsigned char D_0013D4D5 NOT_SDA;
extern unsigned char D_0013D4E0;
extern unsigned char D_0013D4DC NOT_SDA;
extern unsigned char D_0013D4DD NOT_SDA;
extern unsigned char D_0013D4DE NOT_SDA;
extern unsigned char D_0013D4DF NOT_SDA;
extern unsigned char D_0013D4E1 NOT_SDA;
extern unsigned char D_0013D4E9 NOT_SDA;
extern unsigned char D_0013D502 NOT_SDA;
extern unsigned char D_0013D503 NOT_SDA;
extern unsigned char D_0013D504 NOT_SDA;
extern unsigned char D_0013D505 NOT_SDA;
extern unsigned char D_0013D50F NOT_SDA;
extern int D_0013D668[];
extern void func_00209040(void);
extern int func_001FAA28(void *dst, int size, int a, int b);
extern void func_00208860(void *dst);
extern short D_0015EE84;
extern int D_0015EE84_far __asm__("D_0015EE84") NOT_SDA;
extern int D_001A0218[] NOT_SDA;
extern void func_00208458(void *, unsigned char *, int);
extern void func_00208688(void *, unsigned char *);
extern char D_0013D390[];
extern short D_0015EFB0;
extern int D_0015EFB4;
extern int D_001A05C0[];
extern int D_001A08C0[];
extern int func_0020BAD8(int *p);
extern int func_0020BBC8(void *dst, int i, int *table);
extern int func_001236F0(void);
extern int func_001E9730();
extern char D_001E8690[];
extern int D_0013D844 NOT_SDA;
extern unsigned char D_0013D4A8 NOT_SDA;
extern int D_0013D9B4 NOT_SDA;
extern unsigned char D_0013D490[];
extern unsigned char D_0013D5CA NOT_SDA;
extern int D_0013D6B8 NOT_SDA;
extern int D_0013DAE4 NOT_SDA;
extern unsigned char D_0013D4E5 NOT_SDA;
extern int D_0013DB24 NOT_SDA;
extern unsigned char D_0013D4F1 NOT_SDA;
extern int D_0013DC34 NOT_SDA;
extern unsigned char D_0013D605 NOT_SDA;
extern int D_0013D5C8 NOT_SDA;
extern unsigned char D_0013D4B0 NOT_SDA;
extern unsigned char D_0013DE55 NOT_SDA;
extern unsigned char D_0013D5DD NOT_SDA;
extern unsigned char D_0013D5E7 NOT_SDA;
extern int D_001B2F40[];
extern void func_001FA460_2(void *, void *) __asm__("func_001FA460");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern int D_001414D0 NOT_SDA;
extern float D_001CAE00[] NOT_SDA;
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_00118D80(int);
extern char D_00165600[];
extern int D_0015F718;
extern short D_0015F71C;
extern char D_001B3200[];
extern int func_001160D8(void);
extern float func_00214158(void);
extern float func_002140F8(float, float);
extern void func_00215C00(void *, float, float, float);
extern void func_001F9DC0(void *, void *, float);
extern void func_001FA460(void *);
extern void func_002150B0(void *, void *);
extern void func_001FA480(void *, void *);
extern float func_0020D830(void);
extern float func_00215A98(int, float);
extern unsigned char D_0014BFC0[];
extern unsigned char D_0013E620[];
extern unsigned char D_0013D510[];
extern void func_0012F068(void *);
extern void func_002177F0(int);
extern short D_001517D0[];
extern void func_0012EDE0(void *);
extern void func_0012EFE8(void);
extern char D_001E8980[];
extern int func_0012EE98(int, int, int, void *);
extern int func_00217628_v(void) __asm__("func_00217628");
extern void func_00122598(int);
extern void func_00217130(void);
extern void func_0012EC40(void);
extern void func_0012DDC0(void);
extern void func_0012EC30(void);
extern int func_0012F030(void);
extern void func_002167C0(short, short, short);
extern void func_002169B8(short, short, short);
extern short D_001517F0 NOT_SDA;
extern char D_0013CA40[];
extern int D_001CDAE0 NOT_SDA;
extern void func_00124650(void);
extern void func_00124B88(int);
extern int func_00124BC8(void *, void *);
extern void func_00217F68(void *);
extern int D_0015EF90;
extern char D_001D4B90[];
extern char D_001D4BC0[];
extern char D_001D5F70[] NOT_SDA;
extern char D_001D603B[];
extern int D_001A0414;
extern int D_001CFBF4;
extern int D_001CFAD8;
extern void func_0020C7A0(void *);
extern int func_0020CA50(void *, void *, void *, int);
extern int D_00141FA0[];
extern char D_001D0A50[];
extern char D_001D0A88[];
extern int D_001A0418 NOT_SDA;
extern float func_001FA748(float, float);
extern char *D_001D5F74 NOT_SDA;
extern void func_0020E180(int, int);
extern char D_00187040[];
extern void func_00220128(void *);
extern void *func_00226720_a(int) __asm__("func_00226720");
extern int func_002267C0(int);
extern void func_00234C98(int, int);
extern void func_00205E70(void);
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void func_001F68E8_c(int, int, long, void *, int)
    __asm__("func_001F68E8");
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern short D_001602B0;
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern void func_001F5800(int, int, int, int, int, int, int, int, long,
                          long);
extern short D_00151880[];
extern long D_001A0448;
extern int func_00226EA8(int);
extern int func_00226F68(int);
extern int D_0013CC04 NOT_SDA;
extern char D_001D2678[];
extern char *D_001D5F78 NOT_SDA;
extern unsigned char D_001B3E40[] NOT_SDA;
extern void *func_0020D348(void);
extern void func_0020E340(void *, int, int, int, int);
typedef struct {
    int key;
    int flags;
} PadBind;
extern PadBind D_001D6448_t[] __asm__("D_001D6448");
extern int func_00227018(int handle);
extern int D_001D6448[];
extern char D_001D5D58[] NOT_SDA;
extern char *D_001B3580[] NOT_SDA;
extern int D_001D6860[];
extern int D_001D74C0[];
extern int D_001D6760[];
extern char D_00187180_a[] __asm__("D_00187180");
extern char D_00194220[];
extern int D_0013E6BC;
extern void func_002141A8(void *, float, float);
extern void func_001F9BD8_a(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9C30_a(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F9BF0_a(void *, void *, void *) __asm__("func_001F9BF0");
extern int func_001EFE10_a(void *, void *, int, int, int) __asm__("func_001EFE10");
extern char D_00187180[];
extern void func_001F9BF0(void *, void *, void *);
extern void func_001EFE10(void *, void *, int, int, int);
extern float func_001F9D10(int, void *);
extern void func_0022DA10(void *, float, float, float);
extern void func_001F9EE8(void *, void *, void *);
extern float func_001F9CE8(void *);
extern float func_001FA058_a(float, float) __asm__("func_001FA058");
extern void func_001FA898(float);
extern void func_00120F30(int);
extern int func_0012E060(void *, int);
extern void func_0012EE70(int);
extern void func_0012EF48(int);
extern void func_0012E2E8(void);

extern void func_0011AE20(int arg0);

void initialize_sif_rpc(void) {
    func_0011AE20(0);
}

extern int func_0011B2F8(void *client, int id, int mode);
extern int func_0011B6B8(void *client);
extern int func_0011B4C8(void *, int, int, void *, int, void *, int, void *, void *); /* sceSifCallRpc */

/*
 * Authentic structs from IOPSTASH.IRX STABS debug symbols:
 *   StashInfo: base, size, cd[10], free (cur), block (count)
 *   StashBlock: ram (unk_00), qwc (unk_04), comment (unk_08), pad (unk_0C)
 */
typedef StashInfo StashState_00234018;
extern StashState_00234018 D_001DD530_alias __asm__("D_001DD530");
typedef StashBlock Rec10_00234018;
extern Rec10_00234018 D_001DD568_alias[] __asm__("D_001DD568");

/* init_stash: binds the IOP stash RPC server (0x11, no-wait mode; a bind
   error hangs), waits for the bind and retries after a delay loop until
   the server answers, then asks it (RPC 2, sceSifCallRpc's nine
   arguments) for the buffer's base and size and clears the 0x40 slots.
   The server check reads through a pointer set inside the loop: loop.c
   then hoists the struct's full address into a register of its own, as
   retail has it, while the client's address stays a constant. */
void init_stash(void) {
    StashState_00234018 *s;
    int reply[4];
    int i;

    for (;;) {
        if (func_0011B2F8(D_001DD530_alias.cd, 0x11, 1) < 0) {
            for (;;) {
            }
        }
        while (func_0011B6B8(D_001DD530_alias.cd) != 0) {
        }
        s = &D_001DD530_alias;
        if (*(int *)((char *)s->cd + 0x24) != 0) {
            break;
        }
        i = 0xFFFF;
        while (i--) {
        }
    }
    func_0011B4C8(D_001DD530_alias.cd, 2, 0, 0, 0, reply, 0x10, 0, 0);
    D_001DD530_alias.base = reply[0];
    D_001DD530_alias.size = reply[1];
    D_001DD530_alias.free = reply[0];
    D_001DD530_alias.block = 0;
    for (i = 0; i < 0x40; i++) {
        D_001DD568_alias[i].ram = 0;
        D_001DD568_alias[i].qwc = 0;
    }
}
__asm__(".section .text\n\tnop\n");

extern StashBlock D_001DD568[];

/* The stash: a buffer on the IOP side (base, size, from func_00234018's
   RPC), the SIF RPC client at D_001DD538, and a bump allocator over the
   buffer that hands out up to 0x40 slots in D_001DD568. */
extern StashInfo D_001DD530;
extern int func_00118E20(void *, int);

/* Stash_SendData: DMA arg1 quadwords from arg0 to the stash's next free
   address (func_00118E20 takes a {src, dst, size, mode} record), reserve
   arg2 quadwords there, and return the slot. `h->block++` read into the
   slot index is what keeps retail's copy of the old count. */
int Stash_SendData(int arg0, int arg1, int arg2, int arg3) {
    StashInfo *h = &stash_state;
    int dma[4];
    int n;

    if (h->size - (h->free - h->base) < arg2 * 16) {
        return -1;
    }
    if (h->block == 0x40) {
        return -2;
    }
    dma[0] = arg0;
    dma[1] = h->free;
    dma[2] = arg1 * 16;
    dma[3] = 0;
    func_00118E20(dma, IOP_STASH_FETCH);
    n = h->block++;
    stash_entries[n].ram = h->free;
    stash_entries[n].qwc = arg2;
    stash_entries[n].comment = arg3;
    h->free += arg2 * 16;
    return n;
}

extern int func_00234350(unsigned int);
extern int func_0011B4C8(void *, int, int, void *, int, void *, int, void *, void *); /* sceSifCallRpc */
extern char D_001DD538[];

/* stash_receive_data(dest, slot, offset, size, mode): size -1 means the
   slot's whole length (func_00234350). -3 for a bad or empty slot, -1
   if offset + size runs past the slot. Otherwise the data is fetched
   from the stash (slot base + offset quadwords) to dest by the stash
   client's RPC 1 (IOP_STASH_FETCH), at most 0xFFFF quadwords per call; returns 0. The RPC
   is sceSifCallRpc, with nine arguments: end function and end
   parameter 0, the last on the stack. */
int stash_receive_data(void *dest, unsigned int slot, int offset, int size, int mode) {
    int src;
    int chunk;

    if (size == -1) {
        size = stash_get_reserved_qwords(slot);
    }
    if (slot >= 0x40) {
        return -3;
    }
    if (stash_entries[slot].qwc == 0) {
        return -3;
    }
    if (stash_entries[slot].qwc < offset + size) {
        return -1;
    }
    src = stash_entries[slot].ram + offset * 16;
    while (size != 0) {
        chunk = (size > 0xFFFF) ? 0xFFFF : size;
        func_0011B4C8(stash_rpc_client_address, IOP_STASH_FETCH, mode, &src, 0x10, dest, chunk * 16, 0, 0);
        size -= chunk;
        dest = (char *)dest + chunk * 16;
        src += chunk * 16;
    }
    return 0;
}

int stash_get_reserved_qwords(unsigned int arg0) {
    if (arg0 >= 0x40) {
        return -3;
    }
    return stash_entries[arg0].qwc;
}
