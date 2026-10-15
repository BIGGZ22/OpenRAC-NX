/* First plain-C attempt from worker w08. Last check: 296 bytes vs 348; 0/27. */
extern int D_001A79F0;
extern int D_001A73D8;
extern int D_001A73E0[];
extern int D_0013A148[];
extern unsigned char D_0019B2E8[];
extern int D_001A24A8[];
extern int D_L00_001B19C8;
extern unsigned char D_00189E20_1[];
extern void func_L00_002F45B8(void);
extern void func_0027D330(int);

void func_L00_002B30E8(void)
{
    char *state = (char *)D_00189E20_1;
    int player = D_001A79F0;
    int slot;
    int id;
    int threshold;
    int index;
    int address;

    D_0013A148[player]++;
    D_001A73D8++;
    func_L00_002F45B8();
    slot = (unsigned char)state[0x22C6];
    if (slot == 0xFF)
        return;
    id = *(int *)(state + 0x2294);
    if (id == 0x74 || id == 0x79)
        threshold = 0x708;
    else
        threshold = 0x384;
    if (D_L00_001B19C8 < threshold)
        return;
    address = player * 16 + slot;
    if ((D_0019B2E8[address] & 0x80) != 0)
        return;
    index = (player << 4) + (slot & 0xF);
    D_001A24A8[index]++;
    D_001A73E0[slot & 0xF]++;
    D_001A73E0[slot]++;
    func_0027D330(0x10);
    state[0x22C1] = 1;
}
