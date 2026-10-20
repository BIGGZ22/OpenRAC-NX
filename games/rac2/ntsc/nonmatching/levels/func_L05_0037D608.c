/* 0 of 4 places match - best candidate: 05_feltzin_system 216 bytes, expected 224, 41 words
   differ (the literal-base shape below; the symbol shape is 220). The record walk, the two
   transform calls and the loop tail are understood and land instruction for instruction: the
   two-byte records indexed by the object's +0x21 field are walked, bits 0-14 name a cache slot
   ((value & 0x7FFF) << 8 off the pointer at 0x001B251C) and bit 15 ends the list.
   The wall is that base pointer: the retail materialises `lui $a0,0x1B; lw $a0,0x251C($a0)` at
   EACH site, while this cc1 hoists the address's lui out of the loop into a saved register
   (one extra saved register, 0x50 frame instead of 0x40). A volatile pointer does not stop it,
   an alias on the same address makes it worse (two saved registers).
   With the address written as a literal the prologue matches byte for byte and the first
   difference moves to the table load, where this cc1 loads the entry into $s0 directly instead
   of loading into $v0, testing and then moving to $s0 as retail does.
   Belongs to src/levels/25_wupash_nebula/L25_00373ED0-g8.c (draft removed from the file). */

typedef unsigned char u8;

extern int func_002ED688(unsigned char *);

extern float func_002A7940(void);

extern void func_002AF000(float *, float, float, float);

extern void func_00282CC0(void *, void *, void *);

extern void func_002EE920(void *, float *);

extern short *D_L05_001FB220[] __attribute__((nosda));

#define CACHE_BASE (*(u8 **)0x001B251C)

void func_L05_0037D608(void *object, void *argument, u8 value)
{
    unsigned short *p = D_L05_001FB220[((unsigned char *)object)[0x21]];

    if (p != 0) {
        float vector[4];

        while (1) {
            int offset = (*p & 0x7FFF) << 8;

            if (func_002ED688(CACHE_BASE + offset) != 0) {
                float a = func_002A7940();
                float b = func_002A7940();

                func_002AF000(vector, value, a, b);
                func_00282CC0(vector, vector, argument);
                func_002EE920(CACHE_BASE + offset, vector);
            }
            if ((short)*p < 0)
                break;
            p++;
        }
    }
}
