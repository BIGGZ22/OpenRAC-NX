/* NON_MATCHING func_L01_002F0728 -- src/overlays/shared/vendor_002B90A8.c
 * Best so far: SIZE ours 292 / retail 296, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Finds, in the owner's class-list (D_L01_001ABFC0[owner[0x21]], ushort ids with bit 15 as end mark), the moby i
 *   p2.c matches the prologue, the class map (movz), the found-block and the epilogue, but is 284 of 296 bytes: re
 *   for(;;), do-while, for(;;p++), a v-before test and v=0 header loop give 284/288 bytes; none yields that layout
 *   x04 round (p8-p13): p8.c fixes the layout with gotos (`goto body; next: if ((short)v < 0) return 0; p++; body:
 */
extern unsigned short *D_L01_001ABFC0[];
extern short D_L01_00160058;
extern void func_L00_00251E30(void *);

/* finds the linked moby of the partner class in the owner's list and copies the owner's state onto it */
char *func_L01_002F0728(char *owner) {
    int cls = *(short *)(owner + 0xA6);
    int id = -1;
    unsigned short *p;
    unsigned int v;
    char *base;
    if (cls == 0x23C) {
        id = 0x361;
    } else if (cls == 0x361) {
        id = 0x362;
    }
    p = D_L01_001ABFC0[((unsigned char *)owner)[0x21]];
    base = *(char **)&D_L01_00160058;
    if (p == 0) {
        return 0;
    }
    goto body;
next:
    if ((short)v < 0) return 0;
    p++;
body:
    {
        char *moby;
        v = *p;
        moby = base + ((v & 0x7FFF) << 8);
        if (((unsigned char *)moby)[0x20] != 0xC) goto next;
        if (*(short *)(moby + 0xA6) != id) goto next;
        {
            char *data = *(char **)(moby + 0x78);
            char *odata = *(char **)(owner + 0x78);
            moby[0x30] = owner[0x30];
            *(unsigned short *)(moby + 0x32) = *(unsigned short *)(owner + 0x32);
            *(unsigned long *)(moby + 0x38) = *(unsigned long *)(owner + 0x38);
            moby[0x31] = 1;
            *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
            *(unsigned short *)(moby + 0x34) = *(unsigned short *)(owner + 0x34);
            qcopy(moby + 0x10, owner + 0x10);
            qcopy(moby + 0x40, owner + 0x40);
            *(float *)(data + 0x20) = *(float *)(odata + 0x20);
            *(float *)(data + 0x224) = *(float *)(odata + 0x224);
            func_L00_00251E30(moby);
            return moby;
        }
    }
}
