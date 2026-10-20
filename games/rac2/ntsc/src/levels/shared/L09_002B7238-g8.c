#include "include_asm.h"
#include "common.h"

/* The level's resident spawn counters, in small data. */
extern int D_L09_001B0F50 __attribute__((sda));
extern int D_L09_001B0F54 __attribute__((sda));
extern int D_L09_001B0F58 __attribute__((sda));
extern unsigned short D_L09_001B0F4C __attribute__((sda));

extern int func_002A7820(int a0, int a1);
extern int func_00282978(char *a0);
extern void func_002B0990(int a0, void *a1, int a2, int a3, f32 f12);

/* Small data the run's step needs: the resident mode word (0x001A79F0), the level's
   release word (0x001A9040) and the level's record table (0x001B285C). */
extern int D_001A79F0 __attribute__((sda));
extern int D_L09_001A9040 __attribute__((sda));
extern char *D_L09_001B285C __attribute__((sda));

extern void func_L09_003EA798(char *object);
extern float func_00282E48(char *a, char *b);
extern void func_002A93B0(void *object);
extern char D_L09_001B9A00[];
extern int func_002B0ED8(char *object, char *frame, int flag);
extern char *func_L09_00326E90(char *entry);
extern void func_002A7750(char *object, int a1, int a2);
extern void func_002B3BA8(char *object, int a1, int a2, int a3, f32 f12, f32 f13);

/* Steps the run: refreshes the object's frame, drops the record it still points at when
   the resident mode word says 9, and rebuilds the effect. */
void func_L09_003EAA18(char *object) {
    char *frame = *(char **)(object + 0x68);
    short id;
    int result;

    if (*(unsigned char *)(object + 0x95) == 0)
        func_L09_003EA798(object);

    if (*(unsigned char *)(object + 0x31) != 0) {
        if (func_00282E48(object + 0x10, D_L09_001B9A00) < 32.0f) {
            func_002A93B0(object);
            *(unsigned char *)(object + 0x6F) = 0x1E;
        }
    }

    result = func_002B0ED8(object, frame + 0x180, 0);
    if (result >= 4) {
        if (D_001A79F0 == 9) {
            id = *(short *)(frame + 0x1F0);
            if (id != -1) {
                if (func_L09_00326E90(D_L09_001B285C + (id << 8)) == 0)
                    D_L09_001A9040 = 0;
                *(short *)(frame + 0x1F0) = -1;
            }
        }
        func_002A7750(object, 6, -1);
    }
    func_002B3BA8(object, *(int *)(frame + 0x354), 0, 0, 0.0f, 0.0f);
}

/* Re-arms the frame's four spawn slots, bumps the object's counter and re-runs the stage. */
void func_L09_003F8898(char *object) {
    char *frame = *(char **)(object + 0x68);

    if (*(unsigned char *)(frame + 0x2EA) != 0) {
        *(unsigned char *)(frame + 0x2EA) = 0;
        if (*(short *)(frame + 0x2E0) < D_L09_001B0F50)
            *(short *)(frame + 0x2E0) = func_002A7820(D_L09_001B0F54, D_L09_001B0F58);
    }
    if (func_00282978(frame + 0x2E0) == 0)
        *(unsigned short *)(frame + 0x2E8) = D_L09_001B0F4C;
    func_00282978(frame + 0x2E2);
    func_00282978(frame + 0x2E6);
    func_00282978(frame + 0x2E8);
    if (*(unsigned short *)(object + 0x96) <= 0xFFFE)
        *(unsigned short *)(object + 0x96) = *(unsigned short *)(object + 0x96) + 1;
    func_002B0990(*(int *)(frame + 0x2F8), frame + 0x430, 4, 1, 1.9f);
}
