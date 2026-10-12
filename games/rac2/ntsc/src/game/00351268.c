

typedef int s32;

typedef unsigned char u8;

/* func_003512B8 : rounds a field to a multiple of 2048 under a semaphore */
extern s32 func_0011AC60(s32);

extern s32 func_0011AC40(s32);

/* func_00351268 : waits, then combines two fields shifted by 11 */
extern s32 func_0011AC60(s32 a0);

extern s32 func_0011AC40(s32 a0);

extern int func_00351268(unsigned char *owner);

s32 func_00351268(u8 *a0) {
    s32 x;
    func_0011AC60(*(s32 *)(a0 + 0x40));
    x = (*(s32 *)(a0 + 0x10) << 11) + *(s32 *)(a0 + 0x14);
    func_0011AC40(*(s32 *)(a0 + 0x40));
    return x;
}
