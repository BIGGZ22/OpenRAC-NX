void func_L06_00459B18(char *base)
{
    int i;

    if (base == 0) {
        return;
    }
    for (i = 0; i < *(int *)base; i++) {
        int off = i * 16;
        float v;

        v = func_00282E48(base + off + 0x10, base + ((i + 1) % *(int *)base) * 16 + 0x10);
        *(float *)(base + off + 0x1C) = v;
    }
