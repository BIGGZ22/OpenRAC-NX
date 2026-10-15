#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x12AC80-0x12CC70. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's MPEG library (libmpeg): sceMpegInit/Create/Delete-style setup,
 * the callback table (sceMpegAddCallback, func_0012BC50) and its
 * dispatcher, the work-area allocator, the picture/field steps and the
 * error reporter ("[MPEG ERROR]%s"). Built with Sony's 2.9-ee like the
 * rest of libmpeg (0012A2F0.c): see func_0012BC50, whose match needs
 * 2.9-ee's type-based alias analysis.
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
extern void func_00129180(void *);
extern char D_00153A80[];
extern void func_00116248_4(void *, char *, int, int) __asm__("func_00116248");
extern void func_0012C468_a(void *, void *) __asm__("func_0012C468");
extern void func_0012AAC8(void *, int);
extern void func_00127378(int arg0);
extern int func_0012AB60(void *arg0, int arg1);

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012AC80);

LINKER_REMNANT("asm/remnants/core_text", func_0012AD08);

typedef struct {
    u8 pad0[0x18];
    u64 pos;
    u8 pad20[0x10];
} SysBit;
typedef struct {
    s32 v[6];
} PackHdr;
typedef struct {
    u64 id;
    s32 len;
    s32 scrambling;
    s64 pts;
    s64 dts;
    s32 data;
    s32 datalen;
    s32 header;
    s32 pad2C;
} PesHdr;
typedef struct {
    PackHdr pack;
    PesHdr pes;
} PssHdr;
typedef struct {
    s32 type;
    u8 *header;
    u8 *data;
    u32 len;
    s64 pts;
    s64 dts;
} sceMpegCbDataStr;
typedef s32 (*sceMpegCallback)(void *mp, sceMpegCbDataStr *cbstr, void *data);
typedef struct {
    u64 id;
    u64 mask;
    sceMpegCallback func;
    void *data;
} StreamCb;
typedef struct {
    u8 pad0[0x44];
    StreamCb *tbl;
    s32 n;
} MpegSys;
typedef struct {
    u8 pad0[0x40];
    MpegSys *sys;
} sceMpeg;
extern void func_0012AA70(SysBit *bs, u8 *start, u8 *bufstart, s32 bufsize);
extern s32 func_0012AAA8_2AD10(SysBit *bs, s32 n) __asm__("func_0012AAA8");
extern u8 *func_0012AC50(SysBit *bs, s32 pos);
extern s32 func_0012B100_2AD10(SysBit *bs, PackHdr *pack) __asm__("func_0012B100");
extern s32 func_0012B2C0_2AD10(MpegSys *sys, SysBit *bs, PesHdr *pes) __asm__("func_0012B2C0");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/sdk/dma/sce_mpeg_demux_pss_ring.c, sceMpegDemuxPssRing. */
int func_0012AD10(sceMpeg *mp, u8 *start, int size, u8 *bufstart, int bufsize) {
    SysBit bs;
    PssHdr hdr;
    SysBit *b;
    PssHdr *h;
    sceMpegCbDataStr cb;
    sceMpegCallback cbfunc = 0;
    void *cbdata = 0;
    StreamCb *tbl;
    int ret;
    int i;
    int cont;
    MpegSys *sys;
    int headerpos;

    cont = 1;
    h = &hdr;
    sys = mp->sys;
    tbl = sys->tbl;
    func_0012AA70(&bs, start, bufstart, bufsize);
    ret = 0;
    i = 0;
    b = &bs;
    if (i < sys->n) {
        do {
            if (tbl[i].id == 0xBDFF000000) {
                cbfunc = tbl[i].func;
                cbdata = tbl[i].data;
            }
            if (cbfunc != 0) {
                break;
            }
            i++;
        } while (i < sys->n);
    }
    for (;;) {
        if (func_0012AAA8_2AD10(b, 32) == 0x1BA) {
            func_0012B100_2AD10(b, &h->pack);
        }
        while (func_0012AAA8_2AD10(b, 24) == 1 && func_0012AAA8_2AD10(b, 32) != 0x1BA &&
               func_0012AAA8_2AD10(b, 32) != 0x1B9 && b->pos < size * 8 && cont) {
            func_0012B2C0_2AD10(sys, b, &h->pes);
            if (b->pos > size * 8) {
                continue;
            }
            for (i = 0; i < sys->n; i++) {
                if (tbl[i].id == (h->pes.id & tbl[i].mask)) {
                    headerpos = h->pes.header;
                    cb.type = 6;
                    cb.header = func_0012AC50(b, headerpos);
                    cb.data = func_0012AC50(b, h->pes.data);
                    cb.len = h->pes.datalen;
                    cb.pts = h->pes.pts;
                    cb.dts = h->pes.dts;
                    cont = tbl[i].func(mp, &cb, tbl[i].data);
                    break;
                }
            }
            if (i == sys->n && cbfunc != 0) {
                headerpos = h->pes.header;
                cb.type = 6;
                cb.header = func_0012AC50(b, headerpos);
                cb.data = func_0012AC50(b, h->pes.data);
                cb.len = h->pes.datalen;
                cb.pts = h->pes.pts;
                cb.dts = h->pes.dts;
                cont = cbfunc(mp, &cb, cbdata);
            }
            if (cont) {
                ret = b->pos >> 3;
            }
        }
        if (b->pos > size * 8 || func_0012AAA8_2AD10(b, 32) != 0x1BA) {
            break;
        }
    }
    return ret;
}

extern long func_0012AC80(int arg0, int arg1);

typedef struct {
    long key;
    long extra;
    int  cb;
    int  data;
} StrCbEnt; /* 0x18 */

typedef struct { char pad[8]; long extra; } Tbl16;
extern Tbl16 D_00132FD8[];

/*
 * sceMpegAddStrCallback (libmpeg.a:pack.o): key = func_0012AC80(arg1,
 * arg2) (a 64-bit stream/type key); linear-search the object's callback
 * table (handlers[7] reused as {table pointer, count} -- raw offsets,
 * not the Handler struct, per this file's warning about that array
 * overlapping) for a matching key, remembering its old callback (0 if
 * none found). Whether found or not (index lands on the match, or one
 * past the end), if there's room (< 0x40 entries), overwrite the slot
 * with the new key/data/callback and the type's extra 8 bytes from
 * D_00132FD8[arg1], and bump the stored count unconditionally. Returns
 * the previous callback.
 */
