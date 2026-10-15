/* Near miss from src/levels/12_todano/L12_002CF7B0.c: the build refuses this body at its
   place — 100 bytes, expected 108. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/12_todano/L12_002CF7B0.c. */
extern char *D_L12_001BDB00;

extern float D_L12_001AD230;

extern float D_L12_001AD234;

void func_L12_00300138(void)
{
    char *p = D_L12_001BDB00;

    if (*(int *)(p + 0x58) != 0) {
        D_L12_001AD230 = 0.1f;
        D_L12_001AD234 = 0.02f;
        return;
    }
    if ((unsigned int)(*(int *)(p + 0x30) - 3) < 2) {
        D_L12_001AD230 = 0.0f;
        D_L12_001AD234 = 0.0f;
    }
}
