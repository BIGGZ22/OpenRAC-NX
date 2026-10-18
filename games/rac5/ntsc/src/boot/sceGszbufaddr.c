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

typedef struct {
    s16 sceGsInterMode;
    s16 sceGsOutMode;
    s16 sceGsFFMode;
    s16 sceGsVersion;
    volatile s32 (*sceGsVSCfunc)(s32);
    s32 sceGsVSCid;
} sceGsGParam;

extern sceGsGParam *GetCoreDataTable(void);

s16 sceGszbufaddr(s16 psm, s16 w, s16 h) {
    sceGsGParam *gp;
    s32 fw;
    s32 fh;

    gp = GetCoreDataTable();
    fw = (w + 63) / 64;
    if (psm & 2) {
        fh = (h + 63) / 64;
    } else {
        fh = (h + 31) / 32;
    }
    if ((*(u64 *)gp & 0x0000FFFF0000FFFFULL) == 1) {
        return fw * fh;
    }
    return fw * fh * 2;
}
