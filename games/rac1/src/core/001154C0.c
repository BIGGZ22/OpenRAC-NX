#include "common.h"
#include "structs.h"

/*
 * core_text object 0x1154C0-0x116070. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib (the SDK's libc.a): mlock.o (__malloc_lock/__malloc_unlock, both
 * empty) and mprec.o (_Balloc ... _d2b). The linker dead-stripped mprec's
 * unreferenced _s2b, _ulp and _ratio whole, and _b2d and _mprec_log10 down
 * to their last word (func_00115EE0, func_00116068).
 * Built with Sony's 2.9-ee (Makefile.sn, EE29_CORE), like libc.a.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];

void func_001154C0(void) {
}

void func_001154C8(void) {
}

/*
 * mprec.c _Balloc(ptr, k): the freelist at ptr+0x4C (calloc'd 16 pointers
 * on first use); pop freelist[k], or calloc 1 x (0x18 + (x - 1) * 4) with
 * x = 1 << k and set _k/_maxwds; then _sign = _wds = 0 (wds stored
 * first). Exact under both compilers.
 *
 * The one lever: retail carries the freelist pointer across the join in
 * one register (loaded on the entry path, copied after the NULL test on
 * the calloc path). newlib's own text re-reads ptr->_freelist at the join
 * (7/168); an `fl` local assigned in the test and re-read from
 * ptr->_freelist after the calloc NULL test gives retail's order.
 */
extern void *func_001123A8(void *, int, int);

typedef struct Bigint_1154D0 {
    struct Bigint_1154D0 *next;
    int k, maxwds, sign, wds;
    unsigned int x[1];
} Bigint_1154D0;

typedef struct {
    char pad[0x4C];
    Bigint_1154D0 **freelist;
} Reent_1154D0;

void *func_001154D0(void *arg0, int k) {
    Reent_1154D0 *ptr = arg0;
    Bigint_1154D0 **fl;
    int x;
    Bigint_1154D0 *rv;

    if ((fl = ptr->freelist) == 0) {
        ptr->freelist = func_001123A8(ptr, sizeof(Bigint_1154D0 *), 16);
        if (ptr->freelist == 0) {
            return 0;
        }
        fl = ptr->freelist;
    }
    if ((rv = fl[k]) != 0) {
        fl[k] = rv->next;
    } else {
        x = 1 << k;
        rv = func_001123A8(ptr, 1, sizeof(Bigint_1154D0) + (x - 1) * sizeof(rv->x));
        if (rv == 0) {
            return 0;
        }
        rv->k = k;
        rv->maxwds = x;
    }
    rv->sign = rv->wds = 0;
    return rv;
}

/*
 * Close but not exact, same register-allocation-choice category as
 * func_001160D8 above (see its comment) -- identical operations, order,
 * and count as retail, just a different scratch-register assignment
 * among $v0/$v1/$a0 for the three live temporaries (idx, table/bucket,
 * old head). See "Open toolchain questions" in docs/DECOMP_PROGRESS.md.
 *
 * Hash-bucket linked-list insertion: pushes arg1 onto the head of the
 * bucket at table[idx], where idx is read from arg1 itself and table is
 * a pointer stored at offset 0x4C of arg0.
 */
void func_00115578(void *arg0, void *arg1) {
    if (arg1 != 0) {
        int idx = *(int *)((char *)arg1 + 4);
        void **table = *(void ***)((char *)arg0 + 0x4C);
        *(void **)arg1 = table[idx];
        table[idx] = arg1;
    }
}

extern void *func_001154D0(void *ptr, int k);        /* Balloc */
extern void func_00115578(void *ptr, void *b);        /* Bfree */
extern void *func_00115248(void *dest, void *src, unsigned int n); /* memcpy */

/* newlib mprec.c multadd(ptr,b,m,a): b = b*m+a in place (16-bit limbs
 * packed two to a 32-bit word, Pack_32); if the carry out of the top
 * limb doesn't fit, Balloc a wider Bigint, Bcopy the sign/wds/digits
 * over (memcpy from &b->sign, which is what func_00115248 is here),
 * Bfree the old one, and append the carry as one more digit. */
