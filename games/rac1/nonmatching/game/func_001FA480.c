/* fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* Copies three quadwords (48 bytes) from arg1 to arg0: three 128-bit loads, then three stores. */
void func_001FA480(void *arg0, void *arg1) {
    *(unsigned long long *)((char *)arg0 + 0x00) = *(unsigned long long *)((char *)arg1 + 0x00);
    *(unsigned long long *)((char *)arg0 + 0x08) = *(unsigned long long *)((char *)arg1 + 0x08);
    *(unsigned long long *)((char *)arg0 + 0x10) = *(unsigned long long *)((char *)arg1 + 0x10);
    *(unsigned long long *)((char *)arg0 + 0x18) = *(unsigned long long *)((char *)arg1 + 0x18);
    *(unsigned long long *)((char *)arg0 + 0x20) = *(unsigned long long *)((char *)arg1 + 0x20);
    *(unsigned long long *)((char *)arg0 + 0x28) = *(unsigned long long *)((char *)arg1 + 0x28);
}
