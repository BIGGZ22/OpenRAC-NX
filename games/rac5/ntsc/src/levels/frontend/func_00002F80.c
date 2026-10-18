typedef unsigned char u8;
typedef unsigned int u32;

extern u8 *D_00027E9C;
extern u8 MCP_WideScreenMaster;
extern int func_00002F28(void);
extern char *strncpy(char *, const char *, unsigned int);
extern void *memset(void *, int, unsigned int);
extern void func_000036D8(void);
extern void func_00003B58(void);
extern void func_000032E8(int mode);
extern float SND_GetGlobalMusicVolume__Fv(void);
extern int func_000240A0(float value);
extern int func_00002F78(void);
extern void func_00003968(void);

#define FIELD32(offset) (*(u32 *)(D_00027E9C + (offset)))
#define FIELDI32(offset) (*(int *)(D_00027E9C + (offset)))
#define FIELDF32(offset) (*(float *)(D_00027E9C + (offset)))

/* Frontend New Game state initialization; field names remain unproven. */
void func_00002F80(u8 *state, int mode) {
    u8 preserved[256];
    u32 music;

    D_00027E9C = state;
    *(int *)(state + 0x1c70) = -1;
    if (func_00002F28() == 0 || mode != 0) {
        strncpy((char *)preserved, (const char *)D_00027E9C + 0x60, 0x20);
        memset(D_00027E9C, 0, 0x13f60);
        strncpy((char *)D_00027E9C + 0x60, (const char *)preserved, 0x20);
        func_000036D8();
        func_00003B58();
        func_000032E8(mode);
        D_00027E9C[0] = 1;
        FIELD32(0x1c74) = 0;
        FIELD32(0x1c6c) = 1;
        FIELD32(0x13720) = 0;
        FIELD32(0x13728) = 0;
        FIELD32(0x1c78) = 0;
        FIELD32(0x13f40) = 0;
        FIELDI32(0x1c70) = -1;

        music = (u32)func_000240A0(SND_GetGlobalMusicVolume__Fv() * 255.0f);
        FIELD32(0x08) = 0;
        FIELD32(0x0c) = 0;
        FIELD32(0x14) = 1;
        FIELD32(0x10) = 1;
        FIELD32(0x18) = music;
        FIELD32(0x1c) = 0xff;
        FIELDI32(0x48) = -1;
        FIELDI32(0x4c) = -1;
        FIELD32(0x24) = 0;
        FIELDF32(0x28) = 1.0f;
        FIELDF32(0x2c) = -1.0f;
        FIELDF32(0x30) = 1.0f;
        FIELDF32(0x34) = 1.0f;
        FIELD32(0x38) = 1;
        FIELD32(0x44) = 3;
        FIELD32(0x50) = 2;
        FIELD32(0x3c) = func_00002F78();
        FIELD32(0x40) = 0;
        func_00003968();
        FIELD32(0x20) = MCP_WideScreenMaster;
    }
}
