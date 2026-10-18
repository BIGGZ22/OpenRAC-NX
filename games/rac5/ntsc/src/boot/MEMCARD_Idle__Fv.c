extern unsigned int memcard_idle_status;

int MEMCARD_Idle__Fv(void) {
    return (memcard_idle_status & 0xFF00FF00u) == 0;
}
