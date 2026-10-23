/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../game/rac1-pal/hand/game_moby_grid.c"
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"%d: %s\n",__LINE__,#c); exit(1); } } while (0)
#define TRAPS(call) do { expect_trap=1; if (!setjmp(trap_env)) { call; CHECK(0); } expect_trap=0; } while (0)
_Alignas(16) unsigned char exe_grid[0x10000];
unsigned int exe_grid_used[48];
static unsigned char objects[80][0x100];
static jmp_buf trap_env;
static int expect_trap;
void exe_grid_trap(void) { CHECK(expect_trap); longjmp(trap_env,1); }
static unsigned int rect(int x0,int y0,int x1,int y1) {
    return (unsigned)x0 | (unsigned)y0<<8 | (unsigned)x1<<16 | (unsigned)y1<<24;
}
static unsigned char* cell(int x,int y) { return exe_grid+(y*64+x)*4; }
static unsigned short* list(int x,int y) {
    return (unsigned short*)(exe_grid+0x4000+*(unsigned short*)cell(x,y)*32);
}
static void reset(void) {
    memset(exe_grid,0,sizeof(exe_grid)); memset(exe_grid_used,0,sizeof(exe_grid_used));
    memset(objects,0,sizeof(objects));
    for(int i=0;i<80;++i) {
        *(unsigned int*)(objects[i]+0xA0)=0x80807F7F;
        *(unsigned int*)(objects[i]+0xAC)=(unsigned)(100+i);
    }
}
/* An independent membership oracle for a bounded sequence of moves. */
static unsigned int random_state=17;
static unsigned int next_random(void) { random_state=random_state*1664525u+1013904223u; return random_state; }
static void check_membership(void) {
    unsigned char occupied[1536]={0};
    for(int y=0;y<8;++y) for(int x=0;x<8;++x) {
        unsigned char* c=cell(x,y); int count=0;
        for(int n=0;n<80;++n) {
            unsigned int r=*(unsigned int*)(objects[n]+0xA0);
            int inside=!(r&0x80000000) && x>=(int)(r&255) && y>=(int)((r>>8)&255)
                && x<=(int)((r>>16)&255) && y<=(int)(r>>24);
            int found=0;
            for(int k=0;k<c[2];++k) found+=list(x,y)[k]==100+n;
            CHECK(found==inside); count+=inside;
        }
        CHECK(c[2]==count && c[2]<=c[3]*16);
        if(c[3]) {
            unsigned int first=*(unsigned short*)c;
            CHECK(first+c[3]<=1536 && first%c[3]==0);
            for(unsigned int k=first;k<first+c[3];++k) { CHECK(!occupied[k]); occupied[k]=1; }
        }
    }
    for(int k=0;k<1536;++k) CHECK(!!(exe_grid_used[k/32]&(1u<<(k%32)))==occupied[k]);
}
int main(void) {
    unsigned char saved[0x10000];
    reset();
    CHECK(func_0020E9F0(16)==0); CHECK(func_0020E9F0(16)==16);
    CHECK(func_0020E9F0(8)==32); func_0020E990(16,16);
    CHECK(func_0020E9F0(4)==16); CHECK(func_0020E9F0(8)==24);
    CHECK(func_0020E9F0(4)==20);
    CHECK(exe_grid_used[0]==0xFFFFFFFFu && exe_grid_used[1]==0xFF);
    func_0020E990(30,4); CHECK(exe_grid_used[0]==0x3FFFFFFFu && exe_grid_used[1]==0xFC);
    TRAPS(func_0020E990(30,1)); TRAPS(func_0020E9F0(3));
    TRAPS(func_0020E990(-1,1)); TRAPS(func_0020E990(1535,2));
    for(int i=0;i<48;++i) exe_grid_used[i]=0xFFFFFFFFu;
    exe_grid_used[47]=0x7FFFFFFFu; CHECK(func_0020E9F0(1)==1535);
    TRAPS(func_0020E9F0(1));
    reset();
    for(int i=0;i<33;++i) func_0020EA70(objects[i],rect(0,0,0,0));
    CHECK(cell(0,0)[2]==33 && cell(0,0)[3]==4);
    for(int i=0;i<33;++i) CHECK(list(0,0)[i]==100+i);
    /* Membership overlap must not move entries or allocate again. */
    memcpy(saved,exe_grid,sizeof(saved)); func_0020EA70(objects[0],rect(0,0,0,0));
    CHECK(!memcmp(saved,exe_grid,sizeof(saved)));
    for(int i=0;i<5;++i) func_0020EA70(objects[i],0x80807F7F);
    CHECK(cell(0,0)[2]==28 && cell(0,0)[3]==4); /* strict shrink threshold */
    func_0020EA70(objects[5],~0u); CHECK(cell(0,0)[2]==27 && cell(0,0)[3]==2);
    CHECK(list(0,0)[5]==127); /* removed member replaced by last */
    for(int i=6;i<21;++i) func_0020EA70(objects[i],~0u);
    CHECK(cell(0,0)[2]==12 && cell(0,0)[3]==2);
    func_0020EA70(objects[21],~0u); CHECK(cell(0,0)[2]==11 && cell(0,0)[3]==1);
    for(int i=22;i<33;++i) func_0020EA70(objects[i],~0u);
    CHECK(cell(0,0)[2]==0 && cell(0,0)[3]==0 && *(unsigned short*)cell(0,0)==0);
    for(int i=0;i<48;++i) CHECK(!exe_grid_used[i]);
    reset(); func_0020EA70(objects[0],rect(63,63,63,63));
    CHECK(cell(63,63)[2]==1 && list(63,63)[0]==100);
    func_0020EA70(objects[0],~0u); CHECK(cell(63,63)[3]==0);
    /* Column 64 of a row is column 0 of the next, as retail's cell address makes it; column 64 of
       row 63 is past the table. */
    reset(); func_0020EA70(objects[0],rect(64,0,64,0)); CHECK(cell(0,1)[2]==1 && list(0,1)[0]==100);
    func_0020EA70(objects[0],~0u); CHECK(cell(0,1)[2]==0);
    TRAPS(func_0020EA70(objects[0],rect(0,63,64,63)));
    /* Removal compares the full 32-bit ID against zero-extended pool IDs. */
    reset(); func_0020EA70(objects[0],rect(0,0,0,0));
    *(unsigned int*)(objects[0]+0xAC)=0x10064;
    TRAPS(func_0020EA70(objects[0],~0u)); CHECK(cell(0,0)[2]==1);
    *(unsigned int*)(objects[0]+0xAC)=101;
    TRAPS(func_0020EA70(objects[0],~0u));
    reset();
    for(int step=0;step<500;++step) {
        int n=(int)(next_random()%80), x=(int)(next_random()%6), y=(int)(next_random()%6);
        unsigned int r=step%7?rect(x,y,x+2,y+2):0x80807F7F;
        func_0020EA70(objects[n],r); check_membership();
    }
    for(int n=0;n<80;++n) func_0020EA70(objects[n],~0u);
    check_membership();
    puts("executable moby grid: OK"); return 0;
}
