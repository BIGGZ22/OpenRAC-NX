// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A stand-in for libnx's switch.h, for switch_clocks_mock_test.cpp only: the clock, APM and applet
// calls platform/switch/clocks.cpp makes, over fake rates the test drives (failures per domain,
// focus, mode, the system version for the pcv path, the applet hook). Not a model of Horizon.

#pragma once

#include <cstdint>

using u32 = std::uint32_t;
using Result = std::uint32_t;
#define R_SUCCEEDED(r) ((r) == 0)
#define R_FAILED(r) ((r) != 0)
#define R_MODULE(r) ((r) & 0x1FF)
#define R_DESCRIPTION(r) (((r) >> 9) & 0x1FFF)
#define MAKERESULT(module, description) ((((module) & 0x1FF)) | ((description) & 0x1FFF) << 9)
enum { Module_Libnx = 345 };
enum { LibnxError_NotInitialized = 2 };

enum PcvModuleId : u32 { PcvModuleId_CpuBus = 0x40000001, PcvModuleId_GPU = 0x40000002, PcvModuleId_EMC = 0x40000039 };
enum PcvModule : u32 { PcvModule_CpuBus = 0, PcvModule_GPU = 1, PcvModule_EMC = 56 };
struct ClkrstSession { int domain = -1; };
enum ApmPerformanceMode { ApmPerformanceMode_Invalid = -1, ApmPerformanceMode_Normal = 0, ApmPerformanceMode_Boost = 1 };
enum AppletHookType {
    AppletHookType_OnFocusState = 0, AppletHookType_OnOperationMode, AppletHookType_OnPerformanceMode,
    AppletHookType_OnExitRequest, AppletHookType_OnResume
};
enum AppletFocusState { AppletFocusState_InFocus = 1, AppletFocusState_OutOfFocus = 2, AppletFocusState_Background = 3 };
enum AppletOperationMode { AppletOperationMode_Handheld = 0, AppletOperationMode_Console = 1 };
using AppletHookFn = void (*)(AppletHookType, void*);
struct AppletHookCookie { int unused = 0; };

namespace fake {
inline u32 rates[3] = {1020000000u, 307200000u, 1331200000u};  // CPU, GPU, EMC
inline bool read_fail[3] = {}, write_fail[3] = {};
inline int writes = 0, clkrst_writes = 0, pcv_writes = 0;
inline AppletFocusState focus = AppletFocusState_InFocus;
inline AppletOperationMode mode = AppletOperationMode_Handheld;
inline bool new_system = true;  // 8.0.0 or later: clkrst
inline AppletHookFn hook = nullptr;
inline int initialized = 0, closed = 0;
inline int index_of_module(u32 id) { return id == PcvModuleId_CpuBus || id == PcvModule_CpuBus ? 0 : id == PcvModuleId_GPU || id == PcvModule_GPU ? 1 : 2; }
inline void stock() { rates[0] = 1020000000u; rates[1] = mode == AppletOperationMode_Console ? 768000000u : 307200000u; rates[2] = mode == AppletOperationMode_Console ? 1600000000u : 1331200000u; }
}  // namespace fake

inline bool hosversionAtLeast(int, int, int) { return fake::new_system; }
inline Result clkrstInitialize() { ++fake::initialized; return 0; }
inline void clkrstExit() { ++fake::closed; }
inline Result clkrstOpenSession(ClkrstSession* s, PcvModuleId id, u32) { s->domain = fake::index_of_module(id); return 0; }
inline void clkrstCloseSession(ClkrstSession* s) { s->domain = -1; }
inline Result clkrstGetClockRate(ClkrstSession* s, u32* hz) { if (fake::read_fail[s->domain]) return 0x6A0; *hz = fake::rates[s->domain]; return 0; }
inline Result clkrstSetClockRate(ClkrstSession* s, u32 hz) { if (fake::write_fail[s->domain]) return 0x6A0; fake::rates[s->domain] = hz; ++fake::writes; ++fake::clkrst_writes; return 0; }
inline Result pcvInitialize() { ++fake::initialized; return 0; }
inline void pcvExit() { ++fake::closed; }
inline Result pcvGetClockRate(PcvModule m, u32* hz) { const int d = fake::index_of_module(m); if (fake::read_fail[d]) return 0x6A0; *hz = fake::rates[d]; return 0; }
inline Result pcvSetClockRate(PcvModule m, u32 hz) { const int d = fake::index_of_module(m); if (fake::write_fail[d]) return 0x6A0; fake::rates[d] = hz; ++fake::writes; ++fake::pcv_writes; return 0; }
inline Result apmInitialize() { return 0; }
inline void apmExit() {}
inline Result apmGetPerformanceMode(ApmPerformanceMode* m) { *m = fake::mode == AppletOperationMode_Console ? ApmPerformanceMode_Boost : ApmPerformanceMode_Normal; return 0; }
inline Result apmGetPerformanceConfiguration(ApmPerformanceMode, u32* c) { *c = 0x00010000u; return 0; }
inline Result appletGetCurrentPerformanceConfiguration(u32* c) { *c = fake::mode == AppletOperationMode_Console ? 0x00020003u : 0x00020004u; return 0; }
inline AppletOperationMode appletGetOperationMode() { return fake::mode; }
inline AppletFocusState appletGetFocusState() { return fake::focus; }
inline void appletHook(AppletHookCookie*, AppletHookFn fn, void*) { fake::hook = fn; }
inline void appletUnhook(AppletHookCookie*) { fake::hook = nullptr; }