int func_0012B008(void *arg0, int arg1, int arg2, int arg3, int arg4) {
    int old = 0;
    char *obj = *(char **)((char *)arg0 + 0x40);
    StrCbEnt *tbl = *(StrCbEnt **)(obj + 0x44);
    long key = func_0012AC80(arg1, arg2);
    int count = *(int *)(obj + 0x48);
    int i;

    for (i = 0; i < count; i++) {
        if (key == tbl[i].key) {
            old = tbl[i].cb;
            break;
        }
    }
    if (i < 0x40) {
        *(int *)(obj + 0x48) = count + 1;
        tbl[i].key = key;
        tbl[i].data = arg4;
        tbl[i].cb = arg3;
        tbl[i].extra = D_00132FD8[arg1].extra;
    }
    return old;
}

extern void func_0012ABB0(void *arg0);
extern int func_0012B250();

/*
 * _pack_header (libmpeg.a:pack.o): ISO/IEC 11172-1 2.4.3.3 pack_header().
 * func_0012AB60 is nextBit ("get n bits"), func_0012ABB0 a marker-bit
 * skip, func_0012AAA8 a 32-bit peek (nextStartCode-style). Reads the pack
 * start code's fixed fields (34 then 3, marker, 15, marker, 15, marker,
 * 9, marker), discarding the SCR/mux_rate pieces except for three: the
 * low bit of the first 3-bit group goes to +0x8, and the three groups
 * combine into +0x4 as (first<<30)|(second<<15)|third. The last field
 * read (3 bits) is pack_stuffing_length: that many stuffing bytes are
 * then skipped. Peeks the next 32 bits; a system_header_start_code
 * (0x1BB) sets +0xC and hands off into func_0012B250 (system_header
 * parsing -- called prototype-less here since retail's call site passes
 * it a second, unused argument, the same K&R situation as func_00129F40
 * elsewhere in this file), otherwise clears +0xC. Always returns 1.
 */
int func_0012B100(void *arg0, void *arg1) {
    char *p = (char *)arg1;
    unsigned int first, second, third, stuffing;
    unsigned int i;

    func_0012AB60(arg0, 0x22);
    first = func_0012AB60(arg0, 3);
    func_0012ABB0(arg0);
    second = func_0012AB60(arg0, 0xF);
    func_0012ABB0(arg0);
    third = func_0012AB60(arg0, 0xF);
    func_0012ABB0(arg0);
    *(int *)(p + 0x0) = func_0012AB60(arg0, 9);
    func_0012AB60(arg0, 0x1E);
    stuffing = func_0012AB60(arg0, 3);

    *(unsigned int *)(p + 0x8) = (first >> 2) & 1;
    *(unsigned int *)(p + 0x4) = (first << 30) | (second << 15) | third;

    for (i = 0; i < stuffing; i++) {
        func_0012AB60(arg0, 8);
    }

    if (func_0012AAA8(arg0, 0x20) == 0x1BB) {
        *(int *)(p + 0xC) = 1;
        func_0012B250(arg0, p);
    } else {
        *(int *)(p + 0xC) = 0;
    }
    return 1;
}

int func_0012B250(void *arg0) {
    func_0012AB60(arg0, 0x38);
    func_0012AB60(arg0, 0x28);
    while (func_0012AAA8(arg0, 1) == 1) {
        func_0012AB60(arg0, 0x18);
    }
    return 1;
}

extern void func_0012ABF8(void *arg0, int n);
extern char D_00153AC8[];
extern char D_00153AD8[];

struct PES_BS {
    unsigned char pad_0[0x18];
    long stamp;
};

struct PES_HDR {
    long unk0;
    int unk8;
    int unkC;
    long unk10;
    long unk18;
    int unk20;
    int unk24;
    int unk28;
};

struct PES_TBL {
    unsigned char v[16];
} __attribute__((packed));

/* Parse one MPEG-2 PES packet header from the bit stream into hdr (libmpeg _PES_packet).
   Adapted from Lombyte (MIT) for PAL: src/sdk/library/_PES_packet.c, _PES_packet. */
