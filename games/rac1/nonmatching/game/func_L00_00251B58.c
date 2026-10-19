/* fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
extern void func_0020EA70(unsigned char *m, unsigned int rect);

/* UpdateMobyGrids, the level programs' copy: the executable's func_0020EA70 instruction for
   instruction with the level's addresses, which the port maps for it (per-level relocation). */
void func_L00_00251B58(unsigned char *m, unsigned int rect) {
    func_0020EA70(m, rect);
}
