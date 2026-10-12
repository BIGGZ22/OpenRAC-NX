#include "common.h"
#include "structs.h"

extern int r1_func_0011B4C8();

extern char r1_D_0015B108[];

extern int r1_D_0015B180;

 /* exit */
extern void func_00128898(void *, ...);

extern int func_00128440(int);

extern char r1_D_00153658[];

extern char r1_D_00153678[];

extern int func_001284B0(void *, int, int);

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern int r1_D_0015B600[];

extern char r1_D_0015B580[];

extern void r1_func_00119288(void *, void *);

/* EIntr returns the previous interrupt state; the file declares it void.
   A call that returns a value changes how the lookup after it is
   allocated. */
extern int r1_func_0011D9A8_i(void) __asm__("r1_func_0011D9A8");

extern void *memcpy(void *, const void *, unsigned int);

extern char r1_D_001536B8[];

extern int func_00128578(int);

extern void r1_func_00119288(void *a, void *b);

extern char r1_D_001537A0[];

/* rac1-decomp src/core/001236F0.c: func_001247E8 */
/* Same RPC shape as func_00128250 above (9 args: $4-$11 plus one stack
   slot), but with a different command id and an error path.

   The request word goes into the buffer at +4. It has to be written
   through a local pointer, not as (&r1_D_0015B180)[1]: the latter folds to
   the symbol r1_D_0015B180+4, so the compiler builds THAT address and then
   subtracts 4 to pass the buffer. With `buf`, one lui/addiu serves both
   the store and the two pointer arguments, as retail does.

   The test is written >= 0 so the error path is the fall-through and the
   success path is the branch target, letting the result load sit in the
   bgez delay slot. */
int func_00128440(int arg0) {
    int *buf = &r1_D_0015B180;

    buf[1] = arg0;
    if (r1_func_0011B4C8(r1_D_0015B108, 0x80000904, 0, buf, 0x400,
                      buf, 0x400, 0, 0) >= 0) {
        return r1_D_0015B180;
    } else {
        func_00128898(r1_D_00153658);
        return 0;
    }
}

/* rac1-decomp src/core/001236F0.c: func_00124858 */
/* sceDbcCreateSocket (libdbc): copy the caller's socket record (five
   header words, then a 16-byte name) into the RPC buffer r1_D_0015B180, set
   its port (arg1, word 10) and mode (arg2, word 11), then run the
   create-socket RPC (0x80000901) through cd_base. Prints and returns 0
   on failure, else the reply's word 9.

   The header words are copied as pointers. Their alias set then differs
   from the int port/mode stores, so the header loads need not wait for
   those stores; the scheduler then places `i = 0` ahead of them, and the
   loop registers come out in $t0-$t2 as in retail. namesrc/namedst are
   precomputed so each byte address is one base+index add, and the call
   re-takes &r1_D_0015B180 for its buffer arguments. */
int func_001284B0(void *arg0, int arg1, int arg2) {
    int *buf = &r1_D_0015B180;
    char *namesrc = (char *)arg0 + 0x14;
    char *namedst = (char *)buf + 0x14;
    int i;

    buf[10] = arg1;
    buf[11] = arg2;
    ((void **)buf)[0] = ((void **)arg0)[0];
    ((void **)buf)[1] = ((void **)arg0)[1];
    ((void **)buf)[2] = ((void **)arg0)[2];
    ((void **)buf)[3] = ((void **)arg0)[3];
    ((void **)buf)[4] = ((void **)arg0)[4];
    for (i = 0; i < 0x10; i++) {
        namedst[i] = namesrc[i];
    }
    if (r1_func_0011B4C8(r1_D_0015B108, 0x80000901, 0, &r1_D_0015B180, 0x400,
                       &r1_D_0015B180, 0x400, 0, 0) < 0) {
        func_00128898(r1_D_00153678);
        return 0;
    }
    return buf[9];
}

/* rac1-decomp src/core/001236F0.c: func_00124920 */
/*
 * sceDbcGetDepNumber(port) (libdbc): write back the 0x80-byte DMA buffer
 * r1_D_0015B580 (r1_func_00119288), then with interrupts off (DIntr/EIntr)
 * copy its first 0x40 bytes into the link-state table r1_D_0015B600. A port
 * whose state is not 1 (linked) returns -12. Otherwise RPC 0x80000903
 * with the port as the request word; returns the reply's second word, or
 * reports the failure (func_00128898) and returns 0.
 *
 * The RPC buffer is re-taken as &r1_D_0015B180 for the call's arguments and
 * `buf` is only assigned where it is used, so the table base and the
 * buffer share retail's saved register.
 */
int func_00128578(int port) {
    int *buf;

    r1_func_00119288(r1_D_0015B580, r1_D_0015B580 + 0x80);
    r1_func_0011D960();
    memcpy(r1_D_0015B600, r1_D_0015B580, 0x40);
    r1_func_0011D9A8_i();
    if (r1_D_0015B600[port] != 1) {
        return -0xC;
    }
    buf = &r1_D_0015B180;
    buf[0] = port;
    if (r1_func_0011B4C8(r1_D_0015B108, 0x80000903, 0, &r1_D_0015B180, 0x400,
                       &r1_D_0015B180, 0x400, 0, 0) < 0) {
        func_00128898(r1_D_001536B8);
        return 0;
    }
    return buf[1];
}

INCLUDE_ASM("asm/core", func_001286C0);

INCLUDE_ASM("asm/core", func_001286C8);

/* rac1-decomp src/core/001236F0.c: func_00124A70 */
/* RPC 0x8000091A round trip on r1_D_0015B180 (the buffer is both request
   and reply), copying the reply's bytes to arg3. `r` is assigned in both
   arms of the copy guard with one return; two returns would be
   cross-jumped into one. */
int func_001287A8(int arg0, int arg1, int *arg2, char *arg3) {
    int *buf = &r1_D_0015B180;
    unsigned char *src;
    int i;
    int r;

    buf[0] = arg0;
    buf[1] = arg1;
    buf[2] = *arg2;
    if (r1_func_0011B4C8(r1_D_0015B108, 0x8000091A, 0, buf, 0x400, buf, 0x400, 0, 0) < 0) {
        func_00128898(r1_D_001537A0);
        return 0;
    }
    r = buf[0x23];
    if (r >= 0) {
        *arg2 = buf[2];
        i = 0;
        if (i < buf[2]) {
            src = (unsigned char *)buf + 0xC;
            do {
                arg3[i] = src[i];
                i++;
            } while (i < buf[2]);
            r = buf[0x23];
        } else {
            r = buf[0x23];
        }
    }
    return r;
}
