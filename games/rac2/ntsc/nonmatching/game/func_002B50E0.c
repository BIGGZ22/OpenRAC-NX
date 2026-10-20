/* func_002B50E0 (boot function, 372 bytes; 20 levels, 21 places in all).
 * Near miss: compiles to 368 bytes against retail's 372, and tools/check.py
 * reports 0 of 21 places match. The prologue and the register assignment
 * (self in $s2, target in $s3, data in $s1) match, and so do the stack slots
 * (pos at sp+0, local1 at sp+0x10, local2 at sp+0x20). What differs: (1) the
 * vector copy from self + 0x10 is folded into lq ...,0x10($s2), where retail
 * computes addiu $v1,$s2,0x10 first (four bytes less; everything after it is
 * shifted by four and agrees except the items below); (2) the lq takes $v1
 * where retail takes $v0, and the sq follows the table load; (3) the
 * argument registers of the calls to the two functions after the vector are
 * set in another order. Tried without success: the pointer held in a local,
 * the three vectors as separate locals (a different stack layout and an extra
 * saved register), and the three vectors as one array (this version). The
 * sda declaration of D_001B1688 gives the same mix of $gp and absolute loads
 * as the retail.
 */
extern int D_001B1688 __attribute__((sda));
extern void func_002B5CD8(void *, void *);
extern void func_002B5BC0(void *, void *, float);
extern int func_00282C88(void *);
extern void func_002B52A0(void *, void *, void *, void *, void *, float);
extern void func_002B5678(void *, void *, void *, void *);
extern void func_002B5828(void *, void *, void *, void *);
extern void func_002B5EF8(void *, void *, float);
extern void func_002B62A0(void);

typedef int PoseVector128 __attribute__((mode(TI)));

int func_002B50E0(void *self, void *target, float amount)
{
    char *data = func_002B5D28(self);
    PoseVector128 work[3];

    if (data == 0)
        return 8;
    if (D_001B1688 != *(int *)(data + 0xBC)) {
        *(int *)(data + 0x98) = *(int *)(data + 0x94);
        *(int *)(data + 0x94) = 0;
        if (!(*(int *)(data + 0x38) & 0x100))
            func_002B5CD8(self, data);
        {
            PoseVector128 *src = (PoseVector128 *)((char *)self + 0x10);
            work[0] = *src;
        }
        if (D_001B1688 - *(int *)(data + 0xBC) >= 2 || *(int *)(data + 0xBC) == 0) {
            func_002B5BC0(self, data, 0.0f);
            *(unsigned char *)(data + 0xEA) = 0xFF;
            *(int *)(data + 0x9C) &= ~2;
            *(float *)(data + 0xB4) = 0.0f;
            *(int *)(data + 0x80) = 0;
            func_00282C88(data + 0xA0);
        }
        *(int *)(data + 0xBC) = D_001B1688;
        func_002B52A0(self, data, target, &work[1], &work[2], amount);
        func_002B5678(self, data, &work[0], &work[2]);
        func_002B5828(self, data, target, &work[0]);
        func_002B5EF8(self, data, amount);
        func_002B62A0();
    }
    return *(int *)(data + 0x94);
}
