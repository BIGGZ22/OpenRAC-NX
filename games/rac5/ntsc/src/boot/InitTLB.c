typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
typedef void VOID;
#define NULL ((void *)0)


extern s32 GetMemorySize();
extern s32 _InitTLB();
extern s32 InitTLB32MB();

s32 InitTLB(void) {
    s32 result;

    if (GetMemorySize() == 0x2000000) {
        result = InitTLB32MB();
    } else {
        result = _InitTLB();
    }
    return result;
}
