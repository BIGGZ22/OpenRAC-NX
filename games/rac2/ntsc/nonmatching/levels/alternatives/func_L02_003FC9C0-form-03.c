/* Alternative source form for func_L02_003FC9C0.
   Retained from v11-b3.c, entry d1; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

typedef struct {
    u32 w[4];
} V4;

extern void func_002A7750(void *, s32, s32);
extern void func_L23_004278D8(u8 *);
extern f32 D_L23_001B36FC __attribute__((sda));

extern void *func_L00_002D70A0(s32);
extern void func_L00_002D7140(void *);
extern f32 D_L02_001B1234 __attribute__((sda));
extern f32 D_L02_001B1238 __attribute__((sda));
extern f32 D_L02_001B123C __attribute__((sda));
extern f32 D_L02_001B1240 __attribute__((sda));
extern f32 D_L02_001B1244 __attribute__((sda));
extern f32 D_L02_001B1248 __attribute__((sda));
extern f32 D_L02_001B124C __attribute__((sda));
extern f32 D_L02_001B1250 __attribute__((sda));
extern f32 D_L02_001B1254 __attribute__((sda));

extern void func_002A8F70(void *, void *, void *, f32, V4 *);
extern void func_002A90C8(void *, void *, void *, f32, V4 *);
extern s32 D_L23_001B13D4 __attribute__((sda));

void func_L02_003FC9C0(u8 *a0) {
    u8 *s0 = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x20) == 8) {
        u8 *obj = *(u8 **)(s0 + 0xC0);

        if (obj == 0) {
            *(u8 **)(s0 + 0xC0) = func_L00_002D70A0(5);
        } else {
            u8 *src = *(u8 **)(s0 + 0xD0);
            f32 f0, f1, f2;

            if (src == 0) {
                f0 = D_L02_001B1234;
                f1 = D_L02_001B1238;
                f2 = D_L02_001B123C;
            } else if (*(s16 *)(src + 0xAA) == 0x9F8) {
                f0 = D_L02_001B1240;
                f1 = D_L02_001B1244;
                f2 = D_L02_001B1248;
            } else if (*(s16 *)(src + 0xAA) == 0xA44) {
                f0 = D_L02_001B124C;
                f1 = D_L02_001B1250;
                f2 = D_L02_001B1254;
            } else {
                return;
            }
            *(f32 *)(obj + 0) = f0;
            *(f32 *)(*(u8 **)(s0 + 0xC0) + 4) = f1;
            *(f32 *)(*(u8 **)(s0 + 0xC0) + 8) = f2;
        }
    } else {
        func_L00_002D7140(*(u8 **)(s0 + 0xC0));
        *(u8 **)(s0 + 0xC0) = 0;
    }
}