int func_0012B2C0(int arg0, struct PES_BS *bs, struct PES_HDR *hdr) {
    struct PES_TBL tbl;
    int ctx;
    int flag1;
    int len;         /* PES_header_data_length */
    int base;        /* (int)bs->stamp, captured before the header fields */
    long now;         /* the same stamp, re-read at the sync point */
    int v7;          /* PTS_DTS_flags */
    int v8;          /* 4-bit length, indexes tbl */
    int v5;          /* PES_extension_flag */

    ctx = arg0;
    hdr->unk28 = (int)bs->stamp;
    tbl = *(struct PES_TBL *)D_00153AC8;
    func_0012AB60(bs, 0x18);
    hdr->unk0 = (long)func_0012AB60(bs, 8) << 0x20;
    hdr->unk8 = func_0012AB60(bs, 0x10);
    hdr->unk18 = -1;
    hdr->unk10 = -1;
    if (hdr->unk0 != ((unsigned long)0xBC00 << 0x18) &&
        hdr->unk0 != ((unsigned long)0xBE00 << 0x18) &&
        hdr->unk0 != ((unsigned long)0xBF00 << 0x18) &&
        hdr->unk0 != ((unsigned long)0xF000 << 0x18) &&
        hdr->unk0 != ((unsigned long)0xF100 << 0x18) &&
        hdr->unk0 != ((unsigned long)0xFF00 << 0x18) &&
        hdr->unk0 != ((unsigned long)0xF200 << 0x18) &&
        hdr->unk0 != ((unsigned long)0xF800 << 0x18)) {
        func_0012AB60(bs, 2);
        hdr->unkC = func_0012AB60(bs, 2);
        func_0012AB60(bs, 4);
        v7 = func_0012AB60(bs, 2);
        flag1 = func_0012AB60(bs, 1);
        v8 = func_0012AB60(bs, 4);
        v5 = func_0012AB60(bs, 1);
        len = func_0012AB60(bs, 8);
        base = bs->stamp;
        if (v7 & 2) {
            unsigned int a, b, c;
            func_0012AB60(bs, 4);
            a = func_0012AB60(bs, 3);
            func_0012ABB0(bs);
            b = func_0012AB60(bs, 0xF);
            func_0012ABB0(bs);
            c = func_0012AB60(bs, 0xF);
            func_0012ABB0(bs);
            hdr->unk10 = ((long)((a >> 2) & 1) << 0x20) | (long)(unsigned int)(a << 30 | b << 15 | c);
        }
        if (v7 == 3) {
            unsigned int a, b, c;
            func_0012AB60(bs, 4);
            a = func_0012AB60(bs, 3);
            func_0012ABB0(bs);
            b = func_0012AB60(bs, 0xF);
            func_0012ABB0(bs);
            c = func_0012AB60(bs, 0xF);
            func_0012ABB0(bs);
            hdr->unk18 = ((long)((a >> 2) & 1) << 0x20) | (long)(unsigned int)(a << 30 | b << 15 | c);
        }
        if (flag1 == 1) {
            func_0012AB60(bs, 0x30);
        }
        if (v8 != 0) {
            func_0012AB60(bs, tbl.v[v8]);
        }
        if (v5 == 1) {
            int a1;
            int a2;
            int a3;
            int a4;
            int a5;
            a1 = func_0012AB60(bs, 1);
            a2 = func_0012AB60(bs, 1);
            a3 = func_0012AB60(bs, 1);
            a4 = func_0012AB60(bs, 1);
            func_0012AB60(bs, 3);
            a5 = func_0012AB60(bs, 1);
            if (a1 == v5) {
                func_0012AB60(bs, 0x30);
                func_0012AB60(bs, 0x30);
                func_0012AB60(bs, 0x20);
            }
            if (a2 == v5) {
                func_0012C468_a(ctx, D_00153AD8);
                return 0;
            }
            if (a3 == v5) {
                func_0012AB60(bs, 0x10);
            }
            if (a4 == v5) {
                func_0012AB60(bs, 0x10);
            }
            if (a5 == v5) {
                unsigned int i;
                unsigned int n;
                func_0012ABB0(bs);
                n = func_0012AB60(bs, 7);
                for (i = 0; i < n; i++) {
                    func_0012AB60(bs, 8);
                }
            }
        }
        now = bs->stamp;
        {
        int delta = len - (int)((now - base) >> 3);
        if (delta != 0) {
            func_0012ABF8(bs, delta);
        }
        }
        {
        int n = hdr->unk8 - len;
        int m;
        hdr->unk24 = n - 3;
        hdr->unk20 = (int)bs->stamp;
        if (hdr->unk0 == ((unsigned long)0xBD00 << 0x18)) {
            hdr->unk0 |= (unsigned long)(unsigned int)func_0012AB60(bs, 0x20);
            m = n - 7;
        } else {
            m = n - 3;
        }
        if (m != 0) {
            func_0012ABF8(bs, m);
        }
        }
    } else if (hdr->unk0 == ((unsigned long)0xBC00 << 0x18) ||
               hdr->unk0 == ((unsigned long)0xBF00 << 0x18) ||
               hdr->unk0 == ((unsigned long)0xF000 << 0x18) ||
               hdr->unk0 == ((unsigned long)0xF100 << 0x18) ||
               hdr->unk0 == ((unsigned long)0xFF00 << 0x18) ||
               hdr->unk0 == ((unsigned long)0xF200 << 0x18) ||
               hdr->unk0 == ((unsigned long)0xF800 << 0x18)) {
        int n = hdr->unk8;
        if (hdr->unk0 == ((unsigned long)0xBF00 << 0x18)) {
            n -= 4;
            hdr->unk0 |= (unsigned long)(unsigned int)func_0012AB60(bs, 0x20);
        }
        if (n != 0) {
            func_0012ABF8(bs, n);
        }
    } else if (hdr->unk0 == ((unsigned long)0xBE00 << 0x18)) {
        if (hdr->unk8 != 0) {
            func_0012ABF8(bs, hdr->unk8);
        }
    }
    return 1;
}

extern void func_0012D068(void);

/*
 * sceMpegInit (libmpeg.a:mpeg.o): one-time IOP/EE SIF hardware bring-up.
 * func_0011D960 presumably queries the DMA/SIF state, gating whether
 * func_0011D9A8 (a kick) runs; the SIF flag register at 0x1000F590 is set
 * then cleared (bit 0x10000, re-reading 0x1000F520 fresh each time), two
 * DMA channel control registers (0x1000B000/0x1000B400) have bit 0x100
 * masked off, and two more (0x1000B020/0x1000B420) are zeroed before the
 * tail call into func_0012D068.
 */
void func_0012B870(void) {
    int r = func_0011D960();

    *(volatile unsigned int *)0x1000F590 =
        *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B000 &= ~0x100;
    *(volatile unsigned int *)0x1000B400 &= ~0x100;
    *(volatile unsigned int *)0x1000F590 =
        *(volatile unsigned int *)0x1000F520 & ~0x10000;

    if (r != 0) {
        func_0011D9A8();
    }

    *(volatile unsigned int *)0x1000B020 = 0;
    *(volatile unsigned int *)0x1000B420 = 0;
    func_0012D068();
}

extern void func_0012C468(void *, void *);
/* memset returns its pointer: the unused return moves the first temporary
   after the call from $v0 to $v1. */
extern void *func_001153FC_p(void *, int, unsigned int) __asm__("func_001153FC");
extern char D_00153B10[];
extern void func_0012BD28(void *, int, int);
extern unsigned int func_0012BD60(void *, char *, unsigned int, unsigned int);
extern void func_0012C2F8(void *);
extern void func_0012BBA8(void *);
extern int func_0012BBF8(void *);
extern void func_0012BD40(void *);
extern void func_0012CC70(void);
extern void func_0012CC80(void);

/*
 * sceMpegCreate (libmpeg.a:mpeg.o): clear the caller's work area
 * (memset), align it up to 4 and carve the decoder object out of it. Less
 * than 0x10C0 usable bytes is reported through func_0012C468 and returns
 * 0. Otherwise: wrapper->0x40 = the object; a heap descriptor at +0x108
 * (func_0012BD28) covers the space past 0x10C0; the fields are reset
 * (-1 "unset" sentinels or 0); 0x600 bytes are allocated from that heap
 * (func_0012BD60) into +0x44; handler slots +0x1C/+0x24 get the two
 * callback trampolines (func_0012CC70/func_0012CC80); func_0012C2F8,
 * sceMpegReset (func_0012BBA8) and func_0012BBF8 run; the three 0x10-byte
 * entries at +0x1B8 get pointers into nine 0x68-byte regions; and
 * func_0012BD40 closes the heap. Success falls off the end (retail
 * leaves 0x70003600 in $v0).
 *
 * The trampoline store at +0x24 is written before the allocation so it
 * fills that call's delay slot, and the last four stores are in the
 * order that schedules as retail's.
 */
