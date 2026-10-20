/* Correct 64-byte matrix buffer changes the entry; original unit 0 of 4 places.
 * Requires declarations/types from src/levels/shared/L01_00397110-g8.c. */
void func_L01_00417090(char *a0)
{
    TI128 v0[4], v1, v2;
    int i;
    int b;

    if (*(unsigned char *)(a0 + 0x20) == 1) {
        func_00282D30(&v0, a0, 0.0009765625f);
    } else {
        func_0029FCC0(a0, 0, &v0);
    }
    i = 2;
    func_002A7A00(&v1, D_L01_001B1A9C * 0.016666668f, D_L01_001B1AA0 * 0.016666668f);
    b = D_L01_001B1AAC;
    func_002C3150(&v0, &v1, D_L01_001B1AB0, b, (unsigned char)func_002A77E0(255), 0, D_L01_001B1AB8);
    do {
        i--;
        func_002A7A00(&v2, D_L01_001B1AA4 * 0.016666668f, D_L01_001B1AA8 * 0.016666668f);
        func_00282CC0(&v1, &v1, &v2);
        b = D_L01_001B1AAC;
        func_002C3150(&v0, &v1, D_L01_001B1AB4, b, (unsigned char)func_002A77E0(255), 0, D_L01_001B1ABC);
    } while (i >= 0);
}
