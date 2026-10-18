/* Measured globals, call targets and boot placement; no game bytes. */
transition_bank = 0x01EF6978;
snd_UnloadBank = 0x01E94D90;
snd_FlushSoundCommands = 0x01E94700;
DelayThread = 0x01ED1A70;
SECTIONS { . = 0x01E8FE80; .text : { *(.text) } }
