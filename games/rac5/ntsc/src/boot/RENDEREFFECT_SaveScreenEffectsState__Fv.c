extern unsigned char re_src_a;
extern unsigned char re_src_b;
extern unsigned char re_src_c;
extern unsigned char re_dst_a;
extern unsigned char re_dst_b;
extern unsigned char re_dst_c;

void RENDEREFFECT_SaveScreenEffectsState__Fv(void) {
    re_dst_a = re_src_a;
    re_dst_b = re_src_b;
    re_dst_c = re_src_c;
}
