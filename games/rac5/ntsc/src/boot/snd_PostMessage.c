extern int snd_message_index;
extern int *snd_message_slots[];
extern int snd_FlushSoundCommands(void);

void snd_PostMessage(void) {
    int *slot = snd_message_slots[snd_message_index];
    (*slot)++;
    snd_FlushSoundCommands();
}