int func_0012B918(void *arg0, void *workarea, int size) {
    char *w = (char *)arg0;
    unsigned int wa = (unsigned int)workarea;
    char *aligned;
    char *area2;
    unsigned int avail;
    int neg1;

    func_001153FC_p(workarea, 0, size);

    aligned = (char *)(((wa + 3) >> 2) << 2);
    avail = (unsigned int)size - ((unsigned int)aligned - wa);

    if (avail < 0x10C0) {
        func_0012C468(aligned, D_00153B10);
        return 0;
    }

    area2 = aligned + 0x108;
    *(int *)(w + 0x40) = (int)aligned;
    avail -= 0x10C0;
    func_0012BD28(area2, (int)(aligned + 0x10C0), (int)avail);
    neg1 = -1;

    *(int *)(w + 0x0) = 0;
    *(int *)(w + 0x4) = 0;
    *(int *)(w + 0x8) = 0;
    *(long *)(w + 0x10) = -1;
    *(long *)(w + 0x18) = -1;
    *(long *)(w + 0x20) = 0;
    *(long *)(w + 0x28) = -1;
    *(long *)(w + 0x30) = -1;
    *(long *)(w + 0x38) = 0;

    *(int *)(aligned + 0xB4) = 0;
    *(int *)(aligned + 0xB8) = 0;
    *(int *)(aligned + 0xBC) = 0;
    *(int *)(aligned + 0xC0) = 0;
    *(int *)(aligned + 0xC4) = 0;
    *(int *)(aligned + 0xC8) = 0;
    *(int *)(aligned + 0xCC) = 0;
    *(int *)(aligned + 0xD0) = 0;
    *(int *)(aligned + 0xD4) = 0;
    *(int *)(aligned + 0xD8) = 0;
    *(int *)(aligned + 0xDC) = 0;
    *(int *)(aligned + 0xE0) = 0;
    *(int *)(aligned + 0xE4) = 0;
    *(int *)(aligned + 0xE8) = 0;
    *(int *)(aligned + 0xF8) = 0;
    *(int *)(aligned + 0xC) = 0;
    *(int *)(aligned + 0x14) = 0;
    *(int *)(aligned + 0x2C) = 0;
    *(int *)(aligned + 0x34) = 0;
    *(int *)(aligned + 0x3C) = 0;
    *(long *)(aligned + 0xF0) = -1;
    *(int *)(aligned + 0x1C) = (int)func_0012CC70;
    *(int *)(aligned + 0x24) = (int)func_0012CC80;

    *(int *)(aligned + 0x44) = (int)func_0012BD60(aligned, area2, 0x600, 8);

    *(int *)(aligned + 0x48) = 0;
    *(int *)(aligned + 0xFC) = 0;
    *(int *)(aligned + 0x100) = 0;
    *(int *)(aligned + 0x104) = 0;
    *(int *)(aligned + 0x70) = 0;
    *(long *)(aligned + 0x78) = 0;
    *(int *)(aligned + 0x80) = neg1;
    *(long *)(aligned + 0x88) = 0;
    *(int *)(aligned + 0x90) = 0;
    *(int *)(aligned + 0xAC) = 0;
    *(int *)(aligned + 0x94) = neg1;
    *(int *)(aligned + 0x98) = neg1;
    *(int *)(aligned + 0x9C) = neg1;
    *(int *)(aligned + 0x858) = (int)w;
    *(int *)(aligned + 0xB0) = 1;

    func_0012C2F8(aligned);
    func_0012BBA8(w);
    func_0012BBF8(w);

    *(int *)(aligned + 0x1B8) = (int)(aligned + 0x1E8);
    *(int *)(aligned + 0x1BC) = (int)(aligned + 0x250);
    *(int *)(aligned + 0x1C4) = (int)(aligned + 0x2B8);
    *(int *)(aligned + 0x1C8) = (int)(aligned + 0x320);
    *(int *)(aligned + 0x1CC) = (int)(aligned + 0x388);
    *(int *)(aligned + 0x1D4) = (int)(aligned + 0x3F0);
    *(int *)(aligned + 0x1D8) = (int)(aligned + 0x458);
    *(int *)(aligned + 0x1DC) = (int)(aligned + 0x4C0);
    *(int *)(aligned + 0x1E4) = (int)(aligned + 0x528);

    func_0012BD40(area2);

    *(int *)(aligned + 0x850) = neg1;
    *(int *)(aligned + 0x84C) = 0;
    *(int *)(aligned + 0x81C) = 0x70003600;
    *(int *)(aligned + 0x854) = 0;
}

int func_0012BB20(void) {
    return 1;
}

LINKER_REMNANT("asm/remnants/core_text", func_0012BB28);

extern int func_0012BDD0(void *);

int func_0012BB30(char *p, unsigned int addr, int size) {
    char *t = *(char **)(p + 0x40);

    *(int *)(t + 0xB0) = 1;
    *(int *)(t + 0xD8) = (addr & 0x0FFFFFFF) | 0x20000000;
    *(int *)(t + 0xE4) = size;
    *(int *)(t + 0xE0) = 0;
    *(int *)(t + 0xDC) = 0;
    return func_0012BDD0(p);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012BB78);

int func_0012BB88(char *p) {
    return **(int **)(p + 0x40);
}

int func_0012BB98(void *arg0) {
    return ((Wrapper *)arg0)->obj->unk004 == 0;
}

extern void func_0012C358(void *arg0);
extern void func_0012C268(void *arg0);

/*
 * sceMpegReset (libmpeg.a:mpeg.o): clear the object's decode state
 * (unk000/004/008/0AC, the handler-table slot at +0x80, and +0x118), and
 * the wrapper's own +0x08, then call _clearEach (func_0012C358) and hand
 * the object to func_0012C268 (zero +0x848, kick the IPU chain via
 * func_00127378).
 */
void func_0012BBA8(void *arg0) {
    char *w = (char *)arg0;
    char *inner = *(char **)(w + 0x40);

    *(int *)(inner + 0x0) = 0;
    *(int *)(inner + 0x4) = 0;
    *(int *)(inner + 0x8) = 0;
    *(int *)(w + 0x8) = 0;
    *(int *)(inner + 0xAC) = 0;
    *(int *)(inner + 0x80) = -1;
    func_0012C358(inner);
    *(int *)(inner + 0x118) = 0;
    func_0012C268(inner);
}

int func_0012BBF8(void *arg0) {
    char *b = *(char **)((char *)arg0 + 0x40);
    char *p;
    p = *(char **)(b + 0x1B8); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1C8); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1D8); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1BC); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1CC); if (p != 0) *(int *)(p + 0x28) = 0;
    p = *(char **)(b + 0x1DC); if (p != 0) *(int *)(p + 0x28) = 0;
    return 1;
}

/* The callback table at +0x40: {callback, data} pairs from +0xC, one
   per callback type. */
typedef int (*MpegCbFn)(void *, void *, void *);
typedef struct { MpegCbFn func; void *data; } MpegCbEnt;
typedef struct { char pad[0xC]; MpegCbEnt cb[1]; } MpegCbTbl;

