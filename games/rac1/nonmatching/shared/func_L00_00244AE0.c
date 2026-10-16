/* NON_MATCHING func_L00_00244AE0 -- src/overlays/shared/loaders_00240398.c
 * Best so far: SIZE ours 4268 / retail 4264, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   4264-byte LoadLevelCoreData. FrameC0; all integer savedregs and FP20/21, stackarguments0/4, nextfree8. First p
 *   Trials: p0 COMPILE (leftover m2c temp); p1 SIZE4216/4264 typed reconstruction; p2 SIZE4224 reassociated textur
 *   p9 SIZE4272 real packed access plus reset counter and logger delta pseudo; p10 SIZE4244 natural logging OR con
 *   ## Main-agent continuation
 *   Runs p15/p16 SIZE4244: void logger and integer class-address table. p17 SIZE4252: macro address on moby offset
 */
#include "common.h"

typedef struct {
    s32 f0, f4, f8, fC, f10, f14, f18, f1C, f20, f24, f28, f2C;
    s32 f30, f34, f38, f3C, f40, f44, f48, f4C, f50, f54, f58, f5C;
    s32 f60, f64, f68, f6C, f70, f74, f78, f7C, f80, f84, f88, f8C;
    s32 f90, f94, f98, f9C, fA0, fA4, fA8, fAC, fB0, fB4, fB8, fBC, fC0, fC4;
} CoreHeader_244AE0;
typedef struct { s32 f0; CoreHeader_244AE0 *f4; s32 f8, fC, f10, f14, f18, f1C; } LevelMemory_244AE0;
typedef struct { s32 f0, f4, f8, fC, f10, f14, f18; } PackedHeader_244AE0;
typedef struct { s32 offset; u16 mode; u8 pad[10]; } ClassOffset_244AE0;
typedef struct { char pad[0x1A]; s16 texture; s32 stamp; } TexturePacket_244AE0;
typedef struct { char pad[12]; u8 class_count, texture_count; char padE[0x1A]; TexturePacket_244AE0 *packets; char pad2C[0x1C]; s32 bindings[1]; } ClassHeader_244AE0;
typedef struct { u64 value[3]; } TextureSetup_244AE0;
typedef struct { char pad[16]; } __attribute__((aligned(16))) Vec_244AE0;
typedef struct { s32 f0; char pad4[4]; s32 f8,fC; s32 f10; } __attribute__((packed)) UpdateState_244AE0;
typedef struct { s32 value; } __attribute__((packed)) UnalignedWord_244AE0;
extern char update_bytes_244AE0[] __asm__("D_L00_00179200");

extern LevelMemory_244AE0 mem_244AE0 __asm__("D_L00_00173F00");
extern PackedHeader_244AE0 *volatile packed_244AE0 __asm__("D_0015EF4C") MACRO_ADDR;
extern TextureSetup_244AE0 setup1_244AE0 __asm__("D_L00_001828C0");
extern TextureSetup_244AE0 setup2_244AE0 __asm__("D_L00_001828D8");
extern TextureSetup_244AE0 setup3_244AE0 __asm__("D_L00_001828F0");
extern ClassHeader_244AE0 *classes_244AE0[] __asm__("D_L00_00197680");
extern s32 root_address_244AE0 __asm__("D_L00_00197680") NOT_SDA;
extern s32 class_ids_244AE0[] __asm__("D_L00_001AFF40");
extern s32 class_bases_244AE0[] __asm__("D_L00_001AFFA0");
extern s32 class_flags_244AE0[] __asm__("D_L00_001B0000");
extern Vec_244AE0 class_vectors_244AE0[] __asm__("D_L00_001B0060");
extern s16 packet_lists_244AE0[][16] __asm__("D_L00_001B01E0");
extern s16 type_ids_244AE0[] __asm__("D_L00_00197D80");
extern u8 texture_ids_244AE0[] __asm__("D_L00_00197F40");
extern s32 placements_244AE0[] __asm__("D_0014C150");
extern s32 reset_ids_244AE0[] __asm__("D_0014EF90");
extern s32 tfrag_classes_244AE0[] __asm__("D_L00_001C4D00");
extern s32 moby_classes_244AE0[] __asm__("D_L00_00199E00");
extern s32 tie_classes_244AE0[] __asm__("D_L00_001C6A00");
extern s32 shrub_classes_244AE0[] __asm__("D_L00_001BC940");
extern ClassOffset_244AE0 moby_offsets_244AE0[] __asm__("D_L00_001AEF40") MACRO_ADDR;
extern TexturePacket_244AE0 *packets_244AE0 __asm__("D_L00_0015F6D4") MACRO_ADDR;
extern UpdateState_244AE0 update_244AE0 __asm__("D_L00_00179200");
extern char module_end_244AE0[] __asm__("D_L00_002F2298");
extern s32 tie_bases_244AE0[] __asm__("D_L00_001C5B00");
extern s32 collision_244AE0[] __asm__("D_L00_00173F40");
extern s32 data_base_244AE0[] __asm__("D_L00_001B1E00");
extern char mission_flags_244AE0[] __asm__("D_L00_0015FD48");
extern char newline_244AE0[] __asm__("D_L00_0015FD40");
extern s32 count_small_244AE0 SDATA(D_L00_00160088);
extern s32 f6e0_small_244AE0 SDATA(D_L00_0015F6E0);
extern s32 f6e4_small_244AE0 SDATA(D_L00_0015F6E4);