void *func_001155A8(void *ptr, void *arg1, int m, int a) {
    unsigned int *x;
    unsigned int xi, y, z;
    int i, wds;
    Bigint_1154D0 *b1;

    wds = ((Bigint_1154D0 *)arg1)->wds;
    x = ((Bigint_1154D0 *)arg1)->x;
    i = 0;
    do {
        xi = *x;
        y = (xi & 0xFFFF) * m + a;
        z = (xi >> 16) * m + (y >> 16);
        a = (int)(z >> 16);
        *x++ = (z << 16) + (y & 0xFFFF);
    } while (++i < wds);
    if (a) {
        if (wds >= ((Bigint_1154D0 *)arg1)->maxwds) {
            b1 = func_001154D0(ptr, ((Bigint_1154D0 *)arg1)->k + 1);
            func_00115248(&b1->sign, &((Bigint_1154D0 *)arg1)->sign,
                          ((Bigint_1154D0 *)arg1)->wds * sizeof(unsigned int) + 2 * sizeof(int));
            func_00115578(ptr, arg1);
            arg1 = b1;
        }
        ((Bigint_1154D0 *)arg1)->x[wds++] = a;
        ((Bigint_1154D0 *)arg1)->wds = wds;
    }
    return arg1;
}

int func_001156C0(unsigned int arg0) {
    int count;
    if ((arg0 & 0xFFFF0000u) != 0) {
        count = 0;
    } else {
        count = 0x10;
        arg0 <<= 16;
    }
    if ((arg0 & 0xFF000000u) == 0) {
        count += 8;
        arg0 <<= 8;
    }
    if ((arg0 & 0xF0000000u) == 0) {
        count += 4;
        arg0 <<= 4;
    }
    if ((arg0 & 0xC0000000u) == 0) {
        count += 2;
        arg0 <<= 2;
    }
    if ((int)arg0 >= 0) {
        count += 1;
        if ((arg0 & 0x40000000u) == 0) {
            return 0x20;
        }
    }
    return count;
}

int func_00115748(unsigned int *arg0) {
    unsigned int v = *arg0;
    unsigned int count;

    if ((v & 7) != 0) {
        if ((v & 1) != 0) {
            return 0;
        }
        if ((v & 2) != 0) {
            v >>= 1;
            *arg0 = v;
            return 1;
        } else {
            v >>= 2;
            *arg0 = v;
            return 2;
        }
    }

    count = 0;
    if ((v & 0xFFFF) == 0) {
        count = 0x10;
        v >>= 16;
    }
    if ((v & 0xFF) == 0) {
        count += 8;
        v >>= 8;
    }
    if ((v & 0xF) == 0) {
        count += 4;
        v >>= 4;
    }
    if ((v & 3) == 0) {
        count += 2;
        v >>= 2;
    }
    if ((v & 1) == 0) {
        v >>= 1;
        count += 1;
        if (v == 0) {
            return 0x20;
        }
    }
    *arg0 = v;
    return count;
}

extern void *func_001154D0(void *ptr, int k);

/* newlib mprec.c i2b(): b = Balloc(ptr, 1); b->_x[0] = i; b->_wds = 1;
   return b. Returning the Bigint is what keeps the call result in $v0 and
   puts the working copy in $v1 (the void version stores through $v0 and
   comes out 4 bytes short). Bigint: _wds at +0x10, _x[0] at +0x14. */
/* newlib's i2b (mprec.c): a one-word Bigint holding i. It returns b;
   that return is where retail's $v1 copy comes from. */
void *func_00115808(void *ptr, int i) {
    char *b = func_001154D0(ptr, 1);
    *(int *)(b + 0x14) = i;
    *(int *)(b + 0x10) = 1;
    return b;
}

/* newlib mprec.c mult(ptr,a,b): _multiply, Bigint*Bigint for dtoa/strtod
 * (schoolbook long multiplication in 16-bit limbs, ensuring a is the
 * not-shorter operand first). */
extern void *func_001154D0(void *ptr, int k); /* Balloc */

#define STOREINC(xc, hi, lo) \
    (((unsigned short *)(xc))[1] = (unsigned short)(hi), \
     ((unsigned short *)(xc))[0] = (unsigned short)(lo), \
     (xc)++)