/*
 * sceMpegAddCallback: install (callback arg2, data arg3) for type arg1 and
 * return the old callback.
 *
 * Typing the entry as {function pointer, void *} is the lever: 2.9-ee's
 * type-based alias analysis (-fstrict-aliasing is its default) then knows
 * the `data` store cannot alias the `func` load, so sched1 computes
 * `t + 0xC` before `t + off` and the registers fall as in retail. Under
 * 2.95.3 (no type-based aliasing) the same C is 17/36; the old int-offset
 * spelling was 12/36 under both.
 */
int func_0012BC50(void *arg0, int arg1, int arg2, int arg3) {
    MpegCbTbl *t = *(MpegCbTbl **)((char *)arg0 + 0x40);
    MpegCbFn old;
    t->cb[arg1].data = (void *)arg3;
    old = t->cb[arg1].func;
    t->cb[arg1].func = (MpegCbFn)arg2;
    return (int)old;
}

/*
 * Dispatch the callback registered for *arg1 (the callback data's type
 * field) with its data; returns the callback's result, 0 if none.
 *
 * The table entry is indexed afresh for each use (test `.func`, call
 * `.func(arg0, arg1, .data)`) instead of through one `entry` pointer. CSE
 * merges the two `.func` loads, but the `.data` address is rebuilt after
 * the branch, which is retail's second `addu` in a plain beqz slot, and
 * `ret` then lands in $a3 with the table in $a2. Exact under both
 * compilers (one `entry` pointer is SIZE 84/80; the old spelling 12/80).
 */
void *func_0012BC78(void *arg0, int *arg1) {
    int ret = 0;
    if (arg0 != 0) {
        MpegCbTbl *t = *(MpegCbTbl **)((char *)arg0 + 0x40);
        if (t != 0) {
            if (t->cb[*arg1].func != 0) {
                ret = t->cb[*arg1].func(arg0, arg1, t->cb[*arg1].data);
            }
        }
    }
    return (void *)ret;
}

/* No declaration needed: the definition above precedes this caller. The
   extern that used to sit here guessed `void (int, void *)` and now
   conflicts with the real signature. */
void func_0012BCC8(int arg0) {
    int local[8];
    local[0] = 1;
    func_0012BC78(arg0, local);
}

LINKER_REMNANT("asm/remnants/core_text", func_0012BCF0);

void func_0012BD28(void *arg0, int arg1, int arg2) {
    int *p = (int *)arg0;
    p[1] = arg2;
    p[0] = arg1;
    p[2] = arg1;
    p[3] = arg1;
}

void func_0012BD40(void *arg0) {
    char *p = (char *)arg0;
    *(int *)(p + 0xC) = *(int *)(p + 0x8);
}

void func_0012BD50(void *arg0) {
    char *p = (char *)arg0;
    *(int *)(p + 0x8) = *(int *)(p + 0xC);
}

extern void func_0012C468(void *, void *);
extern char D_00153B38[];

/* Bump allocator out of a region {base, size, used}: round `used` up to
   `align`, reserve `size` bytes, and hand back the aligned offset. On
   overflow it reports through func_0012C468 and returns 0 WITHOUT
   touching `used`.

   The fitting case is written first (`>=`): 2.9-ee then branches to the
   report on the true side (bnel) and puts the `used` store in the slot
   of the fall-through `b`, as retail. Written overflow-first, as 2.95.3
   needed, 2.9-ee inverts the test and puts the store in a beql slot,
   one instruction short (SIZE 104/108). */
unsigned int func_0012BD60(void *arg0, char *r, unsigned int size,
                           unsigned int align) {
    unsigned int aligned;
    unsigned int end;

    aligned = ((*(unsigned int *)(r + 0x8) + align - 1) / align) * align;
    end = aligned + size;
    if (*(unsigned int *)(r + 0x0) + *(unsigned int *)(r + 0x4) >= end) {
        *(unsigned int *)(r + 0x8) = end;
        return aligned;
    }
    func_0012C468(arg0, D_00153B38);
    return 0;
}

extern int func_00128C90(void *);
extern int func_0012C200(void *);
extern int func_0012C058(void *, int, int);
/* _errMessage (func_0012C430) sprintf's its format and passes the
   format's arguments on untouched in $a1-$a3, so it is called here
   without a prototype, with the value the "%08x" reports. */
extern void func_0012C430();
extern char D_00153B58[];

/*
 * _getpic (libmpeg.a:mpeg.o): fetch and start decoding one picture.
 * The image buffer (inner +0xD8) must be 64-byte aligned; if not, it is
 * reported through _errMessage with the address and the call fails with
 * -1. Otherwise loop: fetch headers (func_00128C90) until one yields no
 * picture (0), the picture counter reaches its target (+0x174 == +0xD4)
 * or the fault flag (+0x848) clears, skipping the fetch when the last
 * decode returned -1. picture_coding_type 1/2/3 call func_0012C058 with
 * that type's running index (+0xA0/+0xA4/+0xA8, I resetting all three)
 * and slot (+0x94/+0x98/+0x9C) and bump the index; type 4 shares the B
 * arm (retail's jump table maps 3 and 4 to one handler); 0 ends the
 * stream through func_0012C200 and sets inner->0; anything above 4 does
 * nothing. Returns 1 once +0x820 or inner->0 is set.
 *
 * pictureType's initializer and lastResult declared first put both in
 * retail's saved registers and hoist lastResult's 0 into the prologue.
 */
int func_0012BDD0(void *arg0) {
    int pictureType = 1;
    int lastResult = 0;
    char *w = (char *)arg0;
    char *inner = *(char **)(w + 0x40);

    *(int *)inner = 0;
    if ((*(int *)(inner + 0xD8) & 0x3F) != 0) {
        func_0012C430(inner, D_00153B58, *(int *)(inner + 0xD8));
        return -1;
    }
    *(int *)(inner + 0x820) = 0;

    do {
        if (lastResult != -1) {
            for (;;) {
                pictureType = func_00128C90(inner);
                if (pictureType == 0) break;
                if (*(int *)(inner + 0x174) == *(int *)(inner + 0xD4)) break;
                if (*(int *)(inner + 0x848) == 0) break;
            }
        }

        switch (pictureType) {
        case 0:
            func_0012C200(w);
            *(int *)inner = 1;
            break;
        case 1:
            *(int *)(inner + 0xA8) = 0;
            *(int *)(inner + 0xA4) = 0;
            *(int *)(inner + 0xA0) = 0;
            lastResult = func_0012C058(w, 0, *(int *)(inner + 0x94));
            *(int *)(inner + 0xA0) = *(int *)(inner + 0xA0) + 1;
            break;
        case 2:
            lastResult = func_0012C058(w, *(int *)(inner + 0xA4), *(int *)(inner + 0x98));
            *(int *)(inner + 0xA4) = *(int *)(inner + 0xA4) + 1;
            break;
        case 3:
        case 4:
            lastResult = func_0012C058(w, *(int *)(inner + 0xA8), *(int *)(inner + 0x9C));
            *(int *)(inner + 0xA8) = *(int *)(inner + 0xA8) + 1;
            break;
        }

        if (*(int *)(inner + 0x820) != 0) {
            return 1;
        }
    } while (*(int *)inner == 0);

    return 1;
}