extern void reset_core_244AE0(void) __asm__("func_002032D0");
extern void copy_244AE0(void *,const void *,s32) __asm__("func_001F9A98");
extern void copy_short_244AE0(void *,const void *,s32) __asm__("func_L00_001FF040");
extern void relocate_chunks_244AE0(s32,s32,void *) __asm__("func_00203958");
extern void set_intr_244AE0(s32) __asm__("func_00118D80");
extern s32 decode_244AE0(void *,s32) __asm__("func_0020C468");
extern s32 texture_mode_244AE0(s32) __asm__("func_001F9968");
extern void tfrag_load_244AE0(s32,void *) __asm__("func_00204918");
extern void sky_load_244AE0(s32) __asm__("func_00203118");
extern void collision_load_244AE0(char *) __asm__("func_L00_002420C0");
extern void moby_load_244AE0(s32,void *,void *,s32) __asm__("func_00203E78");
extern void moby_fix_244AE0(void *,s32) __asm__("func_L00_00242120");
extern void class_fix_244AE0(ClassHeader_244AE0 *,s32) __asm__("func_00203B18");
extern void tie_load_244AE0(s32,void *,void *,s32) __asm__("func_00203F68");
extern void tie_finish_244AE0(void *,s32) __asm__("func_001E9768");
extern void shrub_load_244AE0(s32,void *,void *,void *,s32) __asm__("func_00204340");
extern void anim_load_244AE0(void *,s32) __asm__("func_00203038");
extern void light_load_244AE0(s32) __asm__("func_L00_0023FAB8");
extern void light_grid_244AE0(s32) __asm__("func_L00_0023FA70");
extern void poly_load_244AE0(void *,s32,void *,s32) __asm__("func_00202F00");
extern s32 core_finish_244AE0(s32,CoreHeader_244AE0 *) __asm__("func_L00_00240398");
extern s32 entities_244AE0(s32) __asm__("func_L00_002422D8");
extern float int_float_244AE0(s32) __asm__("func_001FA888");
extern f64 float_int_244AE0(float) __asm__("func_00120778");
extern void print_244AE0(const char *,...) __asm__("func_001E9730");
extern void clear_244AE0(void *,s32,s32) __asm__("func_001F99B0");
extern void game_load_244AE0(void) __asm__("func_00228268");
extern void checkpoint_load_244AE0(void) __asm__("func_L00_002862E0");
extern void update_init_244AE0(UpdateState_244AE0 *) __asm__("func_L00_002657B8");
extern s32 G_0015EE5C __asm__("D_0015EE5C") MACRO_ADDR;
extern s32 G_0015EE84 __asm__("D_0015EE84") MACRO_ADDR;
extern s32 G_0015EF8C __asm__("D_0015EF8C") MACRO_ADDR;
extern s32 G_0015F50C __asm__("D_L00_0015F50C") MACRO_ADDR;
extern s32 G_001600C4 __asm__("D_L00_001600C4") MACRO_ADDR;
extern s32 G_L00_0015F50C __asm__("D_L00_0015F50C") MACRO_ADDR;
extern s32 G_L00_0015F520 __asm__("D_L00_0015F520") MACRO_ADDR;
extern s32 G_L00_0015F538 __asm__("D_L00_0015F538") MACRO_ADDR;
extern s32 G_L00_0015F53C __asm__("D_L00_0015F53C") MACRO_ADDR;
extern s32 G_L00_0015F540 __asm__("D_L00_0015F540") MACRO_ADDR;
extern s32 G_L00_0015F6B0 __asm__("D_L00_0015F6B0") MACRO_ADDR;
extern s32 G_L00_0015F6D0 __asm__("D_L00_0015F6D0") MACRO_ADDR;
extern s32 G_L00_0015F6E0 __asm__("D_L00_0015F6E0") MACRO_ADDR;
extern s32 G_L00_0015F6E4 __asm__("D_L00_0015F6E4") MACRO_ADDR;
extern s32 G_L00_0015FD58 __asm__("D_L00_0015FD58") MACRO_ADDR;
extern s32 G_L00_0015FD5C __asm__("D_L00_0015FD5C") MACRO_ADDR;
extern s32 G_L00_0015FD60 __asm__("D_L00_0015FD60") MACRO_ADDR;
extern s32 G_L00_0015FD64 __asm__("D_L00_0015FD64") MACRO_ADDR;
extern s32 G_L00_0015FD68 __asm__("D_L00_0015FD68") MACRO_ADDR;
extern s32 G_L00_00160080 __asm__("D_L00_00160080") MACRO_ADDR;
extern s32 G_L00_00160088 __asm__("D_L00_00160088") MACRO_ADDR;
extern s32 G_L00_00160098 __asm__("D_L00_00160098") MACRO_ADDR;
extern s32 G_L00_001600A8 __asm__("D_L00_001600A8") MACRO_ADDR;
extern s32 G_L00_001600C8 __asm__("D_L00_001600C8") MACRO_ADDR;
extern s32 G_L00_001600CC __asm__("D_L00_001600CC") MACRO_ADDR;
extern s32 G_L00_001600D0 __asm__("D_L00_001600D0") MACRO_ADDR;
extern s32 G_L00_001601C8 __asm__("D_L00_001601C8") MACRO_ADDR;
extern s32 G_L00_0016022C __asm__("D_L00_0016022C") MACRO_ADDR;
extern s32 G_L00_00160554 __asm__("D_L00_00160554") MACRO_ADDR;
extern s32 G_L00_0016056C __asm__("D_L00_0016056C") MACRO_ADDR;
extern s32 G_L00_001605DC __asm__("D_L00_001605DC") MACRO_ADDR;
extern s32 G_L00_00161014 __asm__("D_L00_00161014") MACRO_ADDR;
extern s32 G_L00_00161080 __asm__("D_L00_00161080") MACRO_ADDR;
extern s32 G_L00_00161094 __asm__("D_L00_00161094") MACRO_ADDR;
extern s32 shrub_bases_244AE0[] __asm__("D_L00_001BC3C0");
extern const char S_001E8B00[] __asm__("D_L00_001E8B00");
extern const char S_001E8B20[] __asm__("D_L00_001E8B20");
extern const char S_001E8B50[] __asm__("D_L00_001E8B50");
extern const char S_001E8B68[] __asm__("D_L00_001E8B68");
extern const char S_001E8B80[] __asm__("D_L00_001E8B80");
extern const char S_001E8B98[] __asm__("D_L00_001E8B98");
extern const char S_001E8BB0[] __asm__("D_L00_001E8BB0");
extern const char S_001E8BC8[] __asm__("D_L00_001E8BC8");
extern const char S_001E8BE0[] __asm__("D_L00_001E8BE0");
extern const char S_001E8BF8[] __asm__("D_L00_001E8BF8");
extern const char S_001E8C10[] __asm__("D_L00_001E8C10");
extern const char S_001E8C30[] __asm__("D_L00_001E8C30");
extern const char S_001E8C48[] __asm__("D_L00_001E8C48");
extern const char S_001E8C60[] __asm__("D_L00_001E8C60");
extern const char S_001E8C78[] __asm__("D_L00_001E8C78");
extern const char S_001E8C90[] __asm__("D_L00_001E8C90");
extern const char S_001E8CA8[] __asm__("D_L00_001E8CA8");
extern const char S_001E8CC0[] __asm__("D_L00_001E8CC0");
extern const char S_001E8CD8[] __asm__("D_L00_001E8CD8");
extern const char S_001E8CF0[] __asm__("D_L00_001E8CF0");
extern const char S_001E8D08[] __asm__("D_L00_001E8D08");
extern const char S_001E8D20[] __asm__("D_L00_001E8D20");
extern const char S_001E8D38[] __asm__("D_L00_001E8D38");
extern const char S_001E8D50[] __asm__("D_L00_001E8D50");
extern const char S_001E8D68[] __asm__("D_L00_001E8D68");
extern const char S_001E8D80[] __asm__("D_L00_001E8D80");
extern const char S_001E8D98[] __asm__("D_L00_001E8D98");
extern const char S_001E8DB0[] __asm__("D_L00_001E8DB0");
extern const char S_001E8DC8[] __asm__("D_L00_001E8DC8");

