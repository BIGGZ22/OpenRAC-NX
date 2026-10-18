extern unsigned int PS2HEAP_MaxFreeMemSize__Fv(void);

unsigned int sceKernelMaxFreeMemSize__Fv(void) {
    return PS2HEAP_MaxFreeMemSize__Fv();
}
