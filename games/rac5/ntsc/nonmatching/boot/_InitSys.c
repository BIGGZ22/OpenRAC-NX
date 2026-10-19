/* _InitSys 0x01ED8D40..0x01ED8D94 (84 bytes) — boot kernel init chain.
 *
 * Retail (SCUS_976.15) disassembly b03/disasm/_InitSys.txt:
 *   supplement_crt0 (0x01ED8B38, unnamed func_1ED8B38)
 *   InitSystemCallTableAddress (0x01ED8C30, unnamed func_1ED8C30)
 *   InitAlarm, InitTimer(2), StartTimerSystemTime,
 *   InitThread, InitExecPS2, InitTLBFunctions,
 *   tail j sceSifInitRebootNotify.
 * The RAC5 revision adds InitTimer/StartTimerSystemTime and a reboot-notify
 * tail call compared with the lombyte SDK corpus copy.
 * Source: lombyte-upstream src/sdk/library/_InitSys.c + retail disassembly.
 * Date: 2026-10-17 (w11/b03).
 */
extern void supplement_crt0(void);
extern void InitSystemCallTableAddress(void);
extern void InitAlarm(void);
extern void InitTimer(int);
extern void StartTimerSystemTime(void);
extern void InitThread(void);
extern void InitExecPS2(void);
extern void InitTLBFunctions(void);
extern void sceSifInitRebootNotify(void);

void _InitSys(void) {
    supplement_crt0();
    InitSystemCallTableAddress();
    InitAlarm();
    InitTimer(2);
    StartTimerSystemTime();
    InitThread();
    InitExecPS2();
    InitTLBFunctions();
    sceSifInitRebootNotify();
}
