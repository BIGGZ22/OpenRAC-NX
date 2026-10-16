/*
 * UploadMipTexture: sends a texture (a header, then its palette and mip levels) to the GS at the
 * texture allocation cursor and writes its TEX0, MIPTBP1/TEX1 and MIPTBP1 values to regs[0..2].
 * Used for the planet pictures and area captions of the level transition. Adapted from Lombyte
 * (MIT): src/assembly/textbin/fun_00202270.c (US 0x00202270, the same function).
 */
typedef struct {
    u8 pad0[8];
    s32 width;
    s32 height;
    s32 psm;
    s32 palette_psm;
    s32 pad18;
    s32 mip_count;
    u8 data[4];
} MipTexture_202AA8;

typedef struct {
    s32 palette_address;
    s32 mip_addresses[4];
    s32 palette_size;
    s32 mip_sizes[4];
    s32 palette_block;
    s32 blocks[4];
    s32 buffer_widths[4];
    s32 width_log2;
    s32 height_log2;
} MipUpload_202AA8;

/* sceGsLoadImage: twelve doublewords. */
typedef struct {
    u64 q[12];
} LoadImage_202AA8 __attribute__((aligned(16)));

/* As loaders.c declares them further down (sceGsSetDefLoadImage, sceGsExecLoadImage, FlushCache,
 * sceGsSyncPath). */
extern int D_0015EF74 MACRO_ADDR; /* gs texture allocation cursor */
extern void func_001F99B0(void *, s32, s32);
extern s32 func_001F9968(s32);
extern void func_00118D80(int);
extern int func_00122630(void *, short, short, short, short, short, short, short);
extern int func_00122958(void *, void *);
extern int func_00120858(int, unsigned short);

s32 func_00202AA8(MipTexture_202AA8 *tex, u64 *regs)
{
    MipUpload_202AA8 upload;
    LoadImage_202AA8 load_image;
    s32 i;
    s32 bytes;
    s32 *buffer_width;
    u64 tex0;
    u64 mip;

    func_001F99B0(&upload, 0, sizeof(upload));
    switch (tex->psm) {
    case 0:
    case 2:
        upload.palette_address = 0;
        upload.palette_size = 0;
        break;
    case 0x13:
        upload.palette_address = (s32)tex->data;
        upload.palette_size = tex->palette_psm != 0 ? 0x200 : 0x400;
        break;
    case 0x14:
        upload.palette_address = (s32)tex->data;
        upload.palette_size = tex->palette_psm == 0 ? 0x40 : 0x20;
        break;
    default:
        break;
    }
    upload.width_log2 = func_001F9968(tex->width);
    upload.height_log2 = func_001F9968(tex->height);
    upload.mip_addresses[0] = (s32)tex->data + upload.palette_size;
    switch (tex->psm) {
    case 0:
        upload.mip_sizes[0] = tex->width * tex->height * 4;
        break;
    case 2:
        upload.mip_sizes[0] = tex->width * tex->height * 2;
        break;
    case 0x13:
        upload.mip_sizes[0] = tex->width * tex->height;
        break;
    case 0x14:
        upload.mip_sizes[0] = (tex->width * tex->height) >> 1;
        break;
    }
    if (tex->psm == 0x13 || tex->psm == 0x14) {
        upload.palette_block = D_0015EF74 >> 8;
        if (tex->psm == 0x14) {
            D_0015EF74 += 0x100;
            func_00122630(&load_image, upload.palette_block, 1, tex->palette_psm, 0, 0, 8, 2);
        } else {
            D_0015EF74 += upload.palette_size;
            func_00122630(&load_image, upload.palette_block, 1, tex->palette_psm, 0, 0, 16, 16);
        }
        func_00118D80(0);
        func_00122958(&load_image, (void *)upload.palette_address);
        func_00120858(0, 0);
    }
    for (i = 1; i < tex->mip_count; i++) {
        upload.mip_sizes[i] = upload.mip_sizes[i - 1] >> 2;
        upload.mip_addresses[i] = upload.mip_addresses[i - 1] + upload.mip_sizes[i - 1];
    }
    for (i = 0; i < tex->mip_count; i++) {
        buffer_width = &upload.buffer_widths[i];
        *buffer_width = tex->width >> (i + 6);
        if (*buffer_width <= 0) {
            *buffer_width = 1;
        }
        upload.blocks[i] = D_0015EF74 >> 8;
        func_00122630(&load_image, upload.blocks[i], *buffer_width, tex->psm, 0, 0,
                      tex->width >> i, tex->height >> i);
        func_00118D80(0);
        func_00122958(&load_image, (void *)upload.mip_addresses[i]);
        func_00120858(0, 0);
        bytes = upload.mip_sizes[0] >> (i * 2);
        if (bytes <= 0xFF) {
            bytes = 0x100;
        }
        D_0015EF74 += bytes;
    }
    tex0 = (u64)upload.blocks[0];
    tex0 |= (u64)upload.buffer_widths[0] << 14;
    tex0 |= (u64)tex->psm << 20;
    tex0 |= (u64)upload.width_log2 << 26;
    tex0 |= (u64)upload.height_log2 << 30;
    tex0 |= (u64)upload.palette_block << 37;
    tex0 |= (u64)1 << 34;
    tex0 |= (u64)tex->palette_psm << 51;
    tex0 |= (u64)1 << 63;
    regs[0] = tex0;
    mip = (u64)upload.blocks[1];
    mip |= (u64)upload.buffer_widths[1] << 14;
    mip |= (u64)upload.blocks[2] << 20;
    mip |= (u64)upload.buffer_widths[2] << 34;
    mip |= (u64)upload.blocks[3] << 40;
    mip |= (u64)upload.buffer_widths[3] << 54;
    regs[1] = ((u64)(tex->mip_count - 1) << 2) | 0xFFA0000000E0ULL;
    regs[2] = mip;
    return -1;
}
