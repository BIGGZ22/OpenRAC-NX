/* functional: high. The body from func_0023B578 onward (the 0x50 bytes that are code); the 0x68-byte prefix of the 188-byte stub is not code and is left out. Same reads, writes and call as the assembly. */
extern void func_0023B5D0(char *arg0);

void func_0023B510(char *arg0) {
    if (*(unsigned char *)(arg0 + 0x70) & 0x2) {
        float f0 = *(float *)(arg0 + 0x58);
        float f1 = 0.0f;
        float f2 = 1.0f;
        if (f1 < f0) {
            f2 = f1;
        }
        *(float *)(arg0 + 0x54) = f2;
        *(float *)(arg0 + 0x58) = f1;
    }
    func_0023B5D0(arg0);
}
