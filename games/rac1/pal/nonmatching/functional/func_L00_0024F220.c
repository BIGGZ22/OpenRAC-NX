/* functional: medium. Returns 2 if byte D_0013D5DD is nonzero (the func_L00_002347F0 tail), else (word at D_0013DC34 != 0). */
extern unsigned char D_0013D5DD_4F220[] __asm__("D_0013D5DD") NOT_SDA;
extern int D_0013DC34_4F220 __asm__("D_0013DC34") NOT_SDA;
int func_L00_0024F220(void) {
    if (D_0013D5DD_4F220[0] != 0) {
        return 2;
    }
    return D_0013DC34_4F220 != 0;
}
