typedef struct {
    char pad0[0x38];
    int field38;
    char pad1[0xC];
    int field48;
    char pad2[0x14];
    int field60;
} GLOBAL_WAD;

extern GLOBAL_WAD global_wad;
extern void func_1E90960(int value);

void GlobalWad_ResetLoadedFlags__Fv(void) {
    func_1E90960(global_wad.field48);
    if (global_wad.field38 != 0) {
        func_1E90960(global_wad.field60);
    }
}