extern int func_00129690(void *, int);
extern int func_00129530(void *);
extern void func_00129600(void *, int, int);

/* _decodeOrSkipFrame (libmpeg): sibling of func_0012C0A0
   (_decodeOrSkipField), for the frame-picture path. Adapted from
   Lombyte (MIT) for PAL. */
int func_0012BF40(void *arg0, int arg1, int arg2) {
    Wrapper *w = (Wrapper *)arg0;
    Obj40 *p = w->obj;
    int flag = 0;
    int t;
    int ret;

    if (arg2 == -1 || arg1 < arg2) {
        if (p->unk008 == 0) {
            w->unk08 = 0;
            p->unk008 = 1;
        }
        if (func_00129690(p, 0) == 0) {
            t = 0;
        } else {
            t = 0;
            t = func_00129530(p) != t;
        }
        ret = t;
    } else {
        flag = 1;
        ret = func_00129690(p, 0);
        func_0012BCC8((int)w);
    }
    func_00129600(p, p->unk118, p->unk004);
    if (p->unk174 != 3 && flag == 0) {
        p->unk120 = (p->unk120 == 0);
    }
    w->unk08 = p->unk118 - p->unk0AC;
    if (p->unk120 == 0) {
        p->unk118 = p->unk118 + 1;
        p->unk004 = p->unk004 + 1;
    }
    return ret;
}

extern int func_0012C0A0(void *, int, int);
extern int func_0012BF40(void *, int, int);

/* Hands the call on by the inner object's +0x174 mode: func_0012BF40 for
   mode 3, func_0012C0A0 otherwise, with the caller's arguments.

   Returns its callees' values: retail keeps the frame and calls both, and
   2.9-ee tail-calls each arm (bare `j`, SIZE 40/68) when it is void. */
int func_0012C058(void *arg0, int arg1, int arg2) {
    Obj40 *inner = ((Wrapper *)arg0)->obj;
    if (inner->unk174 != 3) {
        return func_0012C0A0(arg0, arg1, arg2);
    } else {
        return func_0012BF40(arg0, arg1, arg2);
    }
}

struct MpegFieldDec
{
  int unk0;
  int unk4;
  int unk8;
  unsigned char pad_C[0xA0];
  int unkAC;
  unsigned char pad_B0[0x24];
  int unkD4;
  unsigned char pad_D8[0x40];
  int unk118;
  int unk11C;
  int unk120;
  unsigned char pad_124[0x50];
  int unk174;
};
struct MpegHandle
{
  unsigned char pad_0[0x8];
  int unk8;
  unsigned char pad_C[0x34];
  struct MpegFieldDec *unk40;
};
extern int func_00129530();
extern int func_00129690();
extern void func_00129600();
/* _decodeOrSkipField (libmpeg). Adapted from Lombyte (MIT) for PAL. */
int func_0012C0A0(void *handle, int arg1, int arg2)
{
  struct MpegHandle *arg0 = handle;
  struct MpegFieldDec *p;
  int decode;
  unsigned int ref;
  int want;
  long r;
  int ret;
  int gate;
  decode = 0;
  p = arg0->unk40;
  p->unk120 = 0;
  if ((arg2 == (-1)) || (arg1 < arg2))
  {
    decode = 1;
  }
  if (p->unk8 == 0)
  {
    arg0->unk8 = 0;
    p->unk8 = 1;
  }
  r = func_00129690(p, 0);
  if ((r != 0) && (decode != 0))
  {
    func_00129530(p);
  }
  p->unk120 = 1;
  r = func_00128C90(p);
  if (r == 0)
  {
    func_0012C200(arg0);
    p->unk0 = 1;
    return 0;
  }
  want = 2;
  if (p->unkD4 != 1)
  {
    want = 1;
  }
  if (p->unk174 != want)
  {
    return -1;
  }
  ref = func_00129690(p, 1);
  gate = 0;
  if (ref != 0)
  {
    gate = 1;
  }
  ret = 0;
  if (gate != 0)
  {
    if (decode == 0)
    {
      goto out;
    }
    r = func_00129530(p);
    if (r != 0)
    {
      ret = 1;
    }
  }
  out:
  func_00129600(p, p->unk118, p->unk4);

  p->unk120 = 0;
  arg0->unk8 = p->unk118 - p->unkAC;
  p->unk118 = p->unk118 + 1;
  p->unk4 = (unsigned long long) (p->unk4 + 1);
  if (decode == 0)
  {
    func_0012BCC8((int)arg0);
  }
  return ret;
}

extern void func_0012C278(void *);

int func_0012C200(void *arg0) {
    Wrapper *w = (Wrapper *)arg0;
    Obj40 *inner = w->obj;
    int ret = 0;

    if (inner->unk004 != 0 && inner->unk008 != 0) {
        func_0012C278((void *)inner);
        w->unk08 = inner->unk118 - inner->unk0AC;
        inner->unk004 = 0;
        ret = 1;
    }
    return ret;
}

/* Tail call: the constant argument setup lands in the jump's delay slot,
   so the field store precedes it. func_00127378 is defined above. */
void func_0012C268(void *arg0) {
    *(int *)((char *)arg0 + 0x848) = 0;
    func_00127378(1);
}

extern void func_00129E30(void *, int, int, int);
extern char D_00153BB8[];

void func_0012C278(void *arg0) {
    Obj40 *s = (Obj40 *)arg0;
    int n = s->unk118;

    if (s->unk120 != 0) {
        func_0012C468(s, D_00153BB8);
    } else if (s->unk174 == 3) {
        func_00129E30(s, (int)s->slots[0].unk04, n - 1, n - 1);
    } else {
        func_00129F40(s, (int)s->slots[1].unk04, (int)s->slots[2].unk04);
    }
    s->unk120 = 0;
}