Bigint_1154D0 *func_00115840(void *ptr, Bigint_1154D0 *a, Bigint_1154D0 *b) {
    Bigint_1154D0 *c;
    int k, wa, wb, wc;
    unsigned int carry, y, z, z2;
    unsigned int *x, *xa, *xae, *xb, *xbe, *xc, *xc0;

    if (a->wds < b->wds) {
        c = a;
        a = b;
        b = c;
    }
    k = a->k;
    wa = a->wds;
    wb = b->wds;
    wc = wa + wb;
    if (wc > a->maxwds)
        k++;
    c = func_001154D0(ptr, k);
    for (x = c->x, xa = x + wc; x < xa; x++)
        *x = 0;
    xa = a->x;
    xae = xa + wa;
    xb = b->x;
    xbe = xb + wb;
    xc0 = c->x;
    for (; xb < xbe; xb++, xc0++) {
        if ((y = *xb & 0xffff)) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
                carry = z >> 16;
                z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
                carry = z2 >> 16;
                STOREINC(xc, z2, z);
            } while (x < xae);
            *xc = carry;
        }
        if ((y = *xb >> 16)) {
            x = xa;
            xc = xc0;
            carry = 0;
            z2 = *xc;
            do {
                z = (*x & 0xffff) * y + (*xc >> 16) + carry;
                carry = z >> 16;
                STOREINC(xc, z, z2);
                z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
                carry = z2 >> 16;
            } while (x < xae);
            *xc = z2;
        }
    }
    for (xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc)
        ;
    c->wds = wc;
    return c;
}

/* newlib mprec.c pow5mult(ptr,b,k): multiplies Bigint b by 5^k using a
 * squaring ladder cached in ptr->_p5s (the first four bits of k via the
 * {5,25,125} table D_001524B8, the rest by repeated squaring of 625). */
extern void *func_001155A8(void *ptr, void *b, int m, int a); /* multadd */
extern int D_001524B8[3];

typedef struct { char pad[0x48]; Bigint_1154D0 *p5s; } Reent_115A70;

void *func_00115A70(void *ptr, void *b, int k) {
    Reent_115A70 *r = ptr;
    Bigint_1154D0 *p5, *p51, *b1;
    int i;

    if ((i = k & 3))
        b = func_001155A8(ptr, b, D_001524B8[i - 1], 0);

    if (!(k >>= 2))
        return b;
    if (!(p5 = r->p5s)) {
        p5 = r->p5s = func_00115808(ptr, 625);
        p5->next = 0;
    }
    for (;;) {
        if (k & 1) {
            b1 = func_00115840(ptr, b, p5);
            func_00115578(ptr, b);
            b = b1;
        }
        if (!(k >>= 1))
            break;
        if (!(p51 = p5->next)) {
            p51 = p5->next = func_00115840(ptr, p5, p5);
            p51->next = 0;
        }
        p5 = p51;
    }
    return b;
}

/* newlib mprec.c lshift(ptr,b,k): shift Bigint b left by k bits, growing
 * into a freshly-Balloc'd result and freeing the input. */
extern void *func_001154D0(void *ptr, int k);       /* Balloc */
extern void func_00115578(void *ptr, void *b);      /* Bfree */

Bigint_1154D0 *func_00115B70(void *ptr, Bigint_1154D0 *b, int k) {
    int i, k1, n, n1;
    Bigint_1154D0 *b1;
    unsigned int *x, *x1, *xe, z;

    n = k >> 5;
    k1 = b->k;
    n1 = n + b->wds + 1;
    for (i = b->maxwds; n1 > i; i <<= 1)
        k1++;
    b1 = func_001154D0(ptr, k1);
    x1 = b1->x;
    for (i = 0; i < n; i++)
        *x1++ = 0;
    x = b->x;
    xe = x + b->wds;
    if ((k &= 0x1f)) {
        k1 = 32 - k;
        z = 0;
        do {
            *x1++ = *x << k | z;
            z = *x++ >> k1;
        } while (x < xe);
        if ((*x1 = z))
            ++n1;
    } else {
        do {
            *x1++ = *x++;
        } while (x < xe);
    }
    b1->wds = n1 - 1;
    func_00115578(ptr, b);
    return b1;
}

/* newlib mprec.c cmp(a,b) (real name __mcmp): three-way compare of two
 * Bigints by word count, then from the most-significant word down. */
int func_00115CE8(Bigint_1154D0 *a, Bigint_1154D0 *b) {
    unsigned int *xa, *xa0, *xb, *xb0;
    int i, j;

    i = a->wds;
    j = b->wds;
    if ((i -= j))
        return i;
    xa0 = a->x;
    xa = xa0 + j;
    xb0 = b->x;
    xb = xb0 + j;
    for (;;) {
        if (*--xa != *--xb)
            return *xa < *xb ? -1 : 1;
        if (xa <= xa0)
            break;
    }
    return 0;
}

