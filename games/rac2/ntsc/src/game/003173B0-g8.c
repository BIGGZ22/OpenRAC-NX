#include "include_asm.h"
extern int D_L00_001B0138 __attribute__((sda));
extern int D_L00_002472C0[][4];
extern void func_0031E528(void *);
extern void func_002FEE50(int);
extern void func_0029F338(void *);

void func_0031E6E8(char *object)
{
    char *data = *(char **)(object + 0x68);
    int index = *(int *)(data + 0x14);
    --D_L00_001B0138;
    D_L00_002472C0[index][0] = 0;
    func_0031E528(object);
    func_002FEE50(*(int *)(data + 0x44));
    func_0029F338(object);
}

extern int ref_L00_0031D308_00 __attribute__((sda));
extern char ref_L00_0031D308_01 __attribute__((sda));
extern int ref_L00_0031D308_02;
extern void func_00281A08(void *, int, void *, void *, int, int, int, int, float);

void func_0031D308(char *object)
{
    int count = ref_L00_0031D308_00;
    int state = ref_L00_0031D308_02;
    char *data = *(char **)(object + 0x68);
    func_00281A08(&ref_L00_0031D308_01, 4, data + 0x3A0, data + 0x4A0,
        *(int *)(data + 0x284), *(int *)(data + 0x280),
        count, state, *(float *)(data + 0x288));
}

extern unsigned short *D_001EE5A0[];
extern char *D_001B1B5C __attribute__((sda));

void func_003182C0(char *self)
{
    if (*(unsigned char *)(self + 0x21) != 0xFF) {
        unsigned short *list = D_001EE5A0[*(unsigned char *)(self + 0x21)];
        if (list != 0) {
            char *base = D_001B1B5C;
            unsigned short *entry = list;
            int key;
            do {
                char *object = (char *)((unsigned int)base + ((*entry & 0x7FFF) << 8));
                if (object[0x20] >= 0 && *(short *)(object + 0xAA) == 0x99C)
                    *(short *)(*(char **)(object + 0x68) + 0x276) = 1;
                key = *(short *)entry++;
            } while (key >= 0);
        }
    }
}

void func_00319F38(char *self)
{
    if (*(unsigned char *)(self + 0x21) != 0xFF) {
        unsigned short *list = D_001EE5A0[*(unsigned char *)(self + 0x21)];
        if (list != 0) {
            char *base = D_001B1B5C;
            unsigned short *entry = list;
            int key;
            do {
                char *object = (char *)((unsigned int)base + ((*entry & 0x7FFF) << 8));
                if (object[0x20] >= 0 && *(short *)(object + 0xAA) == 0x9B4)
                    *(short *)(*(char **)(object + 0x68) + 0x286) = 1;
                key = *(short *)entry++;
            } while (key >= 0);
        }
    }
}

void func_0031C720(char *self)
{
    if (*(unsigned char *)(self + 0x21) != 0xFF) {
        unsigned short *list = D_001EE5A0[*(unsigned char *)(self + 0x21)];
        if (list != 0) {
            char *base = D_001B1B5C;
            unsigned short *entry = list;
            int key;
            do {
                char *object = (char *)((unsigned int)base + ((*entry & 0x7FFF) << 8));
                if (object[0x20] >= 0 && *(short *)(object + 0xAA) == 0x9B5)
                    *(short *)(*(char **)(object + 0x68) + 0x246) = 1;
                key = *(short *)entry++;
            } while (key >= 0);
        }
    }
}

int func_0031A580(char *self)
{
    char *data = *(char **)(self + 0x68);
    unsigned short *list;
    unsigned short *entry;
    char *object;

    if (*(unsigned char *)(self + 0x21) == 0xFF)
        return 0;
    list = D_001EE5A0[*(unsigned char *)(self + 0x21)];
    if (list == 0)
        return 0;
    {
        int busy = 0;
        entry = list;
        do {
            object = (char *)((unsigned int)D_001B1B5C + ((*entry & 0x7FFF) << 8));
            if (object[0x20] >= 0 && *(short *)(object + 0xAA) == 0x9B4
                && *(int *)(*(char **)(object + 0x68) + 0x350) != 0) {
                busy = 1;
                break;
            }
        } while ((short)*entry++ >= 0);
        if (!busy)
            *(int *)(data + 0x350) = 1;
    }
    return *(int *)(data + 0x350);
}

int func_0031A628(char *self)
{
    char *data = *(char **)(self + 0x68);
    char *found = 0;
    int prev = -1;
    unsigned short *list;
    unsigned short *entry;
    char *object;
    char *other;
    int value;

    if (*(unsigned char *)(self + 0x21) == 0xFF)
        return 0;
    list = D_001EE5A0[*(unsigned char *)(self + 0x21)];
    if (list == 0)
        return 0;
    if (*(int *)(data + 0x350) == 0)
        return 0;
    entry = list;
    do {
        object = (char *)((unsigned int)D_001B1B5C + ((*entry & 0x7FFF) << 8));
        if ((unsigned char)object[0x95] != 4 && object[0x20] >= 0
            && *(short *)(object + 0xAA) == 0x9B4) {
            value = *(unsigned short *)(object + 0x96);
            other = *(char **)(object + 0x68);
            if (prev < value) {
                if (object != self) {
                    found = other;
                    prev = value;
                }
            }
        }
    } while ((short)*entry++ >= 0);
    if (found != 0) {
        *(int *)(found + 0x350) = 1;
        *(int *)(data + 0x350) = 0;
        return 1;
    }
    return 0;
}

extern int D_001B2758;
extern void func_0031EFC8(char *, char *);
extern char *func_002A7820(int, int);
extern float func_002A7940(void);
extern void func_0031EB00(char *, unsigned int, float, char *, float, int, int);

void func_0031E2A0(char *p)
{
    int offset = 0;
    int count = 0xB;
    char *entry;
    char *ptr;
    float f;

    do {
        count--;
        entry = (char *)(offset + (unsigned int)D_001B2758);
        if (*(int *)(entry + 0x8C) == (int)p) {
            func_0031EFC8(p, entry);
            if (*(int *)(entry + 0x94) == 0) {
                ptr = func_002A7820(0x50, 0x60);
                f = func_002A7940();
                func_0031EB00(entry + 0x30, 0x103080, 0.5f, ptr, f, 0x31, 2);
                ptr = func_002A7820(0x50, 0x60);
                f = func_002A7940();
                func_0031EB00(entry + 0x30, 0xFFFFFF, 0.3f, ptr, f, 0x31, 2);
            }
        }
        offset += 0xA0;
    } while (count >= 0);
}