/*
 * Point the object's four scratchpad pointers at 0x70000000 and clear a
 * flag. Exact under 2.9-ee. Under 2.95.3 it was 2 of 24 words off:
 * that compiler saved $ra before $s1 in the prologue, where retail (and
 * 2.9-ee) saves $s1 first -- the prologue-order residual of the core
 * spill rewrite, not a source question.
 *
 * What does matter: `int a = 0x70000000;` must be written
 * BEFORE the call. An earlier round reverted this at 8 bytes short
 * having tried binding the constants to locals declared AFTER the call,
 * which changes nothing because gcc folds them straight back into the
 * stores. Declared before the call, the pseudo's live range crosses the
 * call, so the allocator gives it a CALLEE-SAVED register and the
 * function pays retail's sd/ld $s1 pair -- 8 bytes. gcc still
 * rematerialises the `lui` after the call, exactly as retail does, so
 * the only trace of the earlier definition is the register class.
 *
 * That is the general point: a constant hoisted above a call does not
 * survive as a value (constant propagation puts it back), but it does
 * survive as a register-class decision. Where retail spends a
 * callee-saved register on something that looks like it needs no
 * register at all, the source defined it before the call.
 */
void func_0012C2F8(void *arg0) {
    char *p = (char *)arg0;
    int a = 0x70000000;

    func_00127378(1);
    *(int *)(p + 0x590) = a;
    *(int *)(p + 0x594) = 0x70001800;
    *(int *)(p + 0x6D0) = 0x70001B00;
    *(int *)(p + 0x6D4) = 0x70003300;
    *(int *)(p + 0x810) = 0;
}

extern void func_0012CF98(int, int);

/*
 * _clearEach (libmpeg.a:init.o): per-stream hardware reset shared by
 * sceMpegInit's shape -- flag the bitstream state "in use" (+0x818 = 1),
 * clear its slice-address latch (+0x1B0), then the same SIF-flag toggle
 * (0x1000F590 set/cleared around bit 0x10000, re-reading 0x1000F520 fresh
 * each time) and three DMA channel registers zeroed (0x1000B000/B400/D400),
 * gated IOP kick via func_0011D960/func_0011D9A8, three more registers
 * zeroed (0x1000B020/B420/D420), IPU_CTRL (0x10002010) set to 0x40000000,
 * and a tail call into func_0012CF98(0, 0).
 */
void func_0012C358(void *arg0) {
    char *s = (char *)arg0;
    int r;

    *(int *)(s + 0x818) = 1;
    *(int *)(s + 0x1B0) = 0;
    r = func_0011D960();

    *(volatile unsigned int *)0x1000F590 =
        *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B000 = 0;
    *(volatile unsigned int *)0x1000B400 = 0;
    *(volatile unsigned int *)0x1000D400 = 0;
    *(volatile unsigned int *)0x1000F590 =
        *(volatile unsigned int *)0x1000F520 & ~0x10000;

    if (r != 0) {
        func_0011D9A8();
    }

    *(volatile unsigned int *)0x1000B020 = 0;
    *(volatile unsigned int *)0x1000B420 = 0;
    *(volatile unsigned int *)0x1000D420 = 0;
    *(volatile unsigned int *)0x10002010 = 0x40000000;
    func_0012CF98(0, 0);
}

extern char D_00153BD8[];
/* Unprototyped: func_0011A6C8 is a varargs definition (blocked as such),
   but calling one is fine -- only defining one needs stdarg.h. */
extern void func_0011A6C8();

void func_0012C420(void *arg0) {
    func_0011A6C8(D_00153BD8, arg0);
}

extern void func_00116248(void *);

void func_0012C430(void *arg0) {
    char buf[0x100];
    func_00116248(buf);
    func_0012C468(arg0, buf);
}

extern void func_0012C420(void *);

void func_0012C468(void *arg0, void *arg1) {
    char *a = (char *)arg0;
    void *t = *(void **)(a + 0x858);
    /* the arg0 null test is retail's, after it has already dereferenced
       arg0 -- one of the "dead-looking guards" that must be written out */
    if (t != 0 && arg0 != 0 && *(int *)(a + 0xC) != 0) {
        int buf[4];
        buf[0] = 0;
        buf[1] = (int)arg1;
        func_0012BC78(t, buf);
    } else {
        func_0012C420(arg1);
    }
}

/* Store the two sizes and their 16-pixel macroblock counts. (An old note
   here recorded 4/32 with the 0x4/0x8 stores swapped; the function is
   exact under both compilers now.) */
int func_0012C4C0(void *arg0, int arg1, int arg2) {
    char *p = (char *)arg0;
    *(int *)(p + 0x4) = arg1;
    *(int *)(p + 0x8) = arg2;
    *(int *)(p + 0xC) = arg1 >> 4;
    *(int *)(p + 0x10) = arg2 >> 4;
    return 1;
}

extern int func_00128A58(void *, int);
extern void func_0012C468(void *, void *);
extern char D_00153BE8[];
extern void func_00128560(void *, unsigned int);
extern void func_0012C990(void *, int, int);
extern char D_001330C0[];
extern char D_00133100[];
extern void func_00128E68(void *);
extern void func_0012C608(void *);

/*
 * _sequenceHeader (libmpeg.a:init.o): ISO/IEC 13818-2 6.2.2.1
 * sequence_header(), hardware-VLC variant matching mpeg2decode's
 * gethdr.c almost line for line but reading combined bitfields in one
 * Get_Bits (func_00128A58) call each: a 32-bit fetch for
 * horizontal_size/vertical_size/aspect_ratio_information/frame_rate_code
 * (only the two sizes are kept, at +0x124/+0x128; a vertical_size
 * reading >= 0xAF1 is reported through func_0012C468) and a 30-bit fetch
 * for bit_rate_value/marker_bit/vbv_buffer_size/constrained_parameters_
 * flag (bit_rate_value kept at +0x134, vbv_buffer_size at +0x138).
 * load_intra/non_intra_quantizer_matrix (+0x840/+0x844) gate the IPU
 * matrix load (func_00128590/func_00128560) or, when clear, _setDefaultQM
 * (func_0012C990) with the reference decoder's default table. Ends with
 * extensionAndUserData (func_00128E68) and a tail call into _initSeq
 * (func_0012C608) with the object read back from +0x858.
 */
