#include "include_asm.h"

extern int D_001B1B40 __attribute__((section(".sdata")));
extern int D_001B1B44 __attribute__((section(".sdata")));
extern unsigned char D_001CE4E0[];
extern short D_001CE300[];
extern char *D_001CDB80[];
extern int D_001D0E00[];
extern void func_002B65B8(int, int);
extern void func_002934F8(char *, int, int, int);

void func_00293890(char *object, int a, int b, int id)
{
    if (object == 0) {
        func_002B65B8(id, 1);
        D_001CE4E0[id] = *(unsigned char *)&D_001B1B44;
        D_001B1B44 = D_001B1B44 + 1;
    } else {
        D_001CE300[D_001B1B40] = id;
        D_001CE4E0[id] = *(unsigned char *)&D_001B1B40;
        D_001CDB80[D_001B1B40] = object;
        D_001D0E00[D_001B1B40] = (unsigned char)object[0x2D] << 10;
        if ((unsigned char)object[0x2D] == 255)
            D_001D0E00[D_001B1B40] = 0x100000;
        func_002B65B8(id, 0);
        ++D_001B1B40;
        func_002934F8(object, a, b, id);
    }
}


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* The part list a moby hangs its parts from. */
struct PartList {
    u8 pad00[6];
    u8 flag;             /* 0x06 */
    u8 pad07[5];
    u8 used;             /* 0x0C */
    u8 pad0D[0x3B];
    u32 entries[1];      /* 0x48 */
};

/* What the render globals point at for the colour a new moby takes. */
struct ColorSrc {
    u8 pad00[0x38];
    u64 color;           /* 0x38 */
};

struct RenderGlobals {
    u8 pad00[0x2290];
    struct ColorSrc *color_src;    /* 0x2290 */
};

/* A moby: the engine's object. */
struct Moby {
    u8 pad00[0x24];
    struct PartList *parts;    /* 0x24 */
    u8 pad28[0x0A];
    u16 unk32;                /* 0x32 */
    u16 unk34;                /* 0x34 */
    u8 pad36[2];
    u64 color;                /* 0x38 */
    u8 pad40[2];
    u8 idx;                   /* 0x42 */
    u8 slot;                  /* 0x43 */
    u8 pad44[0x1E];
    u8 flag72;                /* 0x62 */
    u8 flag73;                /* 0x63 */
    u8 pad64[4];
    u32 model;                /* 0x68 */
    u8 pad6C[0x2C];
    u32 unk94;                /* 0x98 */
    u8 pad9C[0x0E];
    short class_id;           /* 0xAA */
};

/* The model record of a chunk: its parts, in the order the record lists them. */
struct ModelRec {
    u16 flags;        /* 0x00 */
    u8 pad02[2];
    u32 part_count;   /* 0x04 */
    u16 pad08;        /* 0x08 */
    u8 pad0A[2];
    u16 num_parts;    /* 0x0C */
    u8 pad0E[2];
    u32 end_off;      /* 0x10 */
    int part_off[1];  /* 0x14 */
};

/* The state of the world objects: the record in hand and the mobies built from it. */
struct TransferState {
    u8 pad00[0x38];
    u32 unk38;            /* 0x38 */
    u8 pad3C[4];
    u16 unk40;            /* 0x40 */
    u8 pad42[2];
    short num_parts;      /* 0x44 */
    u8 pad46[2];
    u16 unk48;            /* 0x48 */
    u8 pad4A[2];
    u32 unk4C;            /* 0x4C */
    u32 unk50;            /* 0x50 */
    u32 unk54;            /* 0x54 */
    u8 pad58[0x10];
    u32 unk68;            /* 0x68 */
    struct ModelRec *rec; /* 0x6C */
    u32 unk70;            /* 0x70 */
    u8 pad74[0x118];
    struct Moby *slots[1];/* 0x18C */
};

extern struct TransferState D_001B8840;
extern struct RenderGlobals D_00189E20;
extern struct Moby *D_001B159C __attribute__((sda));

extern void func_0011AEA0(int);
extern void func_0029D4D0(int, int);
extern struct Moby *func_0029EFD0(int);
extern void func_0029F0C8(struct Moby *, int);

/* UpdateWorldObjectAnimation: relocate the model record loaded for the current chunk and
   hook each of its parts to a moby, spawning one and initialising it the first time; each
   part's pointer table is made absolute. */
void func_00293D30(void)
{
    struct ModelRec *rec;
    struct Moby *mob;
    int *cp;
    int *sp;
    int i;
    int k;
    int idx;
    int id;
    int off;
    int endp;
    int pc;

    func_0011AEA0(0);
    func_0029D4D0(D_001B8840.unk70, (int)D_001B8840.rec);
    func_0011AEA0(0);

    rec = D_001B8840.rec;
    D_001B8840.unk38 = 0;
    cp = rec->part_off;
    D_001B8840.unk40 = rec->flags;
    pc = rec->part_count;
    D_001B8840.unk48 = rec->pad08;
    D_001B8840.num_parts = rec->num_parts;
    D_001B8840.unk68 = (u32)rec + rec->end_off;
    if (pc < 0x400) {
        D_001B8840.unk4C = 0;
    } else {
        D_001B8840.unk4C = (u32)rec + pc;
    }

    for (i = 0; i < D_001B8840.num_parts; i++) {
        off = *cp++;
        sp = (int *)((u8 *)rec + off);
        id = sp[0];
        sp = (int *)((u8 *)sp + 0xC);
        endp = (int)rec + sp[0];
        sp = (int *)((u8 *)sp + 4);
        if (id == 0x215 || id == 0x10D1) {
            id = 0xD54;
        }
        mob = D_001B8840.slots[i];
        if (mob == 0) {
            if (id == 0) {
                mob = D_001B159C;
                func_0029F0C8(mob, 0);
            } else {
                mob = func_0029EFD0(id);
            }
            mob->unk32 = 0x1FF;
            mob->unk34 |= 6;
            mob->flag72 = 0xFF;
            mob->unk94 = 0;
            if (D_00189E20.color_src == 0) {
                mob->color = 0x38383800000000ULL;
            } else {
                mob->color = D_00189E20.color_src->color;
            }
            if (mob->parts->flag) {
                mob->flag73 = 0x18;
            }
            idx = mob->parts->used;
            mob->parts->used = idx + 1;
            mob->idx = idx;
            mob->slot = idx;
            D_001B8840.slots[i] = mob;
            if (mob->class_id == 0xD54) {
                D_001B8840.unk54 = (u32)mob;
            }
        }
        mob->model = endp;
        mob->parts->entries[mob->idx] = (u32)sp;
        for (k = 0; k < ((u8 *)sp)[0x10]; k++) {
            int *w = (int *)((u8 *)sp + 0x1C);

            w[k] = (int)sp + w[k];
        }
    }
}
