/* File-scope static constructor for HUD_hudQueuedPrompter.cpp. */
extern void rac5_static_init_1E8B108(int initialize_p, int priority);

void rac5_ctor_hud_queued_prompter(void)
    __asm__("_GLOBAL_$I$HUD_hudQueuedPrompter.cppgPcaaa");

void rac5_ctor_hud_queued_prompter(void) {
    rac5_static_init_1E8B108(1, 0xFFFF);
}
