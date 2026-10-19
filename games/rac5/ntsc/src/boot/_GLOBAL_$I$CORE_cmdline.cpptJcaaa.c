extern void core_cmdline_ctor_1E64CE0(int initialize, unsigned int priority);

void GLOBAL_I_CORE_cmdline(void) __asm__("_GLOBAL_$I$CORE_cmdline.cpptJcaaa");

void GLOBAL_I_CORE_cmdline(void) {
    core_cmdline_ctor_1E64CE0(1, 0xFFFF);
}
