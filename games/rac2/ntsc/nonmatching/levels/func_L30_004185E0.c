/* 0 of 1 places match - last line: 30_insomniac_museum: 144 bytes, expected 140
   Refreshes the owner's 0x50 object: run the two vector helpers on its 0x10 block
   (func_2F5130(local, 0x004005BC, D_L30_001B2100) - the float argument comes from
   $gp - then func_2F4E68(local, local, 0x004005BC - 0x230)), call the object's
   0x64 method with the object when it is set, then func_3194F0 on the object.
   Retail keeps the 0x004005BC base in $s0 across both calls and derives the second
   constant as `addiu $a2,$s0,-0x230`, i.e. one symbol with a negative index, and
   saves s0-s3 + $ra in a 0x30 frame. Measured: the one-base form (below) and the
   two-explicit-locals form both give 144 bytes with a 0x30 frame; the plain form
   with the second constant written as its own address gives 136 bytes and a 0x20
   frame. Blocker: the last four bytes of the frame/schedule. */
typedef char *ptr;
typedef int s32;
typedef float f32;

extern void func_00282F20(char *, char *, f32);
extern void func_00282CC0(char *, char *, char *);
extern void func_002A0F48(void *);
extern char D_L30_004005BC[];
extern f32 D_L30_001B2100 __attribute__((sda));

void func_L30_004185E0(char *a0)
{
    char *s3 = *(char **)(a0 + 0x68);
    char *s2 = *(char **)(s3 + 0x50);
    char *s1 = s2 + 0x10;

    func_00282F20(s1, D_L30_004005BC, D_L30_001B2100);
    func_00282CC0(s1, s1, D_L30_004005BC - 0x230);
    if (*(s32 *)(s2 + 0x64) != 0)
        (*(void (**)(char *))(*(char **)(s3 + 0x50) + 0x64))(s2);
    func_002A0F48(*(void **)(s3 + 0x50));
}
