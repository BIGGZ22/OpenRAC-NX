/* sceKernelGetSystemTimeLow, retail 0x01E86280 (160 bytes): microseconds since
 * reset, from the microsecond bus-clock counter at 147456000 Hz.
 *
 * measured (2026-10-16):
 *  - the natural C -- `(unsigned long long)((double)ticks / 147456000.0 *
 *    1000000.0)` -- is byte-exact 160/160 with this same cc1 when the object is
 *    assembled by the SN Ps2EeAs, which expands the `li.d`/`dli` pseudo-ops
 *    inline as ori/dsll exactly as the retail bytes do.
 *  - the pinned GNU as instead loads `li.d` from a .rdata pool (2 instructions
 *    where retail has 4), which is what parked this target.
 *  - GNU as does expand a DImode `dli` inline, so the two constants are written
 *    here as the 64-bit patterns of those doubles: the EE ABI carries a double
 *    in a single 64-bit GPR, so the transmitted value is identical.  With this
 *    form the body is 155/156 under GNU as (one word: GNU as renders
 *    `dli 0xFFFFFFFF` as `lui $v1,0xFFFF`, retail as `addiu $v1,$zero,-1`).
 *
 * Reopen when a per-file assembler/compiler selector exists (IDIOMES.md 15):
 * with the SN assembler the version with plain `147456000.0` / `1000000.0`
 * doubles is exact.
 */
extern unsigned long long GetTimerSystemTime(void);
extern double dpadd(double, double);
extern double dpdiv(double, unsigned long long);
extern double dpmul(double, unsigned long long);
extern unsigned long long __fixunsdfdi(double);

int sceKernelGetSystemTimeLow__Fv(void) {
    unsigned long long ticks = GetTimerSystemTime();
    unsigned long long usec;

    usec = __fixunsdfdi(dpmul(dpdiv((double)ticks, 0x41A1940000000000ULL),
                              0x412E848000000000ULL));
    return (int)(usec & 0xFFFFFFFF);
}
