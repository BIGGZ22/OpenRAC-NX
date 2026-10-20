#include "include_asm.h"

extern void func_00282D30_2(char *dst, char *src, float scale);

extern void func_L00_003DC820_1(char *pkt);

extern long long func_0027C878_1(char *p);

extern float func_00283248_3(float x, float y);

extern float func_00282E20_2(char *p);

extern void func_00283410_3(float *matrix, float *quat);

extern void func_00282F20_3(float *off, char *src, float scale);

extern void func_L00_003DC8A8_1(char *pkt, float angle);

extern void func_00280B90_1(char *pkt, float *matrix, int mode);

extern void ref_L00_003E0F58_00(char *);

extern void ref_L00_003E0F58_01(char *, float);

extern void func_L00_003E0CE8_1(char *pkt);

extern void func_L00_003E0D70_1(char *pkt, float angle);

extern long long func_002E67A0_1(char *p);

extern float func_802EE7A8_1(float x, float y);

extern float func_802EE2E8_1(char *p, float *v);

extern void func_802EE970_1(float *matrix, float *quat);

extern void func_802EE138_1(float *off, float *v, char *p);

extern void func_802EE3D0_1(float *dst, float *src, float angle);

extern void func_802EAC30_1(char *pkt, float *matrix, int mode);

extern void func_L00_003DFF98_1(char *p);

extern void func_L00_003E0020_1(char *p, float scale);

/* The vector the aim helpers read, at 0x001B8C80. */
typedef struct {
    char pad_000[0x140];
    float x_140;
    float y_144;
    float z_148;
} L00AimVector;

extern L00AimVector D_L00_001B8C80;

INCLUDE_ASM("asm/levels", func_L00_003D37E8);

INCLUDE_ASM("asm/levels", func_L00_003D3DE0);

INCLUDE_ASM("asm/levels", func_L00_003D3FA8);

INCLUDE_ASM("asm/levels", func_L00_003D3FB0);

INCLUDE_ASM("asm/levels", func_L00_003D40D8);

INCLUDE_ASM("asm/levels", func_L00_003D4548);

INCLUDE_ASM("asm/levels", func_L00_003D4958);

INCLUDE_ASM("asm/levels", func_L00_003D4A20);


INCLUDE_ASM("asm/levels", func_L00_003D5358);

/* shared native body of family a04928c6 */
void func_L00_003D6FB8(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L00_003D7250);

/* shared native body of family a04928c6 */
void func_L00_003DA2B0(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L00_003DB0A8);

INCLUDE_ASM("asm/levels", func_L00_003DB118);

INCLUDE_ASM("asm/levels", func_L00_003DBF50);

INCLUDE_ASM("asm/levels", func_L00_003DC1C0);

INCLUDE_ASM("asm/levels", func_L00_003DC350);

INCLUDE_ASM("asm/levels", func_L00_003DC3E0);

