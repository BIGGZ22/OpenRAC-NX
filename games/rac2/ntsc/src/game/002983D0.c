

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_8172b0f0a91a7cba_SDataClearMode112Layout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_8172b0f0a91a7cba_SDataClearMode112Layout;

extern Rac2Native_8172b0f0a91a7cba_SDataClearMode112Layout ref_L00_003095C8_00;

extern int ref_L00_003095C8_01;

extern int ref_L00_003095C8_02;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_b09e58bdde03cd06_SDataRootMarker76Layout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_b09e58bdde03cd06_SDataRootMarker76Layout;

extern Rac2Native_b09e58bdde03cd06_SDataRootMarker76Layout ref_L00_00309638_02;

extern int ref_L00_00309638_00;

extern int ref_L00_00309638_03;

extern int ref_L00_00309638_01;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_584c3844a02fd1eb_SDataBranch124RootLayout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_584c3844a02fd1eb_SDataBranch124RootLayout;

extern Rac2Native_584c3844a02fd1eb_SDataBranch124RootLayout ref_L00_00309688_02;

extern unsigned int ref_L00_00309688_00;

extern int ref_L00_00309688_03;

extern int ref_L00_00309688_01;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_a661193cb7db1192_SDataAccessorRootReset44Layout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
    unsigned char unknown_14[0x150];
    int selected_164;
    int value_168;
} Rac2Native_a661193cb7db1192_SDataAccessorRootReset44Layout;

extern Rac2Native_a661193cb7db1192_SDataAccessorRootReset44Layout ref_L00_00309708_00;

extern int ref_L00_00309708_01;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_1716667661d8e672_SDataMoreGuard96Layout {
    unsigned char unknown_00[0x15c];
    int mode_15c;
    unsigned char unknown_160[0x4];
    int selected_164;
    unsigned char unknown_168[0x4];
    int extra_16c;
} Rac2Native_1716667661d8e672_SDataMoreGuard96Layout;

extern Rac2Native_1716667661d8e672_SDataMoreGuard96Layout ref_L00_00309738_00;

extern unsigned int ref_L00_00309738_01;

extern int ref_L00_00309738_02;

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_715e9b46a377ec8c_SDataMoreBits76RootLayout {
    unsigned char unknown_00[0x8];
    int mode_08;
    unsigned char unknown_0c[0x4];
    int selector_10;
} Rac2Native_715e9b46a377ec8c_SDataMoreBits76RootLayout;

extern Rac2Native_715e9b46a377ec8c_SDataMoreBits76RootLayout ref_L00_00309798_01;

extern unsigned int ref_L00_00309798_00;

extern int ref_L00_00309798_02;

void func_002983D0(void)
{
    ref_L00_003095C8_01 &= ~0x20;
    if (ref_L00_003095C8_00.mode_08 != 2)
        return;
    if (ref_L00_003095C8_00.selector_10 == 0) {
        ref_L00_003095C8_02 = 9;
        return;
    }
    if (ref_L00_003095C8_00.selector_10 == -1) {
        ref_L00_003095C8_00.selector_10 = 0;
        ref_L00_003095C8_02 = 9;
        return;
    }
    if (ref_L00_003095C8_00.selector_10 == -2)
        ref_L00_003095C8_02 = 5;
}

void func_00298440(void)
{
    if (ref_L00_00309638_02.selector_10 != -2) {
        ref_L00_00309638_03 = 3;
        return;
    }
    if (ref_L00_00309638_01 != 0 || (ref_L00_00309638_00 & 2) != 0)
        ref_L00_00309638_03 = 6;
}

void func_00298490(void)
{
    if (ref_L00_00309688_02.selector_10 != -2) {
        ref_L00_00309688_03 = 3;
        return;
    }
    if ((ref_L00_00309688_00 & 0x20u) != 0) {
        ref_L00_00309688_00 ^= 0x20u;
        if (ref_L00_00309688_01 != 0)
            ref_L00_00309688_03 = 23;
        else
            ref_L00_00309688_03 = 5;
        return;
    }
    if ((ref_L00_00309688_00 & 0x8u) != 0) {
        ref_L00_00309688_00 ^= 0x8u;
        ref_L00_00309688_03 = 7;
    }
}

void func_00298510(void)
{
    ref_L00_00309708_00.selector_10 = 0;
    if (ref_L00_00309708_00.selected_164 < 0) {
        ref_L00_00309708_00.value_168 = 0;
        ref_L00_00309708_00.selected_164 = 3;
    }
    ref_L00_00309708_01 = 8;
}

void func_00298540(void)
{
    if (ref_L00_00309738_00.mode_15c != 2 || ref_L00_00309738_00.selected_164 >= 0)
        return;
    if (ref_L00_00309738_00.extra_16c != 0) {
        ref_L00_00309738_02 = 17;
        ref_L00_00309738_01 |= 0x40u;
        return;
    }
    ref_L00_00309738_02 = 14;
}

void func_002985A0(void)
{
    if (ref_L00_00309798_01.selector_10 != 0) {
        ref_L00_00309798_02 = 3;
        return;
    }
    if ((ref_L00_00309798_00 & 6u) != 0) {
        ref_L00_00309798_02 = 10;
        return;
    }
    if ((ref_L00_00309798_00 & 0x200u) != 0)
        ref_L00_00309798_02 = 25;
}
