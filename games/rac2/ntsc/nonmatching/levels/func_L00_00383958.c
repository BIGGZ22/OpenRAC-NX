/* Medium-tier attempt: 528 bytes versus568; dead TI local copies disappear, frame shrinks and registers differ. */
typedef unsigned int VecBits __attribute__((mode(TI)));
typedef union { VecBits bits; float f[4]; } LocalVec;
extern int D_L00_001A8F00;
extern char *D_L00_00220DA0[];
extern void func_00282CC0(void *,void *,void *);
extern void func_00283098(void *,void *,void *);
extern void func_0029FC20(void *, int, void *);
extern void func_0029FCC0(void *, int, void *);
extern void func_002A10E8(void *);
extern void func_002AB6D8(void *, void *);

void func_L00_00383958(char *obj) {
    VecBits position, rotation;
    float matrix[4][4] __attribute__((aligned(16)));
    VecBits transformed, offset;
    VecBits *save=&rotation;
    int i=0,j;
    float *row;
    if (D_L00_00220DA0[0]) {
        do {
            char **entry=&D_L00_00220DA0[i];
            *(unsigned short *)(*entry+0x34)&=0xFFBE;
            *(unsigned short *)(*entry+0x34)|=0x1000;
            *(int *)(*entry+0x98)=*(int *)(*(char **)(*entry+0x24)+0x10);
            position=*(VecBits *)(*entry+0x10);
            *save=*(VecBits *)(*entry+0xF0);
            *(VecBits *)(*entry+0x10)=*(VecBits *)(obj+0x10);
            func_0029FC20(obj,0,matrix);
            i++;
            row=&matrix[0][0];
            for(j=0;j<3;j++) {
                *(float *)(*entry+j*16+0xC0)=row[0];
                *(float *)(*entry+j*16+0xC4)=row[1];
                *(float *)(*entry+j*16+0xC8)=row[2];
                row+=4;
            }
            if(D_L00_001A8F00==2) {
                offset=0;
                ((float *)&offset)[0]=-4.1919f;
                ((float *)&offset)[2]=-1.355f;
                func_0029FCC0(obj,1,&transformed);
                func_00283098(&offset,&offset,*entry+0xC0);
                func_00282CC0(&transformed,&transformed,&offset);
                *(VecBits *)(*entry+0x10)=transformed;
            }
            func_002AB6D8(*entry+0xC0,*entry+0xF0);
            func_002A10E8(*entry);
            if(D_L00_001A8F00==0) {
                if(*(int *)(obj+0x98)) *(int *)(*entry+0x98)=*(int *)(*(char **)(*entry+0x24)+0x10);
            } else *(int *)(*entry+0x98)=0;
        } while(D_L00_00220DA0[i]);
    }
}
