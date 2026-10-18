typedef struct { char b[0x28]; } tPAR_BLOCK;

extern tPAR_BLOCK par_blocks[];
/* The mangling says __FPiii and the retail callee uses the third argument
   as an integer count (slt against a counter, addu then sh); all four
   callers pass a counter. */
extern void func_1E7AC78(void *a, void *b, void *c, int d);

void PAR_Emit__FPiii(void *a, int i, int d) {
    tPAR_BLOCK *b = &par_blocks[i];
    func_1E7AC78(a, (char *)b + 0x10, b, d);
}
