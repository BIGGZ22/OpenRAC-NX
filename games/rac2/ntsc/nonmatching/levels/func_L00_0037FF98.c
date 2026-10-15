/* Near miss from src/levels/shared/L00_0037E898.c: 188 bytes; 156-byte candidate; 0/27. Candidate scans 0x100-stride entries and clears unused table slots. */
extern char *D_L00_001B1E9C;
extern int *D_L00_001B26B8[] __attribute__((sda));
extern char *D_L00_001B1EA4;
extern volatile int D_L00_001B2EA4 __attribute__((sda));

void func_L00_0037FF98(void)
{
    char *cursor = *(char **)&D_L00_001B1E9C;
    int count = 0;

    if ((unsigned int)*(char **)&D_L00_001B1EA4 < (unsigned int)cursor)
        return;
    do {
        if (*(short *)(cursor + 0xAA) == 0x135F) {
            D_L00_001B26B8[count] = (int *)cursor;
            count++;
        }
        (void)D_L00_001B2EA4;
        cursor += 0x100;
    } while ((unsigned int)cursor <= (unsigned int)*(char **)&D_L00_001B1EA4 && count < 16);
    while (count < 16) {
        D_L00_001B26B8[count] = 0;
        count++;
    }
}
