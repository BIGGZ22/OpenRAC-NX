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
extern int IsT10K(void);
extern void GetOsdConfigParam(int *config);
extern short D_001330D0[] __attribute__((section(".data")));

int sceScfGetTimeZone(void) {
    int config;
    int timezone;


    if (IsT10K() != 0) {
        timezone = D_001330D0[0];
    } else {
        GetOsdConfigParam(&config);
        timezone = config >> 21;
        if ((((unsigned int)config >> 13) & 7) == 0) {
            timezone = 0x21C;
        }
    }
    return timezone;
}
