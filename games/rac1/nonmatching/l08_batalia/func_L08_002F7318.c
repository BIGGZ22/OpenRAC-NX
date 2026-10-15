/* NON_MATCHING func_L08_002F7318 -- src/overlays/l08_batalia/vendor_002EAF48.c
 * Best so far: SIZE ours 4668 / retail 4676, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   ## Final round
 *   # func_L08_002F7318 — animated grid / effects
 *   Retail: 4,676 bytes. Best candidate: `best.c` = p23, SIZE 4,668 / 4,676. No source edits. 26 of 30 trials. Cla
 *   Stopped because p23, p24 and p25 produced identical compiler instructions **and branch graph**, checked with b
 *   The full C reconstruction covers noise updates, texture animation, rotated grid generation, UV arrays, quad dr
 *   Still mismatched: early scheduling, hoisted pointer spill order above 0x250, inner waveform drop/shape allocat
 *   `p23.align.txt` is the fully relocated comparison using overlay level 8. Earlier p0/p4/p5 alignment experiment
 *   No progress regeneration, commits or nonmatching edits. Lead can re-stage if useful.
 */
#include "common.h"
typedef float V45[4] __attribute__((aligned(16)));
typedef struct {
    float width,height;
    unsigned char nx,ny; unsigned short flags;
    unsigned char texture,frames,frame_ticks,current;
    unsigned char red,blue,green,alpha;
    float phase_start,phase_end,amp_start,amp2_start,amp_end,amp2_end;
    float wave_u,wave_v,wave2_v,wave2_u;
    unsigned char blend0,blend1,blend2,blend3;
    float u_rate,u_amp,u_phase,v_rate,v_amp,v_phase;
    float noise_amp,noise_speed,phase0,phase1,phase0_end,phase1_end;
    float tex_u_scale,tex_v_scale,normal_u,normal_v;
    float z0,z1,noise_range,noise_rate;
    float noise[8],goal[8],noise_u,noise_v,noise_bias;
    float particle_xy,particle_z,particle_size0,particle_size1;
    unsigned char particle_r,particle_g,particle_b,particle_a;
    short particle_life; unsigned char particle_count,particle_ticks;
    float particle_range; int particle_timer;
    float particle_offset,particle_height,puff_xy,puff_z,puff_size0,puff_size1;
    unsigned char puff_c0[4],puff_c1[4]; short puff_life0,puff_life1;
    unsigned char puff_count,puff_ticks; short puff_timer;
    float puff_range,puff_blend,puff_offset,puff_height;
    char pad134[3]; unsigned char sound_value,pad138,sound_range_x,sound_range_y;
    signed char sound_index; float sound_speed;
    V45 sound_position;
    char pad150[4]; float alpha_scale,alpha_u,alpha_v;
} Grid45;
typedef struct {
    char pad0[0x10]; V45 position;
    char pad20[3]; unsigned char alpha;
    char pad24[0x1C]; V45 rotation;
    char pad50[0x28]; Grid45 *data;
} M45;
typedef struct {
    V45 vertex[4]; unsigned int color[4]; float uv[8]; long gs[4];
} Quad45;
typedef struct {
    V45 corner[4],step,left_step,right_step,left,right,point,edge,normal;
    float matrix[16]; Quad45 quad;
    V45 scratch[5];
} Work45;
typedef struct { char pad0[0x10]; V45 position; } Sound45;
extern int D_L08_0015F4F8 MACRO_ADDR;
extern float D_0015EE60 MACRO_ADDR;
extern Sound45 D_L08_00180B40[];
extern float func_001FA888(int),func_001F9FA8(float),func_001F9F90(float),func_001F9B50(float);
extern float func_002140F8(float,float),func_L00_0025F368(float),func_001F9D10(void*,void*);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_001F4868(int),func_001F9850(int),func_001F9908(int*),func_001F9938(void*);
extern void func_001FA218(void*,void*),func_001F9EE8(void*,void*,void*);
extern void func_001F9BF0(void*,void*,void*),func_001F9BD8(void*,void*,void*),func_001F9CA0(void*,void*,void*);
extern void func_001F9C30(void*,void*,float),func_L00_001FF4B0(void*,void*,float);
extern float func_L08_002F7258(float,char*),func_L00_002001D8(void*,float);
extern void func_L00_001FD1D8(void*,void*,int);
extern unsigned char *particle45(void*,void*,void*,short,unsigned char,unsigned char,unsigned char,unsigned char,float) __asm__("func_L08_0027C218");
extern int func_L00_00258BC8(int,int);
extern void puff45(void*,void*,int,int,int,float) __asm__("func_L00_0026DD70");