INCLUDE_ASM("asm/levels", func_L00_003DC640);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003DC820(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L00_003DC8A8(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

/* Builds a 144-byte packet, aims a quaternion at the shared 0x001B8C80 vector from pos through three float helpers, derives an offset basis by a fourth, writes the last matrix row from pos plus that offset, then dispatches the packet. */
void func_L00_003DC910(char *pos, int a1, int a2, char *a3, int t0,
                       float fA, float fB, float fC)
{
    char pkt[144];
    float m[16];
    float quat[4];
    float off[4];
    long long r;

    func_L00_003DC820_1(pkt);
    r = func_002E67A0_1(a3);

    *(long long *)(pkt + 136) = ((long long)t0 << 2) | 0x8000000040LL;
    *(long long *)(pkt + 128) = 280993940374112LL;
    *(int *)(pkt + 76) = a1 + (a2 << 24);
    *(int *)(pkt + 72) = a1 + (a2 << 24);
    *(int *)(pkt + 68) = a1 + (a2 << 24);
    *(int *)(pkt + 64) = a1 + (a2 << 24);
    *(long long *)(pkt + 112) = 0LL;
    *(long long *)(pkt + 120) = r;

    quat[3] = 0.0f;
    quat[2] = func_802EE7A8_1(D_L00_001B8C80.x_140 - *(float *)(pos + 0),
                              D_L00_001B8C80.y_144 - *(float *)(pos + 4));
    quat[1] = -func_802EE7A8_1(func_802EE2E8_1(pos, &D_L00_001B8C80.x_140),
                               D_L00_001B8C80.z_148 - *(float *)(pos + 8));
    quat[0] = fB;

    func_802EE970_1(m, quat);
    func_802EE138_1(off, &D_L00_001B8C80.x_140, pos);
    func_802EE3D0_1(off, off, fC);

    m[12] = *(float *)(pos + 0) + off[0];
    m[13] = *(float *)(pos + 4) + off[1];
    m[14] = *(float *)(pos + 8) + off[2];
    m[15] = 1.0f;

    func_L00_003DC8A8_1(pkt, fA);
    func_802EAC30_1(pkt, m, 0);
}

/* Builds a 144-byte packet, derives a quaternion from source floats through four float helpers, writes the last matrix row from a source point plus a computed offset, then dispatches the packet. */
void func_L00_003DCAC8(char *pos, int a1, int a2, char *a3, int t0,
                                        char *src, float f12, float f13)
{
    char pkt[144];
    float m[16];
    float quat[4];
    float off[4];
    long long r;

    func_L00_003DC820_1(pkt);
    r = func_0027C878_1(a3);

    *(int *)(pkt + 76) = a1 + (a2 << 24);
    *(int *)(pkt + 72) = a1 + (a2 << 24);
    *(int *)(pkt + 68) = a1 + (a2 << 24);
    *(int *)(pkt + 64) = a1 + (a2 << 24);
    *(long long *)(pkt + 112) = 0LL;
    *(long long *)(pkt + 120) = r;
    *(long long *)(pkt + 128) = 280993940374112LL;
    *(long long *)(pkt + 136) = ((long long)t0 << 2) | 0x8000000040LL;

    quat[3] = 0.0f;
    quat[2] = func_00283248_3(*(float *)(src + 0), *(float *)(src + 4));
    quat[1] = -func_00283248_3(func_00282E20_2(src), *(float *)(src + 8));
    quat[0] = f13;

    func_00283410_3(m, quat);
    func_00282F20_3(off, src, 0.025f);

    m[12] = *(float *)(pos + 0) + off[0];
    m[13] = *(float *)(pos + 4) + off[1];
    m[14] = *(float *)(pos + 8) + off[2];
    m[15] = 1.0f;

    func_L00_003DC8A8_1(pkt, f12);
    func_00280B90_1(pkt, m, 0);
}

INCLUDE_ASM("asm/levels", func_L00_003DCC48);

INCLUDE_ASM("asm/levels", func_L00_003DD2B8);

void func_L00_003DD680(char *p)
{
    if (*(unsigned char *)(p + 0x20) == 0) {
        *(unsigned short *)(p + 0x34) |= 0x800;
        *(unsigned char *)(p + 0x20) = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L00_003DD6A8);

INCLUDE_ASM("asm/levels", func_L00_003DDEC8);

INCLUDE_ASM("asm/levels", func_L00_003DE088);

INCLUDE_ASM("asm/levels", func_L00_003DE258);

INCLUDE_ASM("asm/levels", func_L00_003DE490);

INCLUDE_ASM("asm/levels", func_L00_003DE950);

INCLUDE_ASM("asm/levels", func_L00_003DF338);

INCLUDE_ASM("asm/levels", func_L00_003DF710);

INCLUDE_ASM("asm/levels", func_L00_003DFDC0);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003DFF98(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L00_003E0020(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

/* Builds a 144-byte packet, aims a quaternion at the shared 0x001B8C80 vector from pos through three float helpers, derives an offset basis by a fourth, writes the last matrix row from pos plus that offset, then dispatches the packet. */
void func_L00_003E0088(char *pos, int a1, int a2, char *a3, int t0,
                       float fA, float fB, float fC)
{
    char pkt[144];
    float m[16];
    float quat[4];
    float off[4];
    long long r;

    func_L00_003DFF98_1(pkt);
    r = func_002E67A0_1(a3);

    *(long long *)(pkt + 136) = ((long long)t0 << 2) | 0x8000000040LL;
    *(long long *)(pkt + 128) = 280993940374112LL;
    *(int *)(pkt + 76) = a1 + (a2 << 24);
    *(int *)(pkt + 72) = a1 + (a2 << 24);
    *(int *)(pkt + 68) = a1 + (a2 << 24);
    *(int *)(pkt + 64) = a1 + (a2 << 24);
    *(long long *)(pkt + 112) = 0LL;
    *(long long *)(pkt + 120) = r;

    quat[3] = 0.0f;
    quat[2] = func_802EE7A8_1(D_L00_001B8C80.x_140 - *(float *)(pos + 0),
                              D_L00_001B8C80.y_144 - *(float *)(pos + 4));
    quat[1] = -func_802EE7A8_1(func_802EE2E8_1(pos, &D_L00_001B8C80.x_140),
                               D_L00_001B8C80.z_148 - *(float *)(pos + 8));
    quat[0] = fB;

    func_802EE970_1(m, quat);
    func_802EE138_1(off, &D_L00_001B8C80.x_140, pos);
    func_802EE3D0_1(off, off, fC);

    m[12] = *(float *)(pos + 0) + off[0];
    m[13] = *(float *)(pos + 4) + off[1];
    m[14] = *(float *)(pos + 8) + off[2];
    m[15] = 1.0f;

    func_L00_003E0020_1(pkt, fA);
    func_802EAC30_1(pkt, m, 0);
}

INCLUDE_ASM("asm/levels", func_L00_003E0240);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003E0CE8(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L00_003E0D70(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

/* Builds a 144-byte packet, derives a quaternion from source floats through four float helpers, writes the last matrix row from a source point plus a computed offset, then dispatches the packet. */
void func_L00_003E0DD8(char *pos, int a1, int a2, char *a3, int t0,
                                        char *src, float f12, float f13)
{
    char pkt[144];
    float m[16];
    float quat[4];
    float off[4];
    long long r;

    func_L00_003E0CE8_1(pkt);
    r = func_0027C878_1(a3);

    *(int *)(pkt + 76) = a1 + (a2 << 24);
    *(int *)(pkt + 72) = a1 + (a2 << 24);
    *(int *)(pkt + 68) = a1 + (a2 << 24);
    *(int *)(pkt + 64) = a1 + (a2 << 24);
    *(long long *)(pkt + 112) = 0LL;
    *(long long *)(pkt + 120) = r;
    *(long long *)(pkt + 128) = 280993940374112LL;
    *(long long *)(pkt + 136) = ((long long)t0 << 2) | 0x8000000040LL;

    quat[3] = 0.0f;
    quat[2] = func_00283248_3(*(float *)(src + 0), *(float *)(src + 4));
    quat[1] = -func_00283248_3(func_00282E20_2(src), *(float *)(src + 8));
    quat[0] = f13;

    func_00283410_3(m, quat);
    func_00282F20_3(off, src, 0.025f);

    m[12] = *(float *)(pos + 0) + off[0];
    m[13] = *(float *)(pos + 4) + off[1];
    m[14] = *(float *)(pos + 8) + off[2];
    m[15] = 1.0f;

    func_L00_003E0D70_1(pkt, f12);
    func_00280B90_1(pkt, m, 0);
}

/* Builds a 144-byte packet, aims a quaternion at the shared 0x001B8C80 vector from pos through three float helpers, derives an offset basis by a fourth, writes the last matrix row from pos plus that offset, then dispatches the packet. */
void func_L00_003E0F58(char *pos, int a1, int a2, char *a3, int t0,
                       float fA, float fB, float fC)
{
    char pkt[144];
    float m[16];
    float quat[4];
    float off[4];
    long long r;

    ref_L00_003E0F58_00(pkt);
    r = func_002E67A0_1(a3);

    *(long long *)(pkt + 136) = ((long long)t0 << 2) | 0x8000000040LL;
    *(long long *)(pkt + 128) = 280993940374112LL;
    *(int *)(pkt + 76) = a1 + (a2 << 24);
    *(int *)(pkt + 72) = a1 + (a2 << 24);
    *(int *)(pkt + 68) = a1 + (a2 << 24);
    *(int *)(pkt + 64) = a1 + (a2 << 24);
    *(long long *)(pkt + 112) = 0LL;
    *(long long *)(pkt + 120) = r;

    quat[3] = 0.0f;
    quat[2] = func_802EE7A8_1(D_L00_001B8C80.x_140 - *(float *)(pos + 0),
                              D_L00_001B8C80.y_144 - *(float *)(pos + 4));
    quat[1] = -func_802EE7A8_1(func_802EE2E8_1(pos, &D_L00_001B8C80.x_140),
                               D_L00_001B8C80.z_148 - *(float *)(pos + 8));
    quat[0] = fB;

    func_802EE970_1(m, quat);
    func_802EE138_1(off, &D_L00_001B8C80.x_140, pos);
    func_802EE3D0_1(off, off, fC);

    m[12] = *(float *)(pos + 0) + off[0];
    m[13] = *(float *)(pos + 4) + off[1];
    m[14] = *(float *)(pos + 8) + off[2];
    m[15] = 1.0f;

    ref_L00_003E0F58_01(pkt, fA);
    func_802EAC30_1(pkt, m, 0);
}

INCLUDE_ASM("asm/levels", func_L00_003E2180);

INCLUDE_ASM("asm/levels", func_L00_003E2510);

INCLUDE_ASM("asm/levels", func_L00_003E2BD0);

INCLUDE_ASM("asm/levels", func_L00_003E2E80);

