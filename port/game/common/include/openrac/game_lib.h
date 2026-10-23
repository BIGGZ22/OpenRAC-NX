/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The library replacements every game shares (lib/), one per entry of
 * port/game/common/libraries.tsv, with the signature given there in the
 * port's types (a pointer is a gaddr). Each game's libraries.c (hostgen)
 * binds the game's addresses to them. test_hostgen.py checks that this list
 * and the table agree. */
#ifndef OPENRAC_GAME_LIB_H
#define OPENRAC_GAME_LIB_H

#include "openrac/guest.h"

#ifdef __cplusplus
extern "C" {
#endif

/* libc */
int openrac_lib_memcmp(gaddr, gaddr, unsigned int);
gaddr openrac_lib_memcpy(gaddr, gaddr, unsigned int);
gaddr openrac_lib_memset(gaddr, int, unsigned int);
gaddr openrac_lib_strchr(gaddr, int);
int openrac_lib_strcmp(gaddr, gaddr);
gaddr openrac_lib_strcpy(gaddr, gaddr);
int openrac_lib_strlen(gaddr);
int openrac_lib_strncmp(gaddr, gaddr, int);
gaddr openrac_lib_strncpy(gaddr, gaddr, int);
/* kernel */
int openrac_lib_AddIntcHandler(int, gaddr, int);
int openrac_lib_RemoveIntcHandler(int, int);
int openrac_lib_AddDmacHandler(int, gaddr, int);
int openrac_lib_RemoveDmacHandler(int, int);
int openrac_lib_CreateThread(gaddr);
int openrac_lib_DeleteThread(int);
int openrac_lib_StartThread(int, gaddr);
int openrac_lib_TerminateThread(int);
int openrac_lib_ChangeThreadPriority(int, int);
int openrac_lib_RotateThreadReadyQueue(int);
int openrac_lib_GetThreadId(void);
int openrac_lib_CreateSema(gaddr);
int openrac_lib_DeleteSema(int);
int openrac_lib_SignalSema(int);
int openrac_lib_WaitSema(int);
int openrac_lib_EnableCache(int);
void openrac_lib_FlushCache(int);
void openrac_lib_SyncDCache(gaddr, gaddr);
void openrac_lib_InvalidDCache(gaddr, gaddr);
int openrac_lib_sceSifDmaStat(unsigned int);
unsigned int openrac_lib_sceSifSetDma(gaddr, int);
void openrac_lib_SetGsCrt(int interlace, int mode, int field);
void openrac_lib_GsPutIMR(unsigned int mask);
unsigned long long openrac_lib_GsGetIMR(void);
int openrac_lib_EnableIntc(int);
int openrac_lib_DisableIntc(int);
int openrac_lib_DisableDmac(int);
int openrac_lib_EnableDmac(int);
void openrac_lib_Exit(int);
int openrac_lib_AddDmacHandler2(int, gaddr, int, gaddr);
int openrac_lib_SetAlarm(unsigned short, gaddr, gaddr);
int openrac_lib_ReferThreadStatus(int, gaddr);
int openrac_lib_SleepThread(void);
int openrac_lib_WakeupThread(int);
int openrac_lib_SuspendThread(int);
gaddr openrac_lib_EndOfHeap(void);
int openrac_lib_PollSema(int);
void openrac_lib_SetOsdConfigParam(gaddr);
void openrac_lib_GetOsdConfigParam(gaddr);
void openrac_lib_GetOsdConfigParam2(gaddr, int, int);
int openrac_lib__SetTLBEntry(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
int openrac_lib_Copy(gaddr, gaddr, int);
int openrac_lib_iCopy(gaddr, gaddr, int);
gaddr openrac_lib_GetEntryAddress(int);
void openrac_lib_SetSyscall(int, gaddr);
gaddr openrac_lib_FindAddress(gaddr, gaddr, gaddr);
void openrac_lib_sceSifStopDma(void);
void openrac_lib_SetVSyncFlag(gaddr, gaddr);
void openrac_lib_sceSifSetDChain(void);
int openrac_lib_sceSifSetReg(unsigned int, int);
int openrac_lib_sceSifGetReg(unsigned int);
int openrac_lib_Deci2Call(int, gaddr);
int openrac_lib_GetMemorySize(void);
int openrac_lib__InitTLB(void);
int openrac_lib_RFU000(void);
void openrac_lib_ResumeT3IntrDispatch(int);
int openrac_lib_Interrupt2Iop(int);
int openrac_lib_ReleaseAlarm(int);
int openrac_lib_ReleaseWaitThread(int);
int openrac_lib_CancelWakeupThread(int);
int openrac_lib_ResumeThread(int);
void openrac_lib_SetGsHParam(gaddr, gaddr, gaddr, gaddr);
int openrac_lib_SetEventFlag(int, unsigned int);
int openrac_lib_DisableIntcHandler(int, int);
int openrac_lib_DisableDmacHandler(int, int);
int openrac_lib_DisableCache(int);
unsigned int openrac_lib_GetCop0(int);
void openrac_lib_SetCPUTimerHandler(gaddr);
void openrac_lib_SetPgifHandler(gaddr);
void openrac_lib__ExecOSD(int, gaddr);
/* sif */
void openrac_lib_sceSifInitRpc(int);
int openrac_lib_sceSifBindRpc(gaddr, unsigned int, int);
int openrac_lib_sceSifCallRpc(gaddr, int, int, gaddr, int, gaddr, int, gaddr, gaddr);
int openrac_lib_sceSifCheckStatRpc(gaddr);
/* fileio */
int openrac_lib_sceFsReset(void);
int openrac_lib_sceOpen(gaddr, int, ...);
int openrac_lib_sceClose(int);
int openrac_lib_sceLseek(int, int, int);
int openrac_lib_sceRead(int, gaddr, int);
int openrac_lib_sceWrite(int, gaddr, int);
/* sif */
int openrac_lib_sceSifInitIopHeap(void);
int openrac_lib_sceSifAllocIopHeap(int);
int openrac_lib_sceSifFreeIopHeap(int);
int openrac_lib_sceSifLoadModuleBuffer(gaddr, int, gaddr);
int openrac_lib_sceSifSyncIop(void);
int openrac_lib_sceSifRebootIop(gaddr);
/* kernel */
int openrac_lib_DIntr(void);
int openrac_lib_EIntr(void);
/* vu */
void openrac_lib_sceGsResetPath(void);
int openrac_lib_sceGsSyncPath(int, unsigned short);
/* libgcc */
void openrac_lib___main(void);
double openrac_lib___extendsfdf2(float);
long long openrac_lib___muldi3(long long, long long);
unsigned long long openrac_lib___udivdi3(unsigned long long, unsigned long long);
unsigned long long openrac_lib___umoddi3(unsigned long long, unsigned long long);
unsigned long long openrac_lib___fixunsdfdi(double);
/* cdvd */
int openrac_lib_sceCdSync(int);
int openrac_lib_sceCdInit(int);
int openrac_lib_sceCdDiskReady(int);
int openrac_lib_sceCdMmode(int);
int openrac_lib_sceCdRead(unsigned int, unsigned int, gaddr, gaddr);
int openrac_lib_sceCdGetError(void);
int openrac_lib_sceCdBreak(void);
int openrac_lib_sceCdReadClock(gaddr);
/* graph */
void openrac_lib_sceGsResetGraph(short, short, short, short);
void openrac_lib_sceGsSetDefDispEnv(gaddr, short, short, short, short, short);
void openrac_lib_sceGsPutDispEnv(gaddr);
int openrac_lib_sceGsPutDrawEnv(gaddr);
int openrac_lib_sceGsSyncV(int);
void openrac_lib_sceGsSetDefLoadImage(gaddr, short, short, short, short, short, short, short);
int openrac_lib_sceGsSetDefStoreImage(gaddr, short, short, short, short, short, short, short);
int openrac_lib_sceGsExecLoadImage(gaddr, gaddr);
int openrac_lib_sceGsExecStoreImage(gaddr, gaddr);
gaddr openrac_lib_sceGsSyncVCallback(gaddr);
void openrac_lib__GetGsDxDyOffset(int, gaddr, gaddr, gaddr, gaddr);
/* dma */
int openrac_lib_sceDmaReset(int);
void openrac_lib_sceDmaSend(gaddr, gaddr);
/* mc */
int openrac_lib_sceMcInit(void);
int openrac_lib_sceMcOpen(int, int, gaddr, int);
int openrac_lib_sceMcMkdir(int, int, gaddr);
int openrac_lib_sceMcClose(int);
int openrac_lib_sceMcSeek(int, int, int);
int openrac_lib_sceMcRead(int, gaddr, int);
int openrac_lib_sceMcWrite(int, gaddr, int);
int openrac_lib_sceMcSync(int, gaddr, gaddr);
int openrac_lib_sceMcGetInfo(int, int, gaddr, gaddr, gaddr);
int openrac_lib_sceMcGetDir(int, int, gaddr, unsigned int, int, gaddr);
int openrac_lib_sceMcFormat(int, int);
int openrac_lib_sceMcDelete(int, int, gaddr);
int openrac_lib_sceMcUnformat(int, int);
int openrac_lib_sceMcChdir(int, int, gaddr, gaddr);
/* dbc */
int openrac_lib_sceDbcInit(void);
/* pad2 */
int openrac_lib_scePad2Init(int);
int openrac_lib_scePad2CreateSocket(gaddr, gaddr);
int openrac_lib_scePad2Read(int, gaddr);
int openrac_lib_scePad2GetButtonProfile(int, gaddr);
int openrac_lib_scePad2GetState(int);
/* vib */
int openrac_lib_sceVibGetProfile(int, gaddr);
/* vu0 */
void openrac_lib_sceVu0RotMatrixZ(gaddr, gaddr, float);
void openrac_lib_sceVu0RotMatrixX(gaddr, gaddr, float);
void openrac_lib_sceVu0RotMatrixY(gaddr, gaddr, float);
/* scf */
int openrac_lib_sceScfGetLanguage(void);
void openrac_lib_sceScfGetLocalTimefromRTC(gaddr);
/* 989snd */
int openrac_lib_snd_StartSoundSystem(void);
int openrac_lib_snd_FlushSoundCommands(void);
unsigned int openrac_lib_snd_BankLoadByLoc(int, int);
void openrac_lib_snd_BankLoadFromEE_CB(int, int, long long);
void openrac_lib_snd_ResolveBankXREFS(void);
void openrac_lib_snd_UnloadBank(int);
void openrac_lib_snd_SetMasterVolume(int, int);
void openrac_lib_snd_SetPlaybackMode(int);
void openrac_lib_snd_StopAllSounds(void);
void openrac_lib_snd_StopSound(int);
void openrac_lib_snd_PauseAllSoundsInGroup(int);
void openrac_lib_snd_ContinueAllSoundsInGroup(int);
void openrac_lib_snd_SoundIsStillPlaying_CB(int, int, int);
void openrac_lib_snd_reset_state_and_flush_commands(void);
int openrac_lib_snd_InitVAGStreamingEx(int, int, int, int);
void openrac_lib_snd_StopAllStreams(void);
void openrac_lib_snd_PlayVAGStreamByLocEx_CB(
    int, int, int, int, int, int, int, int, int, int, long long
);
void openrac_lib_snd_PauseVAGStream(int);
void openrac_lib_snd_ContinueVAGStream(int);
void openrac_lib_snd_GetVAGStreamTimeRemaining_CB(int, int, int);
void openrac_lib_snd_IsVAGStreamBuffered_CB(int, int, int);
void openrac_lib_snd_StreamSafeCheckCDIdle(int);
int openrac_lib_snd_StreamSafeCdRead(int, int, int);
int openrac_lib_snd_StreamSafeCdSync(int);
int openrac_lib_snd_StreamSafeCdBreak(void);
int openrac_lib_snd_StreamSafeCdGetError(void);
int openrac_lib_snd_StreamSafeCdCallback(int);
void openrac_lib_snd_PreAllocReverbWorkArea(int, int);
void openrac_lib_snd_AutoReverb(int, int, int, int);
void openrac_lib_snd_ResetMovieSound(void);
void openrac_lib_snd_StartMovieSound(int, int, int, int, int);
int openrac_lib_snd_UpdateMovieADPCM(int, int);
int openrac_lib_snd_GetMovieNAX(void);
int openrac_lib_snd_GetDopplerPitchMod(int);

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_GAME_LIB_H */
