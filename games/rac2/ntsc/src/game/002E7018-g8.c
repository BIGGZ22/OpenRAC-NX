#include "include_asm.h"

/* Small data: the three tables the function reaches through $gp. Their addresses
   differ from program to program, so they carry no level address and are named
   after the function's own place in level 00. */
extern unsigned int ref_L00_00361F20_00 __attribute__((sda));
extern unsigned int ref_L00_00361F20_01 __attribute__((sda));
extern unsigned int ref_L00_00361F20_02 __attribute__((sda));

extern int D_001A79F0;
extern unsigned long long D_001B2160;
extern float D_00261EC0[];

extern void func_002FC560(unsigned int, unsigned long long);
extern void func_00282B50(unsigned char *, unsigned char *, int);
extern void func_00282D30(unsigned char *, unsigned char *, float);
extern void func_00282CC0(unsigned char *, unsigned char *, unsigned char *);
extern void func_00280B90(unsigned char *, int, int);

/* Builds the four quads of one draw call: the scale comes from the resident
   mode word indexing a float table, every quad carries the grey 0x80808080,
   and the block copied out of small data also carries the four render packet
   words. */
void func_002E8190(void)
{
    unsigned char verts[4][16];
    int colors[4];
    unsigned char buf[0x40];
    unsigned long long packet;
    unsigned int mode;
    float scale = 1.0f;
    int i;

    func_002FC560(0x47, 0x31801);

    packet = D_001B2160;

    *(unsigned long long *)(buf + 0x20) = 0;
    *(unsigned long long *)(buf + 0x28) = packet;
    *(unsigned long long *)(buf + 0x30) = 0xFF9000000260ULL;
    *(unsigned long long *)(buf + 0x38) = 0x8000000044ULL;
    func_00282B50(buf, (unsigned char *)&ref_L00_00361F20_00, 0x20);

    mode = D_001A79F0;
    if (mode < 0x13) {
        scale = D_00261EC0[mode];
    }

    for (i = 0; i < 4; i++) {
        colors[i] = 0x80808080;
        func_00282D30(verts[i], (unsigned char *)&ref_L00_00361F20_01 + i * 16, scale);
        func_00282CC0(verts[i], verts[i], (unsigned char *)&ref_L00_00361F20_02);
    }

    func_00280B90(verts[0], 0, 0);
    func_002FC560(0x47, 0x5360B);
}
