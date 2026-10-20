/* func_00132220, 148 bytes, boot. 989snd's snd_ServiceRpcCompletion: it polls
   the command channel's RPC completion (chan 0x1A7040) and either clears the
   pending descriptor or prints an error.
   The $gp-relative globals are gp-0x7B70 (0x1A7480), -0x7B6C (0x1A7484) and
   -0x7AF8 (0x1A74F8); the unit is built with -G8 by the file name, plus
   -mno-split-addresses: retail keeps `lui $a0,%hi ; addiu $a0,$a0,%lo`
   adjacent with a `nop` in the jal delay slot, which only the single
   (unsplit) symbol operand gives. */
extern unsigned int *D_001A7480 __attribute__((sda));
extern unsigned int D_001A7484 __attribute__((sda));
extern unsigned int D_001A74F8 __attribute__((sda));

extern void func_0011AEA0(int);
extern int func_0011D810(void *);
extern void func_00115D48(char *);

extern char D_0013BFA0[];
extern char D_001A7040[] __attribute__((sda));

int func_00132220(void) {
    unsigned int *p;

    func_0011AEA0(0);
    if (D_001A7480 == 0)
        return 1;
    if (func_0011D810(D_001A7040) != 0)
        return 0;
    p = D_001A7480;
    if (p[0] == 0xFFFFFFFFu && p[1 + D_001A7484] == p[0]) {
        D_001A7480 = 0;
        return 1;
    }
    if (D_001A74F8 == 0)
        func_00115D48(D_0013BFA0);
    return 0;
}
