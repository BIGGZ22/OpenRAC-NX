#include "common.h"
#include "structs.h"

extern long r1_func_0012AC80(int arg0, int arg1);

typedef struct {
    long key;
    long extra;
    int  cb;
    int  data;
} StrCbEnt;

 /* 0x18 */

typedef struct { char pad[8]; long extra; } Tbl16;

extern Tbl16 r1_D_00132FD8[];

extern int r1_func_0012AAA8(void *, int);

extern int r1_func_0012AB60(void *arg0, int arg1);

extern void r1_func_0012ABB0(void *arg0);

extern int func_0012F070();

INCLUDE_ASM("asm/core", func_0012EAA0);

INCLUDE_ASM("asm/core", func_0012EB28);

/* rac1-decomp src/core/0012AC80.c: func_0012B008 */
/*
 * sceMpegAddStrCallback (libmpeg.a:pack.o): key = r1_func_0012AC80(arg1,
 * arg2) (a 64-bit stream/type key); linear-search the object's callback
 * table (handlers[7] reused as {table pointer, count} -- raw offsets,
 * not the Handler struct, per this file's warning about that array
 * overlapping) for a matching key, remembering its old callback (0 if
 * none found). Whether found or not (index lands on the match, or one
 * past the end), if there's room (< 0x40 entries), overwrite the slot
 * with the new key/data/callback and the type's extra 8 bytes from
 * r1_D_00132FD8[arg1], and bump the stored count unconditionally. Returns
 * the previous callback.
 */
int func_0012EE28(void *arg0, int arg1, int arg2, int arg3, int arg4) {
    int old = 0;
    char *obj = *(char **)((char *)arg0 + 0x40);
    StrCbEnt *tbl = *(StrCbEnt **)(obj + 0x44);
    long key = r1_func_0012AC80(arg1, arg2);
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
        tbl[i].extra = r1_D_00132FD8[arg1].extra;
    }
    return old;
}

/* rac1-decomp src/core/0012AC80.c: func_0012B100 */
/*
 * _pack_header (libmpeg.a:pack.o): ISO/IEC 11172-1 2.4.3.3 pack_header().
 * r1_func_0012AB60 is nextBit ("get n bits"), r1_func_0012ABB0 a marker-bit
 * skip, r1_func_0012AAA8 a 32-bit peek (nextStartCode-style). Reads the pack
 * start code's fixed fields (34 then 3, marker, 15, marker, 15, marker,
 * 9, marker), discarding the SCR/mux_rate pieces except for three: the
 * low bit of the first 3-bit group goes to +0x8, and the three groups
 * combine into +0x4 as (first<<30)|(second<<15)|third. The last field
 * read (3 bits) is pack_stuffing_length: that many stuffing bytes are
 * then skipped. Peeks the next 32 bits; a system_header_start_code
 * (0x1BB) sets +0xC and hands off into func_0012F070 (system_header
 * parsing -- called prototype-less here since retail's call site passes
 * it a second, unused argument, the same K&R situation as r1_func_00129F40
 * elsewhere in this file), otherwise clears +0xC. Always returns 1.
 */
int func_0012EF20(void *arg0, void *arg1) {
    char *p = (char *)arg1;
    unsigned int first, second, third, stuffing;
    unsigned int i;

    r1_func_0012AB60(arg0, 0x22);
    first = r1_func_0012AB60(arg0, 3);
    r1_func_0012ABB0(arg0);
    second = r1_func_0012AB60(arg0, 0xF);
    r1_func_0012ABB0(arg0);
    third = r1_func_0012AB60(arg0, 0xF);
    r1_func_0012ABB0(arg0);
    *(int *)(p + 0x0) = r1_func_0012AB60(arg0, 9);
    r1_func_0012AB60(arg0, 0x1E);
    stuffing = r1_func_0012AB60(arg0, 3);

    *(unsigned int *)(p + 0x8) = (first >> 2) & 1;
    *(unsigned int *)(p + 0x4) = (first << 30) | (second << 15) | third;

    for (i = 0; i < stuffing; i++) {
        r1_func_0012AB60(arg0, 8);
    }

    if (r1_func_0012AAA8(arg0, 0x20) == 0x1BB) {
        *(int *)(p + 0xC) = 1;
        func_0012F070(arg0, p);
    } else {
        *(int *)(p + 0xC) = 0;
    }
    return 1;
}