/* newlib mprec.c diff(ptr,a,b): __mdiff, the |a-b| used by dtoa/strtod's
 * Bigint arithmetic. cmp(a,b)==0 returns a fresh zero Bigint; otherwise
 * swaps so a is the larger operand (sign records which), subtracts
 * limb-by-limb with borrow, and trims leading zero limbs.
 *
 * Retail reuses the SAME register ($t3/$11) both as the swap's scratch
 * temporary and, right after, as `c` (Balloc's return value) -- their
 * live ranges never overlap. Reusing `c` itself as the swap temp here
 * (instead of a separate `t`) to see whether that steers the allocator
 * onto the same register for both roles the way retail's does. */
extern int func_00115CE8(Bigint_1154D0 *a, Bigint_1154D0 *b);  /* cmp */
extern void *func_001154D0(void *ptr, int k);        /* Balloc */

#define STOREINC(xc, hi, lo) \
    (((unsigned short *)(xc))[1] = (unsigned short)(hi), \
     ((unsigned short *)(xc))[0] = (unsigned short)(lo), \
     (xc)++)

void *func_00115D50(void *ptr, Bigint_1154D0 *a, Bigint_1154D0 *b) {
    Bigint_1154D0 *c;
    unsigned int *xa, *xae, *xb, *xbe, *xc;
    int i, wa, wb;
    int borrow, y, z;

    i = func_00115CE8(a, b);
    if (!i) {
        c = func_001154D0(ptr, 0);
        c->wds = 1;
        c->x[0] = 0;
        return c;
    }
    if (i < 0) {
        c = a;
        a = b;
        b = c;
        i = 1;
    } else {
        i = 0;
    }
    c = func_001154D0(ptr, a->k);
    c->sign = i;
    wa = a->wds;
    xa = a->x;
    xae = xa + wa;
    wb = b->wds;
    xb = b->x;
    xbe = xb + wb;
    xc = c->x;
    borrow = 0;
    do {
        y = (*xa & 0xffff) - (*xb & 0xffff) + borrow;
        borrow = y >> 16;
        z = (*xa++ >> 16) - (*xb++ >> 16) + borrow;
        borrow = z >> 16;
        STOREINC(xc, z, y);
    } while (xb < xbe);
    while (xa < xae) {
        y = (*xa & 0xffff) + borrow;
        borrow = y >> 16;
        z = (*xa++ >> 16) + borrow;
        borrow = z >> 16;
        STOREINC(xc, z, y);
    }
    while (!*--xc)
        wa--;
    c->wds = wa;
    return c;
}

LINKER_REMNANT("asm/remnants/core_text", func_00115EE0);

/* newlib mprec.c d2b(ptr,_d,e,bits): splits a double into a Bigint
 * significand plus a binary exponent and precision (dtoa's first step).
 * Pack_32, !_DOUBLE_IS_32BITS, !Sudden_Underflow, IEEE_8087 word order
 * (the high 32 bits carry sign/exponent/high mantissa, the low 32 bits
 * the rest of the mantissa). */
typedef union {
    double d;
    unsigned int w[2];
} DUnion_115EE8;

extern void *func_001154D0(void *ptr, int k);  /* Balloc */
extern int func_00115748(unsigned int *y);     /* lo0bits */
extern int func_001156C0(unsigned int x);      /* hi0bits */

Bigint_1154D0 *func_00115EE8(void *ptr, double _d, int *e, int *bits) {
    DUnion_115EE8 d;
    Bigint_1154D0 *b;
    int de, i, k;
    unsigned int *x, y, z;

    d.d = _d;
    b = func_001154D0(ptr, 1);
    x = b->x;

    z = d.w[1] & 0xFFFFF;
    d.w[1] &= 0x7FFFFFFF;
    if ((de = (int)(d.w[1] >> 20)))
        z |= 0x100000;

    if ((y = d.w[0])) {
        if ((k = func_00115748(&y))) {
            x[0] = y | z << (32 - k);
            z >>= k;
        } else {
            x[0] = y;
        }
        i = b->wds = (x[1] = z) ? 2 : 1;
    } else {
        k = func_00115748(&z);
        x[0] = z;
        i = b->wds = 1;
        k += 32;
    }
    if (de) {
        *e = de - 1023 - (53 - 1) + k;
        *bits = 53 - k;
    } else {
        *e = de - 1023 - (53 - 1) + 1 + k;
        *bits = 32 * i - func_001156C0(x[i - 1]);
    }
    return b;
}

LINKER_REMNANT("asm/remnants/core_text", func_00116068);
