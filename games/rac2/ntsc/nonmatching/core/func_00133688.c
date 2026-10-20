/* from src/core/00133490.c: the two address forms of one -g8 unit do not mix here */
/*
 * Same switch as func_00133640 (the flag at gp - 0x7B64): with it clear the
 * handler at func_00125588 runs and its value is returned, otherwise the
 * counter at 0x001A7110 is. The body is two instructions from exact; what
 * does not come out is the pair of address forms retail has in ONE unit:
 *
 *     lw  $v0,-0x7B64($gp)     ; the flag, gp-relative
 *     lui $v0,0x1A ; lw $v0,0x7110($v0)   ; the counter, absolute, folded
 *
 * In a -g8 file the counter declared `sda` becomes gp-relative as well (44
 * bytes, no lui at all), and declared `NOT_SDA` it comes out as two registers
 * (`lui $v1,0 ; lw $v0,0($v1)`, 48 bytes). The folded absolute form is the
 * -G0 + sda one, which the same file cannot be. func_001330D0 has the same
 * wall from the other side: its table base is `addiu $v0,$gp,-0x7B50` in
 * retail (28 bytes there) and absolute here.
 */
extern int D_001A748C __attribute__((sda));

extern int D_001A7110;

extern int func_00125588(void);

int func_00133688(void) {
    if (D_001A748C == 0)
        return func_00125588();
    return D_001A7110;
}
