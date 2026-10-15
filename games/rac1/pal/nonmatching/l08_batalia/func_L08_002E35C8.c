/* NON_MATCHING func_L08_002E35C8 -- src/overlays/l08_batalia/vendor_002E0258.c
 * Best so far: SIZE ours 668 / retail 664, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Selects a damaged child moby, accumulates state credit, launches a chosen/random child, and advances parent st
 *   Remaining differences: prologue save/constant scheduling, selected path scale load moved into gp branch slot r
 *   A source form preserving the retail scale-load placement and pseudo lifetimes would unblock it; p7 preserves r
 *   2026-10-15: compiles again. The file now declares it with a struct Moby * parameter, so the candidate
 *   takes that and casts it to the char * it used.
 */
extern void *func_L00_0025B478(void *, int, int);
extern void func_L00_00260108(void *, void *, int, float, float);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern float func_002140F8(float, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern float D_0015EE6C MACRO_ADDR;
/* Selects a damaged child moby, launches it, and advances the parent's state. */
void func_L08_002E35C8(struct Moby *moby) {
    char *parent = (char *)moby;
 char *data=*(char **)(parent+0x78);
 char *entries=data+0x60;
 char *p=entries;
 int i=0, score=0, state=-1, chosen=0;
 char *active_entry,*active_vec;
 do {
  char *m=*(char **)p;
  if(m && func_L00_0025B478(m,0x10000,0)) {
   if(i<7) {if(score<=0) {score=1; state=1;}}
   else if(i<10) {if(score<2) {score=2;state=1;}}
   else {if(score<4) {score+=4;chosen=i;state=2;}}
   (*(unsigned char **)p)[0xA4]=255;
  }
  ++i;p+=16;
 }while(i<34);
 *(short *)(data+0x28C)+=score;
 if(*(short *)(data+0x28C)>=8) {
  state=0;
  *(short *)(data+0x28C)=*(short *)(data+0x28C)%8;
  if(chosen) {
   char *entry=entries+chosen*16;
   char *m=*(char **)entry;
   char *v=*(char **)(m+0x78);
   func_L00_00260108(parent,m+0x10,-1,2.0f,13.0f);
   func_001F9BF0(v,(*(char **)entry)+0x10,(*(char **)(data+chosen*16+0x64))+0x10);
   active_entry=entry;active_vec=v;goto activate;
  } else {
   int start=func_001FA898_r(func_002140F8(0.0f,23.0f));
   for(i=0;i<24;i++) {
    int off=((i+start)%24+10)*16;
    char *entry=entries+off;
    char *m=*(char **)entry;
    if(m) {
     char *v=*(char **)(m+0x78);
     func_L00_00260108(parent,m+0x10,-1,2.0f,13.0f);
     func_001F9BF0(v,(*(char **)entry)+0x10,(*(char **)(data+off+0x64))+0x10);
     active_entry=entry;active_vec=v;goto activate;
    }
   }
   parent[0x20]=99;
  }
 }
 if(state>=0)func_0022ED80(state,0,(int)parent);
 return;
activate:
 func_L00_001FF4B0(active_vec,active_vec,D_0015EE6C*10.0f);
 (*(char **)active_entry)[0x20]=1;
 *(char **)active_entry=0;
}
