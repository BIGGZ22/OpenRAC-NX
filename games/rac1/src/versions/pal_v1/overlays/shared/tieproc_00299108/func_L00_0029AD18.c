void play_queued_movie(void) {
    func_00118D80(0);
    sound_StopAllSounds();
    music_Stop();
    func_0012EF48(0);
    for (;;) {
        char *ld = memory_card_state;
        if (*(int *)(ld + 0xD4) < 3 && *(int *)(ld + 0xDC) < 0) {
            break;
        }
        memcard_update_state();
    }
    Movie_Play(D_L00_00161108, D_L00_0016110C,
                  (*(int *)&D_L00_00161114 + 0x3F) & ~0x3F,
                  (D_L00_00161118 + 0x3F) & ~0x3F, D_L00_00161110);
    func_00120F30(0);
    func_L00_0029AB38();
}
