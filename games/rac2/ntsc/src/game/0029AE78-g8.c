extern char *D_001A8CFC[] __attribute__((sda));
extern int D_001A9A54 __attribute__((sda));
extern char D_001A9A60 __attribute__((sda));
extern char D_001A9AB8 __attribute__((sda));
extern char D_00256418[];
extern int func_0026F7C8(void), func_0026F7D0(void);
extern void func_0029BCC8(void), func_0029B910(void);
extern void func_00349490(void *, int);
extern void func_0033AA00(void *,void *,void *);
extern void func_0033A9C8(void *,float);
extern void func_0033A9E0(void *,int);
void func_0029BA58(void) {
 int enabled;
 if (D_001A8CFC[0] != 0) {
  enabled=0;
  if(func_0026F7C8()) enabled=func_0026F7D0()!=0;
  if(D_001A9A54 != enabled) {
   func_0029BCC8();
   D_001A9A54=enabled;
   if(enabled==1) {
    func_00349490(D_001A8CFC[0]+0x3C950,(int)&D_001A9A60);
    func_0033AA00(D_001A8CFC[0]+0x3F138,&D_001A9AB8,D_00256418);
    func_0033A9C8(D_001A8CFC[0]+0x3F138,36.0f);
    func_0033A9E0(D_001A8CFC[0]+0x3F138,-1);
    func_0029B910();
   }
  }
 }
}
