/* 0 of 1 places match - last line: 14_aranos_prison: instruction at +0x4c differs
   Candidate size is EXACT (160 bytes) and the first 19 stores are byte-identical: the whole
   body is the corner block of a quad written into the record - four rows of
   (0, -2, -4, 1) / (0, -2, 4, 1) / (0, 2, -4, 1) / (0, 2, 4, 1) at 0x00/0x10/0x20/0x30, a
   half-word group at 0x50..0x6C, a 64-bit zero at 0x70 and the 64-bit pair
   0x0000FF9000000260 at 0x80.  Only the ORDER of the store groups differs: this cc1 shuffles
   the groups relative to the source (measured with three source orders - the emitted
   sequence is not the source order, nor its reverse, nor a rotation of it; each order moved
   the divergence one group further or earlier).  Retail's emitted order is
   0x3C, 0x80, [0x28 0x38 0x14 0x34], [0x70 0x50 0x54 0x58 0x5C 0x60 0x64 0x68 0x6C],
   [0x08 0x18 0x04 0x24], [0x00 0x10 0x20 0x30], [0x0C 0x1C 0x2C].  The form below is the one
   that came closest (divergence at +0x4c).  Unblocking needs the group order the scheduler
   actually derives - or a struct-typed initialiser the maintainers would accept. */

typedef unsigned char u8;
typedef unsigned long long u64;
typedef int s32;
typedef float f32;

void func_L14_003F0E98(u8 *a0) {
    *(u64 *)(a0 + 0x80) = 0x0000FF9000000260ULL;
    *(f32 *)(a0 + 0x08) = -4.0f;
    *(f32 *)(a0 + 0x18) = 4.0f;
    *(f32 *)(a0 + 0x04) = -2.0f;
    *(f32 *)(a0 + 0x24) = 2.0f;
    *(u64 *)(a0 + 0x70) = 0;
    *(s32 *)(a0 + 0x50) = 0;
    *(s32 *)(a0 + 0x54) = 0;
    *(f32 *)(a0 + 0x58) = 1.0f;
    *(s32 *)(a0 + 0x5C) = 0;
    *(s32 *)(a0 + 0x60) = 0;
    *(f32 *)(a0 + 0x64) = 1.0f;
    *(f32 *)(a0 + 0x68) = 1.0f;
    *(f32 *)(a0 + 0x6C) = 1.0f;
    *(f32 *)(a0 + 0x28) = -4.0f;
    *(f32 *)(a0 + 0x38) = 4.0f;
    *(f32 *)(a0 + 0x14) = -2.0f;
    *(f32 *)(a0 + 0x34) = 2.0f;
    *(s32 *)(a0 + 0x00) = 0;
    *(s32 *)(a0 + 0x10) = 0;
    *(s32 *)(a0 + 0x20) = 0;
    *(s32 *)(a0 + 0x30) = 0;
    *(f32 *)(a0 + 0x0C) = 1.0f;
    *(f32 *)(a0 + 0x1C) = 1.0f;
    *(f32 *)(a0 + 0x2C) = 1.0f;
    *(f32 *)(a0 + 0x3C) = 1.0f;
}
