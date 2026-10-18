typedef struct {
    unsigned char raw[0x58];
} SND_Positional;

extern unsigned char positional_sound_arena;
extern volatile unsigned char positional_sounds_locked;

SND_Positional *SND_GetPositionalSoundAt__FUi(unsigned int index) {
    if (positional_sounds_locked != 0 && index < 0x40) {
        return &((SND_Positional *)&positional_sound_arena)[index];
    }
    return 0;
}
