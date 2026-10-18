extern int atrac_pause_lock_id;

void SND_PauseAtracResetLockId__Fv(void) {
    atrac_pause_lock_id = -1;
}
