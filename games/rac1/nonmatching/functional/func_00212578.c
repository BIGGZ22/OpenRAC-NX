/* functional: medium. fcheck: unchecked. Moby skin/light job runner: walks the chain of job records from job: while the current record's first word is nonzero, runs func_0020EF30(job, cache) and then func_001EE9F8(job, cache), which returns the next record; afterwards waits for DMA channel 0x1000D000 (fromSPR) to go idle (CHCR STR, bit 8). Retail saves s0-s7, gp, sp, fp and ra to D_001B7A00 around the walk because the two callees are hand-written code that use every register; it parks job and cache in $f0/$f1 for the same reason. The port's C callees keep the ABI, so none of that is needed. Caller: func_0020DFF8 passes D_0015F718 and the word at gp - 0x75E4. Written from the asm; the walk was not run in the emulator (its callees drive VU0 microcode). */
extern void func_0020EF30(char *job, char *scr);
extern unsigned int func_001EE9F8(unsigned char *job, unsigned char *cache);

void func_00212578(int *job, unsigned char *cache) {
    if (*job != 0) {
        do {
            func_0020EF30((char *)job, (char *)cache);
            job = (int *)func_001EE9F8((unsigned char *)job, cache);
        } while (*job != 0);
    }
    while (*(volatile unsigned int *)0x1000D000 & 0x100) {
    }
}
