

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_99b76d48da369e8d_SDataMoreThreshold156Layout {
    unsigned char unknown_00[0xc];
    int value_0c;
    int selector_10;
    unsigned char unknown_14[0x4];
    short phase_18;
    unsigned char unknown_1a[0x6];
    int value_20;
    unsigned char unknown_24[0x138];
    int mode_15c;
    unsigned char unknown_160[0x4];
    int selected_164;
    unsigned char unknown_168[0x4];
    int extra_16c;
} Rac2Native_99b76d48da369e8d_SDataMoreThreshold156Layout;

extern Rac2Native_99b76d48da369e8d_SDataMoreThreshold156Layout ref_L00_00309828_00;

extern int ref_L00_00309828_01;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_a0e246aa2dc0b8e7_SDataAccessorRoot56Layout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_a0e246aa2dc0b8e7_SDataAccessorRoot56Layout;

extern Rac2Native_a0e246aa2dc0b8e7_SDataAccessorRoot56Layout ref_L00_003098C8_01;

extern int ref_L00_003098C8_00;

extern int ref_L00_003098C8_02;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_998ff33d7f808dbf_SDataBranch120RootLayout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_998ff33d7f808dbf_SDataBranch120RootLayout;

extern Rac2Native_998ff33d7f808dbf_SDataBranch120RootLayout ref_L00_00309900_02;

extern unsigned int ref_L00_00309900_00;

extern int ref_L00_00309900_03;

extern int ref_L00_00309900_01;

void func_00298630(void)
{
    short phase;
    if (ref_L00_00309828_00.mode_15c != 2 || ref_L00_00309828_00.selected_164 >= 0)
        return;
    if (ref_L00_00309828_00.extra_16c != 0) {
        ref_L00_00309828_01 = 12;
        return;
    }
    if (ref_L00_00309828_00.selector_10 < -1) {
        ref_L00_00309828_01 = 3;
        return;
    }
    phase = ref_L00_00309828_00.phase_18;
    if (phase == -2) {
        if (ref_L00_00309828_00.value_0c + ref_L00_00309828_00.value_20 < 475)
            ref_L00_00309828_01 = 19;
        else
            ref_L00_00309828_01 = 12;
        return;
    }
    if (phase >= -1)
        ref_L00_00309828_01 = 16;
}

void func_002986D0(void)
{
    if (ref_L00_003098C8_01.selector_10 != 0) {
        ref_L00_003098C8_02 = 3;
        return;
    }
    if ((ref_L00_003098C8_00 & 2) != 0)
        ref_L00_003098C8_02 = 13;
}

void func_00298708(void)
{
    if (ref_L00_00309900_02.selector_10 != 0) {
        ref_L00_00309900_03 = 3;
        return;
    }
    if ((ref_L00_00309900_00 & 0x20u) != 0) {
        ref_L00_00309900_00 ^= 0x20u;
        if (ref_L00_00309900_01 != 0)
            ref_L00_00309900_03 = 24;
        else
            ref_L00_00309900_03 = 12;
        return;
    }
    if ((ref_L00_00309900_00 & 0x10u) != 0) {
        ref_L00_00309900_00 ^= 0x10u;
        ref_L00_00309900_03 = 14;
    }
}
