

extern unsigned int D_001A79F0_1[] __attribute__((sda));

typedef int s32;

extern s32 D_001A79F0_2;

extern s32 func_L00_002D67A8(s32);

/* A13-eq-getter-20 freshly exact-verified under family reservation */
unsigned int func_L00_002D6710(void) { return D_001A79F0_1[0] == 0x19u; }

/* Return one when the resident mode word is25,5,10 or15, otherwise zero */
s32 func_L00_002D6728(void) {
    s32 found = 0;
    if (D_001A79F0_2 == 25 || D_001A79F0_2 == 5 ||
        D_001A79F0_2 == 10 || D_001A79F0_2 == 15) {
        found = 1;
    }
    return found;
}

/* calls the measured integer callee with zero */
void func_L00_002D6768(void) {
 func_L00_002D67A8(0);
}

/* calls the measured integer callee with three */
void func_L00_002D6788(void) {
 func_L00_002D67A8(3);
}
