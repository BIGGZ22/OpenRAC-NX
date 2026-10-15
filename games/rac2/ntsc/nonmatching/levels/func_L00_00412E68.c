/* G8 candidate: 384 bytes, retail 400. TI zero emits sq zero instead of por followed by sq; local TI copy folds stack offsets, saving two callee-saved pointers. No C match claimed. */
/* State-triggered box effect. TI zero initialization still needs validation. */
typedef int EffectQuad10 __attribute__((mode(TI)));
extern void func_002A7750(void *,int,int),func_0029F338(void *);
extern char *func_002A9640(void *,int,int);
extern float func_002A7878(float,float);
extern void func_00282D30(void *,void *,float),func_00282CF0(void *,void *,void *),func_00282CC0(void *,void *,void *);
extern int D_L00_001B1370;
extern char D_L00_001B1368[8],D_L00_001B1360[8];
extern void func_L00_0032A258(void *,void *,void *,void *,int,int,int,int,int,int,int);
void func_L00_00412E68(unsigned char *object) {
    float minimum[4],maximum[4],size[4];
    int *state=*(int **)(object+0x68);
    char *hit;
    switch(object[0x20]) {
    case 0: func_002A7750(object,1,-1);break;
    case 1:
        hit=func_002A9640(object,0x04010000,0);
        if(hit && !state[0]) {
            *(EffectQuad10 *)size=0;
            size[0]=3.5f;size[1]=3.5f;
            *(float *)(hit+0x18)=1.0f;
            func_00282D30(minimum,hit+0x10,func_002A7878(4.0f,8.0f));
            *(EffectQuad10 *)maximum=*(EffectQuad10 *)minimum;
            func_00282CF0(minimum,minimum,size);
            func_00282CC0(maximum,maximum,size);
            func_L00_0032A258(object,minimum,D_L00_001B1368,D_L00_001B1360,65535,0,D_L00_001B1370,8,65535,255,0);
            func_002A7750(object,2,-1);
        }
        object[0xa8]=255;break;
    case 2: func_0029F338(object);break;
    }
}
