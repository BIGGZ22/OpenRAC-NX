/* from src/game/0028BDB0.c: 72-byte near miss; 6 of 18 words differ.
   The stores, their order and the two halfword stores are all right (the two
   halfword stores have to be written in the reverse order in the source, this
   cc1 emits them the other way round), but the scheduler places the constant
   materialisations differently: retail materialises 8, 0xD2, -2, 0xC8, 0x1E
   and this cc1 8, 0xC8, 0xD2, 0x1E, -2, and it puts the store of the gp
   address before the store of the 8 where retail has it after.  The register
   allocation follows from that order.  The body needs the -G8 profile (two
   $gp-relative objects), so it belongs in 0028BDB0-g8.c. */
extern int D_001A8DD8[1] __attribute__((sda));
extern int D_001A8D48 __attribute__((sda));
extern int D_001B18D0;
extern int D_001B18D4;

void func_0028CC98(void *a0)
{
    D_001B18D0 = 8;
    D_001B18D4 = (int)D_001A8DD8;
    *(int *)((char *)a0 + 0x58) = 0xD2;
    *(int *)((char *)a0 + 0x78) = 0x1E;
    *(int *)((char *)a0 + 0x5C) = 0xC8;
    *(int *)((char *)a0 + 0x74) = -2;
    *(short *)((char *)a0 + 0x4A) = 0;
    *(short *)((char *)a0 + 0x48) = 0;
    D_001A8D48 = 0;
}
