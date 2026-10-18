extern unsigned int PS2HEAP_TotalFreeMemSize__Fv(void);

unsigned int sceKernelTotalFreeMemSize__Fv(void) {
    return PS2HEAP_TotalFreeMemSize__Fv();
}