void func_0012C4E0(void *arg0) {
    char *s = (char *)arg0;
    unsigned int v;
    int mid;
    unsigned int flag;

    *(unsigned int *)(s + 0xD4) = 0;

    v = func_00128A58(s, 0x20);
    *(unsigned int *)(s + 0x124) = v >> 20;
    mid = (v >> 8) & 0xFFF;
    *(unsigned int *)(s + 0x128) = mid;
    if (mid >= 0xAF1) {
        func_0012C468(s, D_00153BE8);
    }

    v = func_00128A58(s, 0x1E);
    *(unsigned int *)(s + 0x134) = v >> 12;
    *(unsigned int *)(s + 0x138) = (v >> 1) & 0x3FF;

    flag = func_00128A58(s, 1);
    *(unsigned int *)(s + 0x840) = flag;
    if (flag != 0) {
        func_00128590(s);
        func_00128560(s, 0x50000000);
        func_00128590(s);
    } else {
        func_0012C990(s, 0x50000000, (int)D_001330C0);
    }

    flag = func_00128A58(s, 1);
    *(unsigned int *)(s + 0x844) = flag;
    if (flag != 0) {
        func_00128590(s);
        func_00128560(s, 0x58000000);
        func_00128590(s);
    } else {
        func_0012C990(s, 0x58000000, (int)D_00133100);
    }

    func_00128E68(s);
    func_0012C608(*(void **)(s + 0x858));
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012C608);

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012C8B0);

/* _setDefaultQM: tell the callback (type 2) that the default quantiser
   matrix load starts, then with the IPU idle and reset, DMA the matrix
   at addr to IPU_TO (tag 4 qwords, chcr 0x101) with interrupts held
   off, send command cmd and wait, and report type 3. Adapted from
   Lombyte (MIT) for PAL. */
void func_0012C990(void *arg, int cmd, int addr) {
    char *mp = arg;
    int cb[8];
    int intr;

    cb[0] = 2;
    func_0012BC78(*(void **)(mp + 0x858), cb);
    func_00128590(mp);
    *(volatile int *)0x10002000 = 0;
    func_00128590(mp);
    intr = func_0011D960();
    *(volatile int *)0x1000B410 = addr & 0x0FFFFFFF;
    *(volatile int *)0x1000B420 = 4;
    *(volatile int *)0x1000B400 = 0x101;
    if (intr != 0) {
        func_0011D9A8();
    }
    func_00128560(mp, cmd);
    func_00128590(mp);
    cb[0] = 3;
    func_0012BC78(*(void **)(mp + 0x858), cb);
}

extern void func_0012C468(void *, void *);
extern char D_00153C00[];
extern char D_00153C28[];

/*
 * _sequenceExtension (libmpeg.a:init.o): ISO/IEC 13818-2 6.2.2.3
 * sequence_extension(), matching mpeg2decode's gethdr.c almost line for
 * line but reading two combined bitfields with func_00128A58: a 28-bit
 * fetch covers profile_and_level_indication(8) + progressive_sequence(1)
 * + chroma_format(2) + horizontal_size_extension(2) +
 * vertical_size_extension(2) + bit_rate_extension(12) + marker_bit(1); a
 * 16-bit fetch covers vbv_buffer_size_extension(8) + low_delay(1) +
 * frame_rate_extension_n(2) + frame_rate_extension_d(5) (the last three
 * read and dropped). chroma_format must be 1 (4:2:0) and
 * profile_and_level_indication one of three literal bytes (0x48/0x58/
 * 0x44); either failing reports through func_0012C468. Ends by folding
 * the extension bits into sequenceHeader's fields exactly as the
 * reference decoder's
 * `horizontal_size = (horizontal_size_extension<<12) | (horizontal_size&0xfff)`
 * et al.
 */
void func_0012CA70(void *arg0) {
    char *s = (char *)arg0;
    unsigned int v1, v2;
    unsigned int chroma_format, h_ext, v_ext, bitrate_ext;
    unsigned int progressive, profile, vbv_ext;

    *(int *)(s + 0x848) = 1;
    func_00127378(0);

    v1 = func_00128A58(s, 0x1C);
    bitrate_ext = (v1 >> 1) & 0xFFF;
    chroma_format = (v1 >> 17) & 3;
    v_ext = (v1 >> 13) & 3;
    h_ext = (v1 >> 15) & 3;

    *(unsigned int *)(s + 0x140) = chroma_format;
    if (chroma_format != 1) {
        func_0012C468(s, D_00153C00);
    }

    progressive = (v1 >> 19) & 1;
    *(unsigned int *)(s + 0x13C) = progressive;
    profile = v1 >> 20;

    v2 = func_00128A58(s, 0x10);
    vbv_ext = v2 >> 8;

    if (profile != 0x48 && profile != 0x58 && profile != 0x44) {
        func_0012C468(s, D_00153C28);
    }

    *(int *)(s + 0x124) = (h_ext << 12) | (*(int *)(s + 0x124) & 0xFFF);
    *(int *)(s + 0x128) = (v_ext << 12) | (*(int *)(s + 0x128) & 0xFFF);
    *(int *)(s + 0x134) += bitrate_ext << 18;
    *(int *)(s + 0x138) += vbv_ext << 10;
}


/*
 * MPEG-2 sequence_display_extension(): video_format (3 bits), then
 * colour_description (1) and, when set, colour_primaries,
 * transfer_characteristics and matrix_coefficients (8 each; the last is
 * kept at +0x144); display_horizontal_size (14) to +0x148, a marker
 * bit, display_vertical_size (14) to +0x14C.
 *
 * Plain C, exact under 2.9-ee. (Under 2.95.3 it needed the marker bit's
 * read to go through a void view of the reader, `func_00128A58_v`: as a
 * value call it reset $v0's readers and the +0x148 store lost the
 * scheduler tie to the next call's argument setup, 8/140.)
 */
void func_0012CBA0(void *arg0) {
    char *s = (char *)arg0;

    func_00128A58(s, 3);
    if (func_00128A58(s, 1) != 0) {
        func_00128A58(s, 8);
        func_00128A58(s, 8);
        *(int *)(s + 0x144) = func_00128A58(s, 8);
    }
    *(int *)(s + 0x148) = func_00128A58(s, 0xE);
    func_00128A58(s, 1);
    *(int *)(s + 0x14C) = func_00128A58(s, 0xE);
}

extern char D_00153C48[];
extern char D_00153C78[];
extern char D_00153C90[];

/* Three more of the func_0012C468 family, same shape as func_0012CC60. */
void func_0012CC30(void *arg0) {
    func_0012C468(arg0, D_00153C48);
}

void func_0012CC40(void *arg0) {
    func_0012C468(arg0, D_00153C78);
}

void func_0012CC50(void *arg0) {
    func_0012C468(arg0, D_00153C90);
}

extern char D_00153CC8[];

/* Tail call: arg0 passes straight through, arg1 is &D_00153CC8 whose
   %lo half retail schedules into the jump's delay slot. */
void func_0012CC60(void *arg0) {
    func_0012C468(arg0, D_00153CC8);
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012CC6C);
