int func_00295870(int arg) {
    extern int D_00105540[];
    if ((arg & 0x100) == 0)
        return D_00105540[(arg & 0xFF) * 2 + 0x15D4 / 4] > 0;
    return D_00105540[(arg & 0xFF) * 2 + 0x14F4 / 4] > 0;
}
