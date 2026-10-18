/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_001191E0.c (games/rac1): func_001191E0, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: high. Cache maintenance only (index-invalidates the 128 sets of both ways of the instruction cache, skipping lines whose tag lies in [lo, hi]); the host has no such cache and the function has no memory, register-visible or return effect, so it is empty. */

void _sceSDC(unsigned int lo, unsigned int hi) {
    (void)lo;
    (void)hi;
}
