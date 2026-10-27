/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../game/rac1/host/effect_quad.c"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); exit(1); } } while (0)
static unsigned char ram[0x400000];
uint8_t* openrac_guest_base=ram;
gaddr openrac_relocate_low=0x100000, openrac_relocate_high=0x200000;
gaddr openrac_relocate_data(gaddr a) { return a+0x100000; }
static openrac_game_quad captured;
static int submitted;
void openrac_game_effect_quad(const openrac_game_quad* q) { captured=*q; ++submitted; }
int main(void) {
    openrac_game_quad input={0};
    float matrix[16]={2,0,0,0, 0,3,0,0, 0,0,4,0, 7,11,13,1};
    for (int k=0;k<4;++k) {
        for(int j=0;j<4;++j) input.corner[k][j]=(float)(k*4+j+1);
        input.rgba[k]=0x81234560u+(unsigned)k;
        input.st[k][0]=(float)k/4; input.st[k][1]=(float)k/8;
    }
    input.tex0=0x8123456000000123ULL;
    input.clamp=0x1122334455667788ULL; input.tex1=0xFF9000000260ULL;
    input.alpha=0x8000000049ULL;
    memcpy(ram+0x1000,&input,0x90); memcpy(ram+0x2000,matrix,sizeof(matrix));
    GREF(int,0x25F558)=1;
    GREF(uint16_t,0x28D14E)=(uint16_t)(input.tex0&0x3FFF);
    GREF(uint16_t,0x28D146)=(uint16_t)((input.tex0>>37)&0x3FFF);
    GREF(uint32_t,0x28D140)=0x345000;
    GREF(uint32_t,0x28D148)=0x346000;
    func_L00_001FD1D8(0x1000,0,0);
    CHECK(submitted==1 && !memcmp(&input,&captured,0x90));
    CHECK(captured.pixels == 0x346000 && captured.clut == 0x345000 && captured.source == 1);
    /* The texture is cached after its upload frame. */
    GREF(int,0x25F558)=0;
    func_L00_001FD1D8(0x1000,0x2000,0);
    CHECK(
        submitted == 2 && captured.pixels == 0x346000 && captured.clut == 0x345000
        && captured.source == 2
    );
    for(int k=0;k<4;++k) {
        CHECK(captured.corner[k][0]==2*input.corner[k][0]+7*input.corner[k][3]);
        CHECK(captured.corner[k][1]==3*input.corner[k][1]+11*input.corner[k][3]);
        CHECK(captured.corner[k][2]==4*input.corner[k][2]+13*input.corner[k][3]);
        CHECK(captured.corner[k][3]==input.corner[k][3]);
    }
    CHECK(!memcmp(captured.rgba,input.rgba,0x50));
    CHECK(!memcmp(ram+0x1000,&input,0x90));
    func_001F7EF8(0x1000,0,0);
    CHECK(submitted==3 && !memcmp(&input,&captured,0x90));
    input.tex0=0x5678; memcpy(ram+0x1000,&input,0x90);
    func_L00_001FD1D8(0x1000,0,0);
    CHECK(submitted == 4 && captured.pixels == 0 && captured.clut == 0 && captured.source == 0);
    puts("effect quad: OK"); return 0;
}
