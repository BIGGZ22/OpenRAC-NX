/* func_L00_00409480 — NOT MATCHED, best attempt — 212 bytes expected, 27 places.
 * last check.py line: "00_aranos_tutorial: 216 bytes, expected 212"
 * then "func_L00_00409480: 0 of 27 places match".
 *
 * Only difference: the value `c` is born in $v1 then copied into $a2 for the
 * call (one `daddu $a2,$v1,$zero` too many), whereas the retail births it in
 * $a2 (`addiu $a2,$zero,0x80; subu $a2,$a2,$v0`). Everything else — including
 * the order of the shifts/or and the 4 calls — is identical.
 * (Putting the call before the rgba computation keeps `c` alive across the
 * call: cc1 then places it in $s0 and the body grows to 224 bytes. Worse.)
 */

extern int func_002EF250(float x);
extern void func_0037B508(int a, int b, int c);
extern void func_002E7DD8(int a, int b, int c, int d, int e);
extern short D_001A6480[];

void func_L00_00409480(char *arg)
{
    int *o = *(int **)(arg + 104);
    short *t;
    int c;
    int rgba;

    c = 128 - func_002EF250((float)(o[2] * 2));
    rgba = c << 24;
    rgba |= c << 16;
    rgba |= c << 8;
    rgba |= c;
    func_0037B508(66, 72, c);
    t = D_001A6480;
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
}
