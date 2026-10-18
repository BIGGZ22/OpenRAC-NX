/* _sceCd_cd_callback — N-command completion callback.
   RAC2 src/core/00124568.c func_00124630. RAC5 data addresses from the
   retail lui 0x1F0 + signed offsets (base 0x01F00000):
     -0x38A0 -> 0x01EFC760 (r1_D_00131414), -0x389C -> 0x01EFC764 (..18)
     -0x38D8 -> 0x01EFC728 (r1_D_001313E8), -0x38EC -> 0x01EFC714 (..D4)
     -0x38E0 -> 0x01EFC720 (r1_D_001313E0), -0x38CC -> 0x01EFC734 (..F0)
     lui 0x1FE -0x23C0 -> 0x01FDDC40 (r1_D_00159840)
   iSignalSema resolves from config/boot/symbols.tsv. */

extern volatile int D_01EFC760;
extern volatile int D_01EFC764;
extern volatile int D_01EFC714;
extern volatile int D_01EFC720;
extern volatile int D_01EFC728;
extern volatile int D_01EFC734;
extern void *volatile D_01FDDC40;

extern int iSignalSema(int);

void _sceCd_cd_callback(int *arg0) {
    D_01EFC760 = *arg0;
    D_01EFC764 = D_01EFC760;
    if (D_01EFC760 == 0xB) {
        D_01EFC760 = 0;
        D_01EFC734 = 0;
        return;
    }
    iSignalSema(D_01EFC728);
    if (D_01EFC714 != 0 && D_01FDDC40 != 0) {
        iSignalSema(D_01EFC720);
    } else {
        D_01EFC734 = 0;
    }
    D_01EFC760 = 0;
}
