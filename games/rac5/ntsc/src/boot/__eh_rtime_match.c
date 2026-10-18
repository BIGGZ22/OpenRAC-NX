/* PARKED (w2-agent-04): retail saves callee-saved GPRs with a 16-byte slot stride
   (s0@0x0/s1@0x10/ra@0x20 ...) while the pinned chain emits 8-byte slots
   (mcc mips.c:4723, RAC0/RAC2 TImode anchor neutralised). Only the frame
   instructions differ; not reachable from C. Reopen with that compiler anchor. */
/* __eh_rtime_match 0x01E974E8..0x01E97530 */
typedef int (*EH_MATCH)(void *info, void *rtime, void *arg);
extern void **__get_eh_info(void);
int __eh_rtime_match(void *rtime) {
    void *info = *__get_eh_info();
    EH_MATCH fn = *(EH_MATCH *)info;
    if (fn == 0)
        return 0;
    return fn(info, rtime, 0) != 0;
}
