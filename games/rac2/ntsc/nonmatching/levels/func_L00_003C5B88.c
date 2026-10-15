/* Unmatched plain-C reconstruction. 1740 bytes versus 1804; -G0.
 * instruction at +0x4 differs. Full behavior and linked-image equality remain unverified.
 */
typedef unsigned int Quad __attribute__((mode(TI)));
typedef union { Quad q; float f[4]; } Vec;
extern char *D_0018C0B0;
extern float D_00189EA0[],D_0018A144;
extern short D_0018A168;
extern int D_L00_001A8FF0,D_L00_001A8FF4 __attribute__((sda));
extern float D_L00_001AFFA0 __attribute__((sda)),D_L00_001AFFA4 __attribute__((sda));
extern char *D_L00_001FACE0[],*D_L00_001BF218;
extern Vec D_L00_001BF220;
extern void func_00282D30(void *,void *,float),func_00282CC0(void *,void *,void *),func_00282CF0(void *,void *,void *);
extern int func_002761F8(void *,void *,int,void *,void *);
extern char *func_002AB1F0(void *);
extern float func_00283248(float,float),func_00283C80(float,float),func_00282C48(float),func_002831B0(float),func_00283CE0(int);
extern float func_00282E80(void *,void *),func_00282E48(void *,void *),func_00282DB0(void *,void *),func_00282DF0(void *);
extern void func_002AF000(void *,float,float,float);
char *func_L00_003C5B88(char *obj,Vec *origin,float *yaw,float *pitch) {
    struct { Vec origin,offset,player; char *disabled[128]; Vec target,aim,ray,axis; } frame;
    char *ctx=*(char **)(obj+0x68),*selected=0,*candidate,*definition,*hit;
    int n=0,index=0,tries,accepted,i;
    float best=D_L00_001AFFA4,heading,elevation,delta,distance,dot,length,angle,radius;
    frame.origin.q=origin->q;
    func_00282D30(&frame.offset,D_0018C0B0+0xE0,0.5f);
    func_00282CC0(&frame.player,D_0018C0B0+0x10,&frame.offset);
    if(func_002761F8(&frame.player,&frame.origin,6,D_0018C0B0,0)) return 0;
    while((candidate=D_L00_001FACE0[index])!=0) {
        if((signed char)candidate[0x20]==-2 || (signed char)candidate[0x20]==-3 || *(short *)(candidate+0xAA)==0xE7D || *(short *)(candidate+0xAA)==0xEB0) goto next;
        frame.target.q=*(Quad *)(candidate+0x10);
        definition=func_002AB1F0(candidate);
        if(!definition || definition[0xE]==3) goto next;
        func_00282D30(&frame.aim,candidate+0xE0,*(float *)(definition+0x10));
        func_00282CC0(&frame.target,&frame.target,&frame.aim);
        if(!candidate || !*(char **)(candidate+0x24) || *(short *)(*(char **)(candidate+0x24)+0x46)!=5) goto next;
        accepted=0;
        heading=func_00283248(frame.target.f[0]-frame.origin.f[0],frame.target.f[1]-frame.origin.f[1]);
        elevation=func_00283248(func_00282E80(&frame.origin,&frame.target),frame.target.f[2]-frame.origin.f[2]);
        delta=func_00283C80(*yaw,heading);
        distance=func_00282E48(&frame.origin,&frame.target);
        if(!D_L00_001A8FF0 && !D_0018A168 && !D_L00_001A8FF4) {
            if(distance<2.5f) {
                angle=func_00283248(*(float *)(candidate+0x10)-D_00189EA0[0],*(float *)(candidate+0x14)-D_00189EA0[1]);
                if(func_00283C80(*(float *)(D_0018C0B0+0xF8),angle)<1.0471977f && func_00282C48(frame.origin.f[2]-frame.target.f[2])<2.0f) accepted=1;
            }
        } else if(distance<2.5f) {
            func_00282CF0(&frame.ray,candidate+0x10,D_0018C0B0+0x10);
            dot=func_00282DB0(&frame.ray,D_0018C0B0+0xE0);
            if(func_00282C48(dot)<2.0f) {
                func_00282D30(&frame.axis,D_0018C0B0+0xE0,dot);
                func_00282CF0(&frame.ray,&frame.ray,&frame.axis);
                dot=func_00282DB0(&frame.ray,D_0018C0B0+0xC0);
                length=func_00282DF0(&frame.ray);
                if(length!=0.0f && 1.5707964f-func_002831B0(dot/length)<1.0471977f) accepted=1;
            }
        }
        if(accepted && !func_002761F8(&frame.origin,&frame.target,6,D_0018C0B0,0)) { selected=candidate; *pitch=-elevation; *yaw=heading; break; }
        if((distance>best && candidate!=*(char **)(ctx+0x10)) || distance>D_L00_001AFFA4) goto next;
        func_002AF000(&frame.aim,distance,*yaw,*pitch+D_0018A144*0.5f);
        func_00282CC0(&frame.aim,&frame.aim,&frame.origin);
        angle=func_002831B0(func_00282E48(&frame.aim,&frame.target)/(distance+distance)); angle+=angle;
        if(D_L00_001AFFA0*0.017453294f<angle && (definition=func_002AB1F0(candidate))!=0) {
            radius=func_00283CE0((unsigned char)definition[10]);
            length=func_002831B0((radius*0.125f)/distance);
            if(radius*0.125f<distance) { angle-=length; if(angle<0.0f) angle=0.0f; }
        }
        accepted=1;
        if(angle<D_L00_001AFFA0*0.017453294f) {
            frame.ray.q=frame.origin.q; tries=0;
            while(func_002761F8(&frame.ray,&frame.target,6,obj,0)) {
                if(++tries>6) break;
                hit=D_L00_001BF218;
                if(!hit) { accepted=0; break; }
                if(*(short *)(hit+0xAA)!=0xE7D && *(short *)(hit+0xAA)!=0xEB0) { if(hit!=candidate) accepted=0; break; }
                frame.disabled[n++]=hit; *(int *)(hit+0x98)=0;
                if(n>127) break;
                frame.ray.q=D_L00_001BF220.q;
            }
            if(accepted) { *pitch=-elevation; if(delta<0.17453295f) *yaw=heading; selected=candidate; best=distance; if(candidate==*(char **)(ctx+0x10)) return candidate; }
        }
next:
        ++index;
    }
    for(i=0;i<n;++i) { candidate=frame.disabled[i]; *(int *)(candidate+0x98)=*(int *)(*(char **)(candidate+0x24)+0x10); }
    return selected;
}
