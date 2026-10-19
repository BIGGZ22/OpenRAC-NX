/* Free a block; when it is the most recent allocation, roll the bump pointer
   back and credit the released amount to the running total.
   Fields of g_GlobalContext (0x01F7E580): +0x08 alloc pointer, +0x0C recent
   block, +0x10 byte total. */
struct GlobalContextMem {
    char pad00[8];
    unsigned int alloc_current; /* 0x08 */
    unsigned int alloc_recent;  /* 0x0C */
    unsigned int alloc_total;   /* 0x10 */
};
extern struct GlobalContextMem g_GlobalContext;

void MEM_free__FPvPcUi(void *block, char *name, unsigned int line) {
    (void)name;
    (void)line;
    if (block == (void *)g_GlobalContext.alloc_recent) {
        unsigned int released = g_GlobalContext.alloc_current - (unsigned int)block;
        g_GlobalContext.alloc_current = (unsigned int)block;
        g_GlobalContext.alloc_total = g_GlobalContext.alloc_total + released;
    }
}
