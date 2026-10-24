#include "common.h"
#include "structs.h"

typedef struct {
    unsigned char type;
    unsigned char id;
} TopThreadEntry;

typedef struct {
    int counter;
    unsigned int pad;
    TopThreadEntry entries[0x200];
} TopThreadContext;

extern int r1_D_00154600;

extern char r1_D_001527E8[];

extern int r1_func_00118CB0(int);

extern int r1_func_00118C10(int);

extern int r1_func_00118BC0(int);

extern int r1_func_00118C40(int);

extern int r1_func_0011A690(const char *, ...);

extern void func_0011B728(void *);

ASM_FUNC("asm/core", func_0011B588);

ASM_FUNC("asm/core", func_0011B5F0);

ASM_FUNC("asm/core", func_0011B658);

ASM_FUNC("asm/core", func_0011B6C0);

/* rac1-decomp src/core/00119328.c: func_001194C8 */
/* topThread: kernel service dispatcher. Adapted from Lombyte (GPL-3.0) for PAL. */
void func_0011B728(void *arg0) {
    TopThreadContext *ctx = (TopThreadContext *)arg0;
    int i;
    unsigned char type;

    for (;;) {
        r1_func_00118CB0(r1_D_00154600);
        i = ctx->counter & 0x1FF;
        ctx->counter = i + 1;
        type = ctx->entries[i].type;
        switch (type) {
        case 0:
            r1_func_00118C10(ctx->entries[i].id);
            break;
        case 1:
            r1_func_00118BC0(ctx->entries[i].id);
            break;
        case 2:
            r1_func_00118C40(ctx->entries[i].id);
            break;
        default:
            r1_func_0011A690(r1_D_001527E8);
            break;
        }
    }
}