/* Builds the animated grid, draws its quads, and updates attached effects. */
void func_L08_002F7318(M45 *moby) {
    V45 corner[4],step,left_step,right_step,left,right,point,edge,normal;
    float matrix[16];
    Quad45 quad;
    V45 scratch[5];
    float fraction;
    Grid45 *data;
    if (!moby || !(data=moby->data)) return;
    {
        int row_bytes=(data->nx+1)*sizeof(V45);
        V45 grid[data->ny+1][data->nx+1];
        unsigned char *points=(unsigned char*)grid;
        int remaining;
        float inv_x,inv_y;
        inv_x=1.0f/func_001FA888(data->nx);
        inv_y=1.0f/func_001FA888(data->ny);
        {
        float u[data->nx+1],v[data->ny+1];
        float phase,phase_step,amp,amp_step,amp2,amp2_step;
        float phase0_step,phase1_step,time,z0,z1,bias;
        int alpha,frame;
        float row_noise,phase0,phase1;
        int i,j;
        unsigned int color;
        float *noise=data->noise,*goal=data->goal;
        time=func_001FA888(D_L08_0015F4F8);
        for (remaining=7;remaining>=0;goal++,remaining--,noise++) {
            if (*noise<*goal) {
                *noise+=data->noise_range*data->noise_rate;
                if (!(*goal<=*noise)) continue;
            } else {
                *noise-=data->noise_range*data->noise_rate;
                if (!(*noise<=*goal)) continue;
            }
            *goal=func_002140F8(1.0f-data->noise_range,1.0f);
        }
        alpha=func_001FA898_r(data->alpha_scale*func_001FA888(data->alpha)*func_001FA888(moby->alpha)*0.0078125f)&0xFF;
        frame=D_L08_0015F4F8;
        if (data->frames) { frame/=data->frame_ticks+1; data->current=data->texture+frame%data->frames; }
        else data->current=data->texture;
        if (data->flags&1) quad.gs[1]=func_001F4868(data->current);
        else quad.gs[1]=func_001F4868(data->current+0x28);
        quad.gs[0]=0;
        quad.gs[2]=0xFF9000000260L;
        quad.gs[3]=(long)data->blend0|((long)data->blend1<<2)|((long)data->blend2<<4)|((long)data->blend3<<6)|((long)data->alpha<<32);
        color=(data->alpha<<24)|(data->blue<<16)|(data->green<<8)|data->red;
        quad.color[3]=color; quad.color[0]=color; quad.color[1]=color; quad.color[2]=color;
        qzero(&corner[0]); qzero(&corner[1]); qzero(&corner[2]); qzero(&corner[3]);
        {
        float half_width=data->width*0.5f;
        corner[3][0]=corner[1][0]=half_width;
        corner[0][0]=corner[2][0]=-half_width;
        }
        corner[0][2]=data->z0+0.01f; corner[1][2]=data->z1+0.01f;
        func_001FA218(matrix,&moby->rotation);
        func_001F9EE8(&corner[0],&corner[0],matrix);
        func_001F9EE8(&corner[1],&corner[1],matrix);
        func_001F9EE8(&corner[2],&corner[2],matrix);
        func_001F9EE8(&corner[3],&corner[3],matrix);
        func_001F9BF0(&left_step,&corner[2],&corner[0]);
        func_001F9C30(&left_step,&left_step,inv_y);
        func_001F9BF0(&right_step,&corner[3],&corner[1]);
        func_001F9C30(&right_step,&right_step,inv_y);
        phase_step=(data->phase_end-data->phase_start)*inv_y;
        amp_step=(data->amp_end-data->amp_start)*inv_y;
        amp2_step=(data->amp2_end-data->amp2_start)*inv_y;
        phase0_step=(data->phase0_end-data->phase0)*inv_x;
        phase1_step=(data->phase1_end-data->phase1)*inv_y;
        z0=data->height-data->z0; z1=data->height-data->z1;
        func_001F9BF0(&edge,&corner[1],&corner[0]);
        func_001F9CA0(&normal,&edge,&left_step);
        func_L00_001FF4B0(&normal,&normal,1.0f);
        qcopy(&left,&corner[0]); qcopy(&right,&corner[1]);
        bias=data->noise[0];
        phase=data->phase_start; amp=data->amp_start; amp2=data->amp2_start;
        phase0=data->phase0; phase1=data->phase1;
        {
        int i;
        register int j;
        for (i=0;i<=data->ny;phase+=phase_step,amp+=amp_step,amp2+=amp2_step,phase0+=phase0_step,phase1+=phase1_step,i++) {
            float y=func_001FA888(i)/func_001FA888(data->ny);
            qcopy(&point,&left);
            j=0;
            func_001F9BF0(&step,&right,&left);
            func_001F9C30(&step,&step,inv_x);
            row_noise=func_L08_002F7258(y,(char*)data);
            for (;j<=data->nx;j++) {
                float x=func_001FA888(j)/func_001FA888(data->nx);
                float theta=x*3.1415927f;
                float wave=time*phase+data->noise_amp*func_001F9FA8(func_L00_0025F368(data->noise_speed*time));
                float noise=func_L08_002F7258(x,(char*)data);
                float angle0,angle1,shape,drop,a,b,normal_angle0,normal_angle1;
                wave+=noise*data->noise_u+row_noise*data->noise_v+bias*data->noise_bias;
                drop=z0*x;
                drop+=z1*(1.0f-x);
                angle0=func_L00_0025F368(wave+phase0+data->wave_u*x+data->wave2_v*y);
                angle1=func_L00_0025F368(wave+phase1+data->wave_v*y+data->wave2_u*x);
                shape=func_001F9FA8(theta);
                shape=func_001F9B50(shape);
                drop*=shape;
                drop*=1.0f-y;
                func_L00_001FF4B0(&scratch[0],&step,amp*func_001F9F90(angle0)*shape*noise);
                func_L00_001FF4B0(&scratch[1],&left_step,amp2*func_001F9FA8(angle1)*shape*noise);
                a=func_001F9FA8(y*3.1415927f); b=func_001F9FA8(theta);
                normal_angle0=func_001F9FA8(func_L00_0025F368(wave+data->wave_u*x));
                normal_angle1=func_001F9FA8(func_L00_0025F368(wave+data->wave2_v*y));
                func_L00_001FF4B0(&scratch[3],&normal,a*b*(data->normal_u*normal_angle0+data->normal_v*normal_angle1));
                func_L00_001FF4B0(&scratch[4],&left_step,drop);
                func_001F9BD8(&scratch[2],&scratch[0],&scratch[1]);
                func_001F9BD8(&scratch[2],&scratch[2],&scratch[3]);
                func_001F9BF0(&scratch[2],&scratch[2],&scratch[4]);
                func_001F9BD8((V45*)(points+i*row_bytes)+j,&point,&scratch[2]);
                *(float*)(points+(i*row_bytes+j*sizeof(V45))+12)=1.0f/((1.0f-data->alpha_u+data->alpha_u*shape*shape*shape)*(1.0f-data->alpha_v+data->alpha_v*y));
                func_001F9BD8(&point,&point,&step);
            }
            func_001F9BD8(&left,&left,&left_step);
            func_001F9BD8(&right,&right,&right_step);
        }
        }
        {
            int i;
            float f=func_L00_002001D8(&fraction,time*data->u_rate+data->u_amp*func_001F9FA8(func_L00_0025F368(time*data->u_phase)));
            for (i=0;i<=data->nx;i++) { u[i]=f; f+=inv_x/data->tex_u_scale; }
            f=func_L00_002001D8(&fraction,time*data->v_rate+data->v_amp*func_001F9FA8(func_L00_0025F368(time*data->v_phase)));
            for (i=0;i<=data->ny;i++) { v[i]=f; f+=inv_y/data->tex_v_scale; }
        }
        {
        int i;
        for (i=0;i<data->ny;i++) {
            if (data->nx) {
            int row_offset=i*row_bytes,next_offset=(i+1)*row_bytes;
            V45 *row=(V45*)(points+row_offset),*next_row=(V45*)(points+next_offset);
            int j;
            for (j=0;j<data->nx;j++) {
                quad.uv[0]=u[j]; quad.uv[1]=v[i];
                quad.uv[2]=u[j+1]; quad.uv[3]=v[i];
                quad.uv[4]=u[j]; quad.uv[5]=v[i+1];
                quad.uv[6]=u[j+1]; quad.uv[7]=v[i+1];
                func_001F9BD8(&quad.vertex[0],&row[j],&moby->position);
                func_001F9BD8(&quad.vertex[1],&row[j+1],&moby->position);
                func_001F9BD8(&quad.vertex[2],&next_row[j],&moby->position);
                func_001F9BD8(&quad.vertex[3],&next_row[j+1],&moby->position);
                quad.color[0]=((alpha/func_001FA898_r(*(float*)(points+(row_offset+(j)*sizeof(V45))+12)))<<24)|(data->blue<<16)|(data->green<<8)|data->red;
                quad.color[1]=((alpha/func_001FA898_r(*(float*)(points+(row_offset+(j+1)*sizeof(V45))+12)))<<24)|(data->blue<<16)|(data->green<<8)|data->red;
                quad.color[2]=((alpha/func_001FA898_r(*(float*)(points+(next_offset+(j)*sizeof(V45))+12)))<<24)|(data->blue<<16)|(data->green<<8)|data->red;
                quad.color[3]=((alpha/func_001FA898_r(*(float*)(points+(next_offset+(j+1)*sizeof(V45))+12)))<<24)|(data->blue<<16)|(data->green<<8)|data->red;
                func_L00_001FD1D8(&quad,0,0);
            }
            }
        }
        }
        if (data->flags&2) {
            if (func_001F9908(&data->particle_timer)) {
                data->particle_timer=func_001F9850(data->particle_ticks);
                for (i=0;i<data->particle_count;i++) {
                    float width=data->width*0.5f;
                    float x=func_002140F8(-1.0f,1.0f)*data->particle_range;
                    float fraction=func_002140F8(0.0f,x);
                    float size;
                    short life;
                    int r,g,b,a;
                    func_L00_001FF4B0(&scratch[2],&left_step,-(data->particle_offset+func_001F9B50(func_001F9F90(fraction*3.1415927f))*data->particle_height));
                    func_L00_001FF4B0(&scratch[1],&step,fraction*width);
                    func_001F9BD8(&scratch[1],&scratch[1],&moby->position);
                    func_001F9BD8(&scratch[1],&scratch[1],&scratch[2]);
                    scratch[0][0]=func_002140F8(-data->particle_xy,data->particle_xy)*D_0015EE60;
                    scratch[0][1]=func_002140F8(-data->particle_xy,data->particle_xy)*D_0015EE60;
                    scratch[0][2]=func_002140F8(0.0f,data->particle_z)*D_0015EE60;
                    size=func_002140F8(data->particle_size0,data->particle_size1);
                    life=func_001F9850(data->particle_life);
                    r=data->particle_r; g=data->particle_g; b=data->particle_b;
                    a=func_001FA898_r(func_001FA888(data->particle_a)*fraction);
                    particle45(moby,&scratch[1],&scratch[0],life,r,g,b,a,size);
                }
            }
            if (func_001F9938(&data->puff_timer)) {
                data->puff_timer=func_001F9850(data->puff_ticks);
                for (i=0;i<data->puff_count;i++) {
                    int c0=(data->puff_c0[3]<<24)|(data->puff_c0[2]<<16)|(data->puff_c0[1]<<8)|data->puff_c0[0];
                    int c1=(data->puff_c1[3]<<24)|(data->puff_c1[2]<<16)|(data->puff_c1[1]<<8)|data->puff_c1[0];
                    float width=data->width*0.5f;
                    float size=func_002140F8(data->puff_size0,data->puff_size1);
                    int life=func_001F9850(func_L00_00258BC8(data->puff_life0,data->puff_life1));
                    float fraction=func_002140F8(-1.0f,1.0f)*data->puff_range;
                    float shape;
                    fraction=func_002140F8(-fraction,fraction);
                    shape=func_001F9B50(func_001F9F90(fraction*3.1415927f));
                    func_L00_001FF4B0(&scratch[2],&left_step,-(data->puff_offset+shape*data->puff_height));
                    func_L00_001FF4B0(&scratch[1],&step,fraction*width);
                    func_001F9BD8(&scratch[1],&scratch[1],&moby->position);
                    func_001F9BD8(&scratch[1],&scratch[1],&scratch[2]);
                    scratch[0][0]=func_002140F8(-data->puff_xy,data->puff_xy)*D_0015EE60;
                    scratch[0][1]=func_002140F8(-data->puff_xy,data->puff_xy)*D_0015EE60;
                    scratch[0][2]=func_002140F8(0.0f,data->puff_z)*D_0015EE60;
                    func_001F9C30(&scratch[0],&scratch[0],1.0f-data->puff_blend+shape*data->puff_blend);
                    puff45(&scratch[1],&scratch[0],c0,c1,life,size);
                }
            }
            if (data->sound_index!=-1) {
                Sound45 *sound=&D_L08_00180B40[data->sound_index];
                if (func_001F9D10(&data->sound_position,&sound->position)<data->sound_speed*D_0015EE60) {
                    float x=(func_002140F8(-(int)data->sound_range_x,data->sound_range_x)+data->width)*0.5f;
                    float y=(func_002140F8(-(int)data->sound_range_y,data->sound_range_y)+data->height)*0.5f;
                    func_L00_001FF4B0(&scratch[0],&step,x);
                    func_L00_001FF4B0(&scratch[1],&left_step,-y);
                    func_001F9BD8(&data->sound_position,&scratch[0],&scratch[1]);
                    func_001F9BD8(&data->sound_position,&data->sound_position,&corner[0]);
                    func_001F9BD8(&data->sound_position,&data->sound_position,&moby->position);
                } else {
                    func_001F9BF0(&scratch[0],&data->sound_position,&sound->position);
                    func_L00_001FF4B0(&scratch[0],&scratch[0],data->sound_speed*D_0015EE60);
                    func_001F9BD8(&sound->position,&sound->position,&scratch[0]);
                    sound->position[3]=func_001FA888(data->sound_value);
                }
            }
            data->flags^=2;
        }
        }
    }
}
