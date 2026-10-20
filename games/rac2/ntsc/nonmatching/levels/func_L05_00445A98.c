/* Retained wave8/w10 draft, rechecked 2026-10-15.
   Compiler rebuilt from master bb4c620; -O2 -G0 -ffunction-sections.
   176 bytes versus 192; 0 of 3 placements match. The tail reuses the table and index instead of reloading them.
   No matching credit or behavioral validation. Original C body retained;
   missing declarations supplied and calls named from the current catalogue.
   Reproduce: copy this file to src/levels/shared/ with the same filename,
   remove its original INCLUDE_ASM marker, and run tools/check.py func_L05_00445A98 --diff.
   Reopen when the compiler/source can reproduce the measured differences. */
typedef int TI128 __attribute__((mode(TI)));
extern char *D_L05_001C9710;
extern char *D_L05_001B1EC4;
extern char *D_L05_001B251C;
extern TI128 D_L05_001B3600, D_L05_001B35D0, D_L05_001B35E0, D_L05_001B35F0;
extern void func_L05_00441D40(char *);

/* Saves the four 16-byte blocks of the state record and hands its bound block on. */
void func_L05_00445A98(void)
{
    char *state = D_L05_001C9710;

    if (*(short *)(state + 0x86) != 0x19)
        return;
    func_L05_00441D40(state);
    *(TI128 *)&D_L05_001B3600 = *(TI128 *)(state + 0x30);
    *(TI128 *)&D_L05_001B35D0 = *(TI128 *)(state + 0x00);
    *(TI128 *)&D_L05_001B35E0 = *(TI128 *)(state + 0x10);
    *(TI128 *)&D_L05_001B35F0 = *(TI128 *)(state + 0x20);
    {
        char *entry = D_L05_001B1EC4 + (*(short *)(state + 0x84) << 5);
        char *sub = *(char **)(entry + 0x1C);
        char *dst = D_L05_001B251C + (*(int *)(sub + 0x20) << 8) + 0x10;

        *(TI128 *)dst = *(TI128 *)(state + 0x30);
    }
}
