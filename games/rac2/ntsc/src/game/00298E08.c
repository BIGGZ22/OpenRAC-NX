#include "include_asm.h"

/* Default -G0 profile, no cflags line. D_001A7B90 and the scalars are `sda`: cc1 emits them as
   bare symbols (one `la` for the buffer, a plain load or store for each scalar) and the
   assembler expands them in place, which is retail's shape. D_0014B540 is a struct global read
   through its members so that cc1 forms its base address in $s0 directly instead of copying it
   from a temporary. Measured: declaring func_002B7CE0 as void changes the schedule, and the
   five stores at the end are written in an order that schedules to retail's. */
typedef struct {
    char pad_00[0x32C];
    int f32C;
    char pad_330[0x10];
    int f340;
    int f344;
} Table_0014B540;
typedef struct {
    char pad_00[0x18];
    short f18;
    char pad_1A[0x162];
    int f17C;
} State_001393E0;
extern Table_0014B540 D_0014B540;
extern State_001393E0 D_001393E0;
extern char D_001A7B90[] __attribute__((sda));
extern int D_001A79F0 __attribute__((sda));
extern int D_001A7290 __attribute__((sda));
extern int D_001A72C0 __attribute__((sda));
extern int D_001A7424 __attribute__((sda));
extern int D_00152C00[];

extern void func_00288970(int, void *);
extern int func_002B7CE0(int);
extern void func_002B7C80(int, int, int);
extern void func_00282AB0(void *, void *, int);
extern void func_00298D28(char *);

void func_00298E08(void)
{
    char buf[40];
    char *p;
    char *data;

    func_00288970(D_0014B540.f344 << 11, &p);
    func_002B7CE0(1);
    func_002B7C80((int)p, D_0014B540.f340 + D_0014B540.f32C, D_0014B540.f344);
    data = p;
    data += *(int *)(data + 0x10);
    func_00282AB0(buf, D_001A7B90, 40);
    func_00298D28(data);
    func_00282AB0(D_001A7B90, buf, 40);
    D_001A79F0 = 0;
    if (D_001393E0.f17C != 0)
        D_001393E0.f17C = 0;
    if (D_001393E0.f18 >= 0)
        D_001393E0.f18 = -1;
    D_001A7290 = -1;
    D_001A7424 &= ~0x200;
    D_001A72C0 = -1;
    D_00152C00[12] = -1;
    D_00152C00[6] = -1;
}
