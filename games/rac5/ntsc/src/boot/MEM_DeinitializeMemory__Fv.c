/* The memory manager state lives in the shared global context (g_GlobalContext,
   0x01F7E580) and the heap pointer is its first field. */
extern void *g_GlobalContext;
extern void PS2HEAP_Free__FPv(void *block);

void MEM_DeinitializeMemory__Fv(void) {
    PS2HEAP_Free__FPv(g_GlobalContext);
    g_GlobalContext = 0;
}
