typedef unsigned char u8;
extern void *func_002B5D28(u8 *);

void *func_002B60B0(u8 *object, float x, float y, float z, float w)
{
    char *p = func_002B5D28(object);
    if (p != 0) {
        *(float *)(p + 0x20) = x;
        *(float *)(p + 0x1C) = w;
        *(float *)(p + 0x14) = z;
        *(float *)(p + 0x24) = x;
        *(float *)(p + 0x28) = y;
        *(float *)(p + 0x18) = z;
    }
    return p;
}
