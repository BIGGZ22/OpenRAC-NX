#include "include_asm.h"

/* The threshold lives in small data, so this file is built with -G8. It differs
   from program to program, so it carries no level. */
extern float ref_L00_00327D08_00 __attribute__((sda));
extern void func_002FC808(void);
extern void func_00327D50(char *);

/* Runs the shared update, then advances the object's sub-record while its
   stored float is still below the small-data threshold. */
void func_00327D08(char *a0)
{
    float value;

    func_002FC808();
    value = *(float *)(*(char **)(a0 + 0x68) + 0x24);
    if (value < ref_L00_00327D08_00)
        func_00327D50(a0);
}

extern int D_001B1688;
extern char *D_001B1B68;
extern float ref_L00_00327A60_00 __attribute__((sda));
extern float func_00282E48(void *, void *);
extern void func_00302440(void *, void *);

void func_00327A60(char *object)
{
    char *cursor;
    if (D_001B1688 & 1) {
        cursor = D_001B1B68;
        while (cursor) {
            int kind = *(unsigned char *)(cursor + 0x20);
            if (kind == 0xfe) return;
            if (kind == 0xfd) return;
            if (cursor && *(char **)(cursor + 0x24) &&
                *(short *)(*(char **)(cursor + 0x24) + 0x46) == 0x13) {
                if (func_00282E48(cursor + 0x10, object + 0x10) < ref_L00_00327A60_00)
                    func_00302440(cursor, object);
            }
            cursor = *(char **)(cursor + 0x28);
        }
    }
}
