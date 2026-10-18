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

extern u8 D_001330D6[];
extern void GetOsdConfigParam();
extern s32 GetOsdConfigParam2();
extern s32 IsT10K();
struct M2c_SummerWork { u32 config; u8 summer; };
u8 sceScfGetSummerTime(void) {
    struct M2c_SummerWork work;
    u8 result;

    if (IsT10K() != 0) {
        result = D_001330D6[0];
    } else {
        GetOsdConfigParam(&work.config);
        if (((work.config >> 13) & 7) == 0) {
            result = 0;
        } else {
            GetOsdConfigParam2(&work.summer, 1, 1);
            result = (work.summer >> 4) & 1;
        }
    }
    return result;
}
