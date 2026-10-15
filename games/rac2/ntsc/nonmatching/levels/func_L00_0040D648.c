/* 1460 bytes, retail 1472. Five source variants: nibble-path tail merging, object/state register allocation and phase scheduling remain. Typed object/state fields did not close the difference. */
/* Aranos controller: initialize linked objects and drive the two scripted sequences. */
extern unsigned char D_L00_0021DDB0[];
extern unsigned char D_0019B4A8[];
extern int D_001A79F0 __attribute__((sda));
extern int D_L00_001A8F00 __attribute__((sda));
extern unsigned char *D_L00_001B1E9C __attribute__((sda));
extern char D_L00_001A8F20[], D_L00_001B1010[];
typedef struct { char pad[0x30]; int phase, frame; char pad2[0x15c]; int first, second; } Controller10;
extern Controller10 D_L00_001BCAC0;
extern void func_0029F338(void *);
extern void func_00325B70(int,void *,int,int);
extern void func_00325C50(float,int,void *,int,void *);
extern float func_00283B98(float,float);
extern void func_002E5CF8(int,int,void *);
extern void func_0026FEB8(void *);
void func_L00_0040D648(unsigned char *object) {
    int *state = *(int **)(object+0x68);
    unsigned char *other, *counter;
    int i, *index;
    switch(object[0x20]) {
    case 0:
        state[9]=0; state[10]=0; state[12]=0; state[13]=0;
        object[0x30]=255; object[0x20]=1;
        if (!D_L00_0021DDB0[0x464+(short)*(unsigned short *)(object+0xb2)]) {
            int n=D_0019B4A8[((short)*(unsigned short *)(object+0xb2)>>1)+(D_001A79F0<<10)];
            switch (*(unsigned short *)(object+0xb2)&1) {
            case 0: if (!(n&7)) return; break;
            default: if (!((n>>4)&7)) return; break;
            }
        }
        if(state[17]!=-1) func_0029F338(D_L00_001B1E9C+(state[17]<<8));
        if(state[16]!=-1) func_0029F338(D_L00_001B1E9C+(state[16]<<8));
        break;
    case 1:
        if(D_L00_001A8F00!=2 && state[8]!=-1) {
            other=D_L00_001B1E9C+(state[8]<<8);
            *(unsigned short *)(other+0x34)=(*(unsigned short *)(other+0x34)&0xffbe)|0x1000;
            *(int *)(other+0x98)=*(int *)(*(char **)(other+0x24)+0x10);
        }
        if(D_L00_001BCAC0.phase==1 && D_L00_001A8F00==2) {
            if(D_L00_001BCAC0.frame==0) {
                other=D_L00_001B1E9C+(state[8]<<8);
                *(int *)(other+0x98)=0;
                *(unsigned short *)(other+0x34)=(*(unsigned short *)(other+0x34)|0x41)&0xefff;
                if(state[17]!=-1) func_0029F338(D_L00_001B1E9C+(state[17]<<8));
                if(state[16]!=-1) func_0029F338(D_L00_001B1E9C+(state[16]<<8));
                if(*(unsigned short *)(object+0xb2)&1) {
                    counter=D_0019B4A8+((short)*(unsigned short *)(object+0xb2)>>1)+(D_001A79F0<<10);
                    if(((*counter>>4)&7)!=7) *counter+=16;
                } else {
                    counter=D_0019B4A8+((short)*(unsigned short *)(object+0xb2)>>1)+(D_001A79F0<<10);
                    if((*counter&7)!=7) ++*counter;
                }
                func_00325B70(D_L00_001BCAC0.first,state+9,0,2);
            } else if(D_L00_001BCAC0.frame>=1976) {
                int first=D_L00_001BCAC0.first;
                *(float *)(state+11)=func_00283B98(*(float *)(state+11),0.436332345f);
                func_00325C50(*(float *)(state+11),first,state+9,1,D_L00_001A8F20);
            } else func_00325C50(*(float *)(state+11),D_L00_001BCAC0.first,state+9,0,D_L00_001A8F20);
            if(D_L00_001BCAC0.frame==223) {
                index=state; i=1;
                do {
                    other=D_L00_001B1E9C+(*index<<8);
                    if(other && other[0x20]!=254 && other[0x20]!=253) {
                        func_002E5CF8(0,0,other); other[0x20]=1;
                    }
                    --i; ++index;
                } while(i>=0);
            }
            if(D_L00_001BCAC0.frame==226) func_0026FEB8(D_L00_001B1010);
        }
        if(D_L00_001A8F00!=2) {
            if(state[9]) {func_0029F338((void *)state[9]);func_0029F338((void *)state[10]);state[9]=0;state[10]=0;}
            if(state[12]) {func_0029F338((void *)state[12]);func_0029F338((void *)state[13]);state[12]=0;state[13]=0;}
        }
        if(D_L00_001BCAC0.phase==2 && D_L00_001A8F00==2) {
            if(D_L00_001BCAC0.frame==0) {
                other=D_L00_001B1E9C+(state[4]<<8);
                *(int *)(other+0x98)=0;
                *(unsigned short *)(other+0x34)=(*(unsigned short *)(other+0x34)|0x41)&0xefff;
                index=state+5; i=2;
                do {
                    other=D_L00_001B1E9C+(*index++<<8); --i;
                    *(int *)(other+0x98)=0;
                    *(unsigned short *)(other+0x34)=(*(unsigned short *)(other+0x34)|0x41)&0xefff;
                } while(i>=0);
                func_00325B70(D_L00_001BCAC0.first,state+9,0,2);
                func_00325B70(D_L00_001BCAC0.second,state+12,0,2);
            } else if(D_L00_001BCAC0.frame<780) {
                int second=D_L00_001BCAC0.second,first=D_L00_001BCAC0.first;
                *(float *)(state+11)=func_00283B98(*(float *)(state+11),0.261799425f);
                *(float *)(state+14)=func_00283B98(*(float *)(state+14),0.261799425f);
                func_00325C50(*(float *)(state+11),first,state+9,1,D_L00_001A8F20);
                func_00325C50(*(float *)(state+14),second,state+12,1,D_L00_001A8F20);
            } else {
                int second=D_L00_001BCAC0.second,first=D_L00_001BCAC0.first;
                *(float *)(state+11)=func_00283B98(*(float *)(state+11),0.087266475f);
                *(float *)(state+14)=func_00283B98(*(float *)(state+14),0.087266475f);
                func_00325C50(*(float *)(state+11),first,state+9,0,D_L00_001A8F20);
                func_00325C50(*(float *)(state+14),second,state+12,0,D_L00_001A8F20);
            }
        }
        break;
    }
}
