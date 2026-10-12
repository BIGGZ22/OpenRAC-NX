/* func_L01_003C08A0 */
void func_L01_003C08A0(char *a0)
{
    char *g = (char *)D_L01_00189E20;
    char *p = a0;
    struct L01_003C08A0_S {
        int x;
        int y;
        float z;
    } s;
    float v[4];
    int f;

    if (*(int *)(g + 0x2294) != 0x6F) {
        f = *(unsigned char *)(g + 0x22B5) != 0;
        *(int *)(p + 0x98) = 0;
        func_002BEB90(0x6F, 1);
        func_002C7928(10, 0, 10.0f);
        func_002A6CF0(D_L01_001AFF00, D_L01_001B2088);
        *(unsigned char *)(g + 0x22B5) = f;
    }
    *(unsigned short *)(g + 0x1F0) = 2;
    func_0029FCC0(p, 10, &s);
    s.z = s.z - 1.0f;
    func_00282CF0(v, &s, g + 0x80);
    func_002AC9F8(v, 0.06666667f);
    func_00282CC0(g + 0x80, g + 0x80, v);
    func_00282C48(g + 0xF0);
    func_00282C48(g + 0x110);
    func_00282C48(g + 0x120);
    func_00282C48(g + 0x130);
    *(unsigned char *)(g + 0x149B) = 0;
    *(unsigned char *)(g + 0x1496) = 0;
    *(int *)(g + 0x1B8) = 0;
    *(int *)(g + 0x18C0) = 0;
}
