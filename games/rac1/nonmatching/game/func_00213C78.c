/* Moby update loop. Fetches the head of the moby list from func_0020E6B8 and keeps it in
   D_00160024. For each moby whose state byte is non-negative (signed): calls func_0020E3D0
   unless flags 0x40 is set, calls its update callback at 0x74 if non-null, then calls
   func_0020ED48 unless flags 0x4 is set. Walks the list through next at 0x28. */
extern void *func_0020E6B8(void);
extern void func_0020E3D0(void *);
extern void func_0020ED48(void *);
extern void *D_00160024;

void func_00213C78(void) {
    char *m;

    m = (char *)func_0020E6B8();
    D_00160024 = m;
    if (m != 0) {
        do {
            if ((signed char)m[0x20] >= 0) {
                if (!(*(unsigned short *)(m + 0x34) & 0x40)) {
                    func_0020E3D0(m);
                }
                if (*(void (**)(void *))(m + 0x74) != 0) {
                    (*(void (**)(void *))(m + 0x74))(m);
                }
                if (!(*(unsigned short *)(m + 0x34) & 0x4)) {
                    func_0020ED48(m);
                }
            }
            m = *(char **)(m + 0x28);
        } while (m != 0);
    }
}
