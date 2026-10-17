/* functional: low. Corrected from nonmatching/game/func_0020E9F0.c: retail builds the mask with a 32-bit sllv, so a count of 32 gives mask 0 (not 0xFFFFFFFF): it returns the first non-full word's base without marking anything. The near miss used a 64-bit shift. Identical for counts 1..16. */
extern unsigned char D_001C7A60[];

/* Allocates `count` consecutive 32-byte blocks of the moby grid's list storage (count a power of
   two, at most 32): the first 32-bit word of the bitmap D_001C7A60 that is not full, the first
   position in it, in steps of `count`, whose `count` bits are all clear. Marks them used and
   returns the first block's number (word * 32 + position). */
int func_0020E9F0(int count) {
    unsigned int *word = (unsigned int *)D_001C7A60;
    unsigned long mask = (unsigned long)((1u << (count & 31)) - 1u);
    int base = 0;

    for (;; word++, base += 32) {
        unsigned long bits = *word;
        int pos;

        if (bits == 0xFFFFFFFFu) {
            continue;
        }
        for (pos = 0; (bits & (mask << pos)) != 0; pos += count) {
        }
        if (pos == 32) {
            continue;
        }
        *word = (unsigned int)(bits | (mask << pos));
        return base + pos;
    }
}