/* Loads and relocates level-core sections, then reports memory use. */
s32 func_L00_00244AE0(s32 load_core, s32 checkpoint) {
    CoreHeader_244AE0 *h = mem_244AE0.f4;
    PackedHeader_244AE0 *packed = packed_244AE0;
    s32 base;
    s32 decoded, next, count, i, j, mode;
    ClassOffset_244AE0 *entry, *moby_offsets, *offset_list;
    char *record;
    char *vector_record;
    s32 *binding_record;
    s16 *map_header, *maps, *map_source, *class_map;
    s32 packet_bytes, packet_base, next_free;
    TexturePacket_244AE0 *packet;
    ClassHeader_244AE0 **class_ptr;
    s32 class_slot;
    s32 var_v0,var_v0_2;
    u32 code_end;
    LevelMemory_244AE0 *memory;
    s32 i1,i2,i3,i4,i5,i6,i7,i8,i9,i10;
    s64 tex_low1,tex_high1,tex_low2,tex_high2,tex_low3,tex_high3;
    s32 gs_base, resource_base;
    if (load_core) reset_core_244AE0();
    base = mem_244AE0.f14;
    if (load_core) {
        i=0;
        copy_244AE0(h,(char *)packed + packed->f10,packed->f14);
        G_0015F50C=0;
        copy_short_244AE0(mission_flags_244AE0,(char *)placements_244AE0 + G_0015EE84 * 16,16);
        relocate_chunks_244AE0((s32)packed + packed->f18,h->f0,(char *)h + h->f4);
        G_001600C4=G_0015EE5C;
        set_intr_244AE0(0);
        decoded=decode_244AE0((char *)packed+*(s32 *)((char *)packed+0x50),base);
        gs_base=G_0015EF8C;
        resource_base=base+h->f60;
                tex_high1=(s32)(gs_base+h->f94)>>8;
        tex_low1=(s32)(gs_base+h->f90)>>8;
        tex_high1<<=37;
        tex_high1|=(s64)0xB800<<19;
        tex_low1|=0x1D308000;
        setup1_244AE0.value[0]=tex_low1|tex_high1|((u64)1<<63);
        setup1_244AE0.value[1]=0x0000FFA0000000E0ULL;
        setup1_244AE0.value[2]=0x40000400004000ULL;
        tex_high2=(s32)(gs_base+h->f9C)>>8;
        tex_low2=(s32)(gs_base+h->f98)>>8;
        tex_high2<<=37;
        tex_high2|=(s64)0xB000<<19;
        tex_low2|=0x19304000;
        tex_high2|=tex_low2;
        tex_high2|=((u64)1<<63);
        setup2_244AE0.value[0]=tex_high2;
        setup2_244AE0.value[1]=0x0000FFA0000000E0ULL;
        setup2_244AE0.value[2]=0x40000400004000ULL;
        
        tex_high3=(s32)(gs_base+h->fC4)>>8;
        tex_low3=(s32)(gs_base+h->fC0)>>8;
        tex_high3<<=37;
        tex_high3|=(s64)0xB800<<19;
        tex_low3|=0x1D308000;
        G_L00_0015F50C=resource_base;
        tex_high3|=tex_low3;
        tex_high3|=((u64)1<<63);
        setup3_244AE0.value[0]=tex_high3;
        setup3_244AE0.value[1]=0x0000FFA0000000E0ULL;
        setup3_244AE0.value[2]=0x40000400004000ULL;
        offset_list=(ClassOffset_244AE0 *)((char *)h+h->f34);
        G_L00_00161014=h->f30;
        if(G_L00_00161014>0) { entry=offset_list; do { mode=texture_mode_244AE0(entry->mode); tfrag_classes_244AE0[i]=G_L00_0015F50C+entry->offset+(mode<<28); i++; entry++; } while(i<G_L00_00161014); }
        next=base+decoded;
        moby_offsets=(ClassOffset_244AE0 *)((char *)h+h->f3C);
        count_small_244AE0=h->f38;
        offset_list=moby_offsets;
        i1=0; if(G_L00_00160088>0) { entry=moby_offsets; do { mode=texture_mode_244AE0(entry->mode); moby_classes_244AE0[i1]=G_L00_0015F50C+entry->offset+(mode<<28); entry++; i1++; } while(i1<G_L00_00160088); }
        copy_244AE0(moby_offsets_244AE0,moby_offsets,G_L00_00160088*16);
        offset_list=(ClassOffset_244AE0 *)((char *)h+h->f44);
        G_L00_00161094=h->f40;
        i2=0; if(G_L00_00161094>0) { entry=offset_list; do { mode=texture_mode_244AE0(entry->mode); tie_classes_244AE0[i2]=G_L00_0015F50C+entry->offset+(mode<<28); i2++; entry++; } while(i2<G_L00_00161094); }
        offset_list=(ClassOffset_244AE0 *)((char *)h+h->f4C);
        G_L00_0016056C=h->f48;
        i3=0; if(G_L00_0016056C>0) { entry=offset_list; do { mode=texture_mode_244AE0(entry->mode); shrub_classes_244AE0[i3]=G_L00_0015F50C+entry->offset+(mode<<28); i3++; entry++; } while(i3<G_L00_0016056C); }
        tfrag_load_244AE0(base+h->f8,(char *)h+h->f34);
        if(h->fC) f6e0_small_244AE0=base+h->fC; else G_L00_0015F6E0=0;
        if(h->f10) sky_load_244AE0(base+h->f10); else G_L00_001605DC=0;
        collision_load_244AE0((char *)(base+h->f14));
        record=(char *)h+h->f1C;
        for(i4=0;i4<h->f18;i4++,record+=32) { s32 offset=*(s32 *)record; moby_load_244AE0(offset?base+offset:0,(char *)h+h->f3C,record+16,*(s32 *)(record+4)); }
        moby_fix_244AE0((char *)h+h->f1C,h->f1C);
        binding_record=(s32 *)((char *)h+h->f78);
        for(i5=0;i5<((ClassHeader_244AE0 *)root_address_244AE0)->class_count;i5++,binding_record++) { if(*binding_record) { *(s32 *)((char *)((ClassHeader_244AE0 *)root_address_244AE0)+0x48+i5*4)=base+*binding_record; class_fix_244AE0((ClassHeader_244AE0 *)root_address_244AE0,i5); } }
        vector_record=(char *)h+h->f84;
        G_L00_001600CC=-1; G_L00_001600D0=0; G_L00_001600C8=h->f80;
        for(i6=0;i6<h->f80;i6++,vector_record+=16) { class_ids_244AE0[i6]=*(s32 *)(vector_record+4); class_bases_244AE0[i6]=base+*(s32 *)vector_record; class_flags_244AE0[i6]=*(s32 *)(vector_record+8); qcopy(&class_vectors_244AE0[i6],(char *)h+h->f1C+((texture_ids_244AE0[*(s32 *)(vector_record+4)]<<5)+16)); }
        record=(char *)h+h->f24;
        for(i7=0;i7<h->f20;i7++,record+=32) tie_load_244AE0(base+*(s32 *)record,(char *)h+h->f44,record+16,*(s32 *)(record+4));
        tie_finish_244AE0((char *)h+h->f24,h->f20);
        record=(char *)h+h->f2C;
        for(i8=0;i8<h->f28;i8++,record+=48) shrub_load_244AE0(base+*(s32 *)record,(char *)h+h->f4C,record+16,record+32,*(s32 *)(record+4));
        G_L00_0015F520=base+h->f68;
        anim_load_244AE0((char *)h+h->f5C,h->f58);
        if(h->fA0) light_load_244AE0(base+h->fA0); else { G_L00_0015F538=0;G_L00_0015F53C=0;G_L00_0015F540=0; }
        if(h->fA4) light_grid_244AE0(base+h->fA4); else {G_L00_0015FD58=0;G_L00_0015FD5C=0;G_L00_0015FD64=0;G_L00_0015FD60=0;G_L00_0015FD68=0;}
        if(h->fA8) f6e4_small_244AE0=base+h->fA8; else G_L00_0015F6E4=0;
        i9=0;
        poly_load_244AE0((char *)h+h->f6C,base+h->f64,(char *)h+h->f54,h->f50);
        maps=(s16 *)((char *)h+h->f70);
        packet_base=(next+63)&0xFFFFFFC0U;
        count=maps[1];
        map_header=maps;
        packets_244AE0=(TexturePacket_244AE0 *)packet_base;
        G_L00_0015F6D0=count;
        packet_bytes=count*32;
        copy_short_244AE0((void *)packet_base,(char *)maps+maps[0],packet_bytes);
        maps+=2;
        packet_base+=packet_bytes;
        map_source=(s16 *)((char *)map_header+maps[0]);
        for(;i9<G_L00_0015F6D0;i9++) {
            packet=packets_244AE0+i9;
            if(packet->texture>=maps[1]) {print_244AE0(S_001E8B00); packets_244AE0[i9].texture=-1;}
            else packet->texture=*(u16 *)((char *)map_source+packet->texture*4);
            packets_244AE0[i9].stamp=G_0015EE5C;
        }
        next_free=packet_base+63;
        maps+=2;
        for(i10=0;i10<G_L00_00160080;i10++,maps+=2) {
            class_ptr=&classes_244AE0[i10];
            if(!*class_ptr) {
                for(class_slot=0;class_slot<G_L00_001600C8;class_slot++) if(class_ids_244AE0[class_slot]==type_ids_244AE0[i10]) break;
                if(class_slot<G_L00_001600C8) {
                    clear_244AE0(packet_lists_244AE0[class_slot],-1,32);
                    class_map=(s16 *)((char *)map_header+maps[0]);
                    for(j=0;j<maps[1]&&j<15;j++,class_map+=2) packet_lists_244AE0[class_slot][j]=*(u16 *)class_map;
                }
            } else {
                if(maps[1]!=(*class_ptr)->texture_count) print_244AE0(S_001E8B20,type_ids_244AE0[i10]);
                class_map=(s16 *)((char *)map_header+maps[0]);
                for(j=0;j<(*class_ptr)->texture_count;j++,class_map+=2) {(*class_ptr)->packets[j].texture=*(u16 *)class_map; (*class_ptr)->packets[j].stamp=G_0015EE5C;}
            }
        }
        mem_244AE0.f18=core_finish_244AE0(next_free&0xFFFFFFC0U,h);
    }
    memory=&mem_244AE0;
    memory->f1C=entities_244AE0(load_core);
    if (load_core != 0) {
        print_244AE0(S_001E8B50);
        var_v0=0x100000;
        print_244AE0(S_001E8B68, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        code_end=(u32)module_end_244AE0;
        var_v0=code_end+0xFFF00000U;
        print_244AE0(S_001E8B80, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=mem_244AE0.f0 - code_end;
        print_244AE0(S_001E8B98, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=memory->fC - mem_244AE0.f0;
        print_244AE0(S_001E8BB0, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=memory->f10 - memory->fC;
        print_244AE0(S_001E8BC8, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=memory->f14 - memory->f10;
        print_244AE0(S_001E8BE0, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=G_L00_0015F6E0;
        if(var_v0!=0 || (var_v0=G_L00_001605DC)!=0) {
            print_244AE0(S_001E8BF8,float_int_244AE0(int_float_244AE0(var_v0-memory->f14)*0.0009765625f));
            var_v0_2=f6e0_small_244AE0;
        } else {
            print_244AE0(S_001E8C10);
            var_v0_2=G_L00_0015F6E0;
        }
        if (var_v0_2 != 0) {
            if (G_L00_001605DC != 0) {
                var_v0=G_L00_001605DC - var_v0_2;
        print_244AE0(S_001E8C30, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
            } else {
                var_v0=collision_244AE0[0] - var_v0_2;
        print_244AE0(S_001E8C30, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
            }
        }
        if (h->f10 != 0) {
            var_v0=collision_244AE0[0] - G_L00_001605DC;
        print_244AE0(S_001E8C48, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        }
        var_v0=G_L00_0015F50C - collision_244AE0[0];
        print_244AE0(S_001E8C60, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=((s32) data_base_244AE0[1] >> 4) - G_L00_0015F50C;
        print_244AE0(S_001E8C78, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=G_L00_0015F520 - ((s32) data_base_244AE0[1] >> 4);
        print_244AE0(S_001E8C90, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=(s32)((ClassHeader_244AE0 *)root_address_244AE0) - G_L00_0015F520;
        print_244AE0(S_001E8CA8, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=tie_bases_244AE0[0] - (s32)((ClassHeader_244AE0 *)root_address_244AE0);
        print_244AE0(S_001E8CC0, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        if (h->f28 != 0) {
            var_v0=shrub_bases_244AE0[0] - tie_bases_244AE0[0];
        print_244AE0(S_001E8CD8, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        } else {
            var_v0=G_L00_00161080 - tie_bases_244AE0[0];
        print_244AE0(S_001E8CD8, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        }
        if (h->f28 != 0) {
            var_v0=((ClassHeader_244AE0 *)root_address_244AE0)->bindings[0] - shrub_bases_244AE0[0];
        print_244AE0(S_001E8CF0, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        }
        var_v0=G_L00_00161080 - ((ClassHeader_244AE0 *)root_address_244AE0)->bindings[0];
        print_244AE0(S_001E8D08, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=G_L00_00160554 - G_L00_00161080;
        print_244AE0(S_001E8D20, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=G_L00_00160098 - G_L00_00160554;
        print_244AE0(S_001E8D38, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=G_L00_001600A8 - G_L00_00160098;
        print_244AE0(S_001E8D50, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=G_L00_001601C8 - G_L00_001600A8;
        print_244AE0(S_001E8D68, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        var_v0=G_L00_0016022C - G_L00_001601C8;
        print_244AE0(S_001E8D80, float_int_244AE0(int_float_244AE0(var_v0) * 0.0009765625f));
        print_244AE0(S_001E8D98, 128.0);
        print_244AE0(newline_244AE0);
        var_v0=G_L00_0016022C + 0x20000;
        print_244AE0(S_001E8DB0, float_int_244AE0(int_float_244AE0(var_v0) * 0.0000009536743f));
        print_244AE0(newline_244AE0);
        var_v0=memory->f1C - (mem_244AE0.f0 + (s32)0xFFD30000);
        print_244AE0(S_001E8DC8, float_int_244AE0(int_float_244AE0(var_v0) * 0.0000009536743f));
        print_244AE0(newline_244AE0);
    }

    if(checkpoint) checkpoint_load_244AE0(); else {game_load_244AE0();{s32 reset_i; for(reset_i=15;reset_i>=0;reset_i--) reset_ids_244AE0[reset_i]=0;}}
    update_244AE0.f10=0;*(s32 *)&update_244AE0.fC=0;*(s32 *)&update_244AE0.f8=0;
    update_init_244AE0(&update_244AE0);
    G_L00_0015F6B0++;
    return 0;
}
