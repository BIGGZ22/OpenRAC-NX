extern void mcps2_ctor_1E64A38(int initialize, unsigned int priority);

void GLOBAL_I_SYSTEMS_memcard_ps2(void) __asm__("_GLOBAL_$I$SYSTEMS_memcard_ps2.cppBRcaaa");

void GLOBAL_I_SYSTEMS_memcard_ps2(void) {
    mcps2_ctor_1E64A38(1, 0xFFFF);
}
