/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_0011AD70.c (games/rac1): func_0011AD70, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: high. Data-cache writeback of [addr, addr + size) in 64-byte lines (cache op 0x18, nothing when size <= 0); the host has no separate cache, memory is already coherent, and retail returns nothing, so it is empty. */

void sceSifWriteBackDCache(void *addr, int size) {
    (void)addr;
    (void)size;
}
