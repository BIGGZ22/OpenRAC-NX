/* functional: medium. Interrupt-safe wake-up: if the current thread (func_00118BE0, GetThreadId; retail uses its interrupt twin syscall -0x2F, same result on the host) is tid, queue "type 0, id tid" in the ring at D_00154608 (counter at +4 masked with 0x1FF, entries of two bytes from +8) for the service thread and signal its semaphore D_00154600 via func_00118CA0, returning tid; with tid >= 0x100 or no service thread (D_0012FCF8 == 0) return -1; if the current thread is not tid, return func_00118C20(tid) (WakeupThread shape, called with a0 = tid as retail leaves it). The meaning of the syscalls is inferred from the topThread dispatcher. */
extern int D_0012FCF8;
extern int D_00154600;
extern int D_00154608[];
extern int func_00118BE0(void);
extern int func_00118C20(int tid);
extern int func_00118CA0(int sema);

int func_00119678(int tid) {
    int cur = func_00118BE0();
    unsigned char *base;
    int idx;
    if (cur != tid) {
        return func_00118C20(tid);
    }
    if ((unsigned int)cur >= 0x100 || D_0012FCF8 == 0) {
        return -1;
    }
    base = (unsigned char *)D_00154608;
    idx = D_00154608[1] & 0x1FF;
    D_00154608[1] = idx + 1;
    base[8 + idx * 2] = 0;
    base[9 + idx * 2] = (unsigned char)cur;
    func_00118CA0(D_00154600);
    return cur;
}
