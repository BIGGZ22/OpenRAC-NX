/* func_L06_0041FD60 (44 bytes) -- ABANDON (best of 3 shapes).
 * Every shape compiles to the same size and the same instruction sequence but a
 * different register assignment:
 *   retail: addiu v1,zero,0xFF ; lbu v0,0x21(a0) ; beq v0,v1 ; delay daddu a0,v0
 *   mine  : addiu v0,zero,0xFF ; lbu a0,0x21(a0) ; beq a0,v0 ; delay ld ra
 * The load coalesces into $a0 (its destination as arg 0 of the call) instead of
 * staying in $v0 with an explicit copy. Tried: unsigned char v / unsigned int v /
 * named `int last = 0xFF` constant. All identical output.
 */
extern void func_35B810(int, int);

void func_L06_0041FD60(char *a0)
{
    unsigned char v = *(unsigned char *)(a0 + 0x21);

    if (v != 0xFF)
        func_35B810(v, 1);
}
