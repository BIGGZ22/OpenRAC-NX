# Controllers: the DualSense, the DualShock 2, and what Ratchet & Clank
actually uses

What OpenRAC's port does with a PlayStation 5 controller today, what the
PlayStation 2 game actually asks a controller for, and what it would take to
use the DualSense's advanced features — adaptive triggers, HD haptics, the
light bar, the gyroscope.

Every claim about the port cites a file and line, checked against the tree at
commit `b522692a`. The SDL3 and DualSense claims are sourced to the SDL
headers, SDL's own driver, the Linux kernel and the community references
listed at the end.

## 1. The short answer

| Question | Answer |
|---|---|
| **Does the port work with a PS5 DualSense today?** | **Yes.** SDL3 recognises it and the port reads it through SDL's generic gamepad API: buttons, both sticks, both triggers. |
| **Does the game use analog button pressure?** | **Yes, but only for one thing**: the hero's **swimming turn rate**, scaled by the pressure of the **Cross** and **Square** buttons. |
| **Could a DualSense supply that?** | **No** — a DualSense's face buttons are digital; only its triggers are analog, and the game never reads the triggers' pressure. |
| **Does the game use rumble?** | **No — and neither does the retail game.** The retail executable links no motor command at all. There is no rumble to restore; any rumble would be a *new* feature. |
| **Does SDL3 expose adaptive triggers?** | **No.** SDL's maintainer ruled it out in 2022; the request was re-opened in 2026 and is still open. The raw escape hatch, `SDL_SendGamepadEffect()`, exists since SDL 3.2.0. |
| **Does SDL3 expose the light bar?** | **Yes** — `SDL_SetGamepadLED()`, since 3.2.0. |
| **Does SDL3 expose the gyroscope?** | **Yes** — `SDL_GamepadHasSensor` / `SDL_SetGamepadSensorEnabled` / `SDL_GetGamepadSensorData`, since 3.2.0. |
| **Would adaptive triggers help this game?** | **Not for fidelity.** Nothing in R&C1 reads a trigger's travel. They could only be a *presentation* extra — a resistance cue on the Swingshot, say — and that would be a mod, not the port. |

The port's own notes already say much of this
(`docs/port/SETTINGS.md:312`): *"Controller LED, trigger effects, pressure |
yes | no | no | **never** for the LED and triggers (nothing in the game drives
them); pressure **later**"*. This document explains *why* — and what remains
doable.

## 2. What the port does today

The input path is short and entirely generic:

```
SDL3 events                                    port/platform/input.cpp:110-188
  SDL_EVENT_GAMEPAD_ADDED -> SDL_OpenGamepad   input.cpp:112-119
  buttons polled                                input.cpp:240-244
  triggers polled as axes                      input.cpp:245-254
  sticks polled as axes                        input.cpp:255-258
        |
        v
platform::Pad                                    port/platform/pad.h:96-119
  buttons (16 bits, active low) · 4 stick bytes
  pressure[12]  (per-button, 0..255)
  small_motor / large_motor
        |
        v
frontend::pad(port, &buttons, analog[4])         port/game/common/frontend.cpp:3072-3150
        |
        v
openrac_game_pad(port, &buttons, analog)          port/game/common/main.cpp:1220-1231
        |
        v
openrac_lib_scePad2Read(socket, data)             port/game/common/lib/pad.c:42-64
  18 bytes: 2 buttons + 4 sticks + 12 pressures
        |
        v
the game's Pad_ProcessReport                     games/rac1/src/game/pad.c:336-492
  analog[0..3]  = the four sticks
  analog[4..15] = the twelve pressures
```

Gamepad buttons are mapped **by position**, so the bottom face button is
Cross on a DualSense and A on an Xbox pad
(`port/platform/input.cpp:46-62`).

### The three things SDL3 gives the port that it does not use

| SDL3 feature | In the port? |
|---|---|
| **Touchpad click** (`SDL_GAMEPAD_BUTTON_TOUCHPAD`) | No |
| **Gyroscope / accelerometer** | No |
| **Light bar** (`SDL_SetGamepadLED`) | No |
| **Rumble** (`SDL_RumbleGamepad`) | Called every frame, **always with zero** (`input.cpp:262-264`) |
| **Player LEDs / player index** | No — ports are assigned by connection order (`input.cpp:112-129`, `:190-193`) |

There is no DualSense-specific code anywhere in the tree: no
`SDL_GAMEPAD_TYPE_PS5` check, no `SDL_SendGamepadEffect`, no `SDL_hid_*`, no
sensor call. A grep for `dualsense|adaptive|trigger resistance|haptic|lightbar|
gyro|accelerometer` over `port/` returns prose in `docs/port/OPENGOAL_NOTES.md`
about what *OpenGOAL* does, and nothing else.

## 3. What the game actually asks the controller for

This is the part that decides everything, so it is worth being exact.

### 3.1 The pressure bytes

The DualShock 2 has twelve pressure-sensitive buttons: the D-pad, the four
face buttons, L1, R1, L2 and R2. The game's pad library returns twelve
pressure bytes, and the game turns them into floats:

```c
if (len >= 0x12) {
    for (i = 4; i < 16; i++) {
        p->analog[i] = func_001FA888_181F0(buf[i + 2]) * 0.003921569f;
    }
}
```

— `games/rac1/src/game/pad.c:363-367` (the `1/255` is what makes the range
0..1). `PadState.analog[16]` is documented at `games/rac1/include/pad.h:28`:
*"[0..3] stick axes (-1..1), [4..15] pressure (0..1)"*.

The order is right, left, up, down, triangle, circle, cross, square, L1, R1,
L2, R2 — the port's own order (`port/game/common/lib/pad.c:43`,
`port/platform/pad.h:67-82`), matching the community's memory map
(`games/rac1/docs/kb/data-structures.md:473-474`).

So `analog[10]` is **Cross** and `analog[11]` is **Square**.

### 3.2 The one consumer: swimming

Searching every `analog[...]` read in the whole game
(`grep -rn "analog\[" games/rac1/src/`) finds exactly four pressure reads,
and they are all in one place: the hero's **swimming** turn rate.

```c
lim = D_0015EE6C * 0.6981317f;                                    // 40 degrees
trig = max(D_0013CA40.analog[10], D_0013CA40.analog[11]);        // Cross / Square pressure
lim = lim + (D_0015EE6C * 1.2217305f - lim) * trig;               // up to 70 degrees
if ((held & 0x80) && !(held & 0x40)) {                            // Square
    t = D_0013CA40.analog[11] * 1.5f;                             // Square pressure
    ...
    turn = t * 1.3613569f;                                        // ~78 degrees/second
} else if (held & 0x40) {                                        // Cross
    t = D_0013CA40.analog[10] * 1.5f;                             // Cross pressure
    ...
    turn = t * -1.3613569f;
}
```

— `games/rac1/src/overlays/l01_novalis/help_002343F8.c:663-716`, and the
same function in each level's own copy (Kerwan `l03_kerwan/help_00205E28.c`,
Rilgar `l05_rilgar/help_00245208.c`, Batalia `l08_batalia/help_0021DFB8.c`,
Pokitaru `l11_pokitaru/help_0023CD98.c`, Hoven `l12_hoven/help_0022E428.c`,
Quartu `l15_quartu/help_00204AC0.c`, Kalebo III `l16_kalebo3/help_00214C88.c`,
Drek's Fleet `l17_fleet/help_00214F68.c`, Veldin 2 `l18_veldin2/help_0021DFB8.c`,
and the shared copy `overlays/shared/help_002180F8.c`).

The state this is in is the underwater one: the block is `case 0x33/0x34/0x35`,
and `0x33` is `HERO_STATE_ENTER_UNDERWATER`
(`games/rac1/ntsc/src/overlays/l01/gameplay/hero/hero_set_state.c:82`). The
same block raises `HeroKickUpDust` and `RippleDisturb` — water, not dust.

**Mechanically:** holding Square or Cross turns the hero while swimming; how
*hard* you press sets the turn rate, and how hard sets how far you can turn
(40° to 70° of lean). A Cross pressure above 0.75 also feeds a usage
statistic (record 11).

**This is the only analog button input Ratchet & Clank 1 has.** Everything else
was checked and does not read a pressure value:

| Mechanic | Reads pressure? | Evidence |
|---|---|---|
| **Hoverboard** | No | Steering is the left stick; the boost is the digital Square button (`l05_rilgar/help_00245208.c:850-857`) |
| **Swingshot** | No | `overlays/shared/vendor_002C12B0.c:1806-1860`, no `analog` read |
| **Suck Cannon** | No | `overlays/shared/vendor_002D9438.c:1036`, no `analog` read |
| **Thruster Pack / Heli Pack** | No | Flight is stick-driven (`l01_novalis/help_002343F8.c:1936`) |
| **Ship flight between planets** | No | `games/rac1/src/game/space.c:1074`, no `analog` read |
| **Every weapon** | No | `overlays/shared/help_00237B00.c` tests only digital masks |
| **Camera, galaxy map, vendor menus** | No | Only `analog[0..3]`, the sticks |
| **Blowtorch** | **Does not exist in R&C1** | It is a Ratchet & Clank 2 weapon; `grep -rni blowtorch games/rac1/` returns nothing |

So: **the whole of Ratchet & Clank 1's analog-button surface is the pressure
of Cross and Square while the hero is underwater.**

### 3.3 The control-mode detection

The game asks the pad whether it has pressure at all, and behaves differently
if it does not:

```c
n = func_00124DF0(p->socket, buf);          /* scePad2GetButtonProfile */
if (*(unsigned int *)buf == 0xFFFFFFFF) {
    p->unk1DC = 0x79;                        /* "this pad reports pressure" */
} else {
    p->unk1DC = 0;                           /* "digital pad" */
}
```

— `games/rac1/src/game/pad.c:221-226`; the field is documented at
`games/rac1/include/pad.h:56`.

When `unk1DC != 0x79`, the swimming code substitutes a **time ramp** for the
pressure value — hold the button longer and the turn gets faster, up to 1.0
after 0x14 frames (`l01_novalis/help_002343F8.c:679-686` and `:701-708`).

So the game has a first-class path for a **digital** controller. That matters:
a DualSense's digital face buttons are a supported configuration, not a
degradation.

### 3.4 What the port reports

The port returns an all-ones button profile
(`port/game/common/lib/pad.c:67-72`), so the game always takes the pressure
branch. That is the honest answer for what the port emulates — it does
deliver twelve pressure bytes — but the values it delivers are binary.

## 4. What the game does *not* use: rumble

**Ratchet & Clank 1 has no rumble — not on the console either.**

The retail executable's linked SDK functions in the pad and vibration area are
exactly these (`games/rac1/ntsc/config/us/rnc1.us.yaml:558-574`):

```
sdk/input/sce_pad2_init
sdk/input/sce_pad2_create_socket
sdk/input/sce_pad2_read
sdk/input/sce_pad2_get_button_profile
sdk/input/sce_pad2_get_state
sdk/input/sce_pad2_link_driver
sdk/input/sce_pad2_get_side
sdk/input/sce_pad2_check_dma
sdk/input/sce_pad2_set_button_order
sdk/vib/sce_vib_get_profile
```

There is **no `scePadSetActDirect`, no `scePadSetActAlign`, no
`sceVibSetActParam`** — the motor command was dead-stripped by the linker.
The decompilation says so in its own words
(`games/rac1/src/core/00125218.c:9-11`): *"Sony's libvib, libvib.o
(sceVibGetProfile; sceVibSetActParam was dead-stripped)."*

There *is* a vibration mixer in the game — `actuator_CalcPower`
(`games/rac1/src/game/actuator.c:30-133`), an eight-slot waveform table
(`D_00165580[8]`) that mixes down to two motor values per tick. But it has
**no caller**: `grep -rn "actuator_CalcPower" games/rac1/src/` finds only its
own definition and the name table (`games/rac1/include/names.h:17`). The
retail in-game debug menu even has an **"Actuator" on/off row**
(`games/rac1/docs/kb/datamining.md:231`), whose flag is initialised and
toggled but never read. The feature was shelved.

**So the port's silence is faithful.** There is no rumble to restore, and
adding `scePadSetActDirect` to rac1's library table would bind an address the
game never calls.

The port's plumbing already exists and is simply unused: `platform::Pad`'s
`small_motor` and `large_motor` (`port/platform/pad.h:104-107`), carried
across frames (`input.cpp:231-235`) and played through
`SDL_RumbleGamepad(g, low, high, 100)` (`input.cpp:262-264`) — with `low` and
`high` always zero.

For the other games this is a real gap:

| Game | Retail rumble call | Port |
|---|---|---|
| rac1 | none linked | none needed |
| rac2 | `scePadSetActAlign` (`games/rac2/ntsc/symbol_addrs/eu/symbol_addrs.txt:494`) | not bound |
| rac3 | — | no pad rows at all in `libraries.tsv` |
| rac4 | `scePadSetActDirect` / `scePadSetActAlign` | bound, but **no-ops returning 0** (`port/game/rac4-ntsc/host/pad.c:123-135`) |

## 5. The DualSense feature by feature

### 5.1 What SDL3 exposes

The port builds against **SDL 3.2.24** when the system has none
(`port/cmake/Dependencies.cmake:59`, `GIT_TAG release-3.2.24`). Every gamepad
and HID API below is present in that version — checked against
`include/SDL3/SDL_gamepad.h` and `include/SDL3/SDL_hidapi.h` at tag
`release-3.2.24`.

| DualSense feature | SDL3 API | In 3.2.24? | What it does on a DualSense |
|---|---|---|---|
| Identify the pad | `SDL_GetGamepadType()` → **`SDL_GAMEPAD_TYPE_PS5`** | **Yes** | Note the name: SDL has never used `DUALSENSE` |
| Identify by USB ID | `SDL_GetGamepadVendor()` / `SDL_GetGamepadProduct()` | **Yes** | Sony `0x054c`, DualSense `0x0ce6`, DualSense Edge `0x0df2` |
| Touchpad click | `SDL_GAMEPAD_BUTTON_TOUCHPAD` | **Yes** | Joystick button 11 |
| Touchpad as a surface | `SDL_GetNumGamepadTouchpads()`, `SDL_GetGamepadTouchpadFinger()`, `SDL_EVENT_GAMEPAD_TOUCHPAD_*` | **Yes** | Two fingers |
| Edge paddles | `SDL_GAMEPAD_BUTTON_LEFT/RIGHT_PADDLE1/2` | **Yes** | Only on the Edge |
| Light bar | `SDL_SetGamepadLED(gp, r, g, b)` | **Yes** | The RGB strip around the top edge |
| Light-bar capability | `SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN` | **Yes** | — |
| Player LEDs | `SDL_PROP_GAMEPAD_CAP_PLAYER_LED_BOOLEAN` | **Yes** | Only via the player index; not freely settable |
| Classic rumble | `SDL_RumbleGamepad(gp, low, high, ms)` | **Yes** | The two classic motors |
| Trigger rumble | `SDL_RumbleGamepadTriggers(gp, l, r, ms)` | **Yes** | **Does nothing.** SDL's PS5 driver returns `SDL_Unsupported()`: the API is Xbox impulse triggers, and a DualSense's triggers are adaptive |
| HD haptics | — | **No** | No SDL API exists |
| **Adaptive triggers** | — | **No first-class API** | But `SDL_SendGamepadEffect(gp, data, size)` (since 3.2.0) sends the raw DS5 output report, which carries the trigger-effect slots |
| Gyroscope / accelerometer | `SDL_GamepadHasSensor()`, `SDL_SetGamepadSensorEnabled()`, `SDL_GetGamepadSensorData()`, `SDL_GetGamepadSensorDataRate()` | **Yes** | 250 Hz over USB, 1000 Hz over Bluetooth, per SDL's driver |
| Gyro calibration / reset | — | **No** | SDL reads the factory calibration; it cannot command a reset |
| Raw HID | `SDL_hid_init/open/write/send_feature_report/get_feature_report/get_device_info/...` | **Yes** | The full escape hatch. `SDL_hid_get_properties()` is 3.4.0 and **not** in 3.2.24 |
| The controller's speaker | — | **No** | It enumerates as a 4-channel USB audio device, which is an OS audio device, not a gamepad API |

**SDL's position on adaptive triggers** is on the record. The maintainer, in
2022: *"There are no plans for integrating the DualSense adaptive tiggers into
the haptics API. The SDL haptics API doesn't map well to the capabilities of
the adaptive triggers. You are welcome to build your own library on top of
SDL's DualSense raw effects support however."*
(<https://github.com/libsdl-org/SDL/issues/5125>, closed as completed.) The
request was re-opened in March 2026 and is still open
(<https://github.com/libsdl-org/SDL/issues/15288>).

### 5.2 The DS5 output report

SDL's own driver defines the 47-byte state block it sends
(`src/joystick/hidapi/SDL_hidapi_ps5.c`):

```c
typedef struct {
    Uint8 ucEnableBits1;              // 0
    Uint8 ucEnableBits2;              // 1
    Uint8 ucRumbleRight;              // 2
    Uint8 ucRumbleLeft;               // 3
    Uint8 ucHeadphoneVolume;          // 4
    Uint8 ucSpeakerVolume;            // 5
    Uint8 ucMicrophoneVolume;         // 6
    Uint8 ucAudioEnableBits;          // 7
    Uint8 ucMicLightMode;             // 8
    Uint8 ucAudioMuteBits;            // 9
    Uint8 rgucRightTriggerEffect[11]; // 10
    Uint8 rgucLeftTriggerEffect[11];  // 21
    Uint8 rgucUnknown1[6];            // 32
    Uint8 ucEnableBits3;              // 38
    Uint8 rgucUnknown2[2];            // 39
    Uint8 ucLedAnim;                  // 41
    Uint8 ucLedBrightness;            // 42
    Uint8 ucPadLights;                // 43
    Uint8 ucLedRed;                   // 44
    Uint8 ucLedGreen;                 // 45
    Uint8 ucLedBlue;                  // 46
} DS5EffectsState_t;
```

The two 11-byte arrays at offsets 10 and 21 are the **adaptive trigger effect
slots**. SDL leaves them zero; an application can fill them.

The enable bits that matter:

| Byte | Bit | Meaning |
|---|---|---|
| `ucEnableBits1` | `0x01` | enable classic rumble emulation |
| | `0x02` | **use rumble, not haptics** — SDL sets this whenever it emulates rumble, which *disables* the controller's own audio haptics |
| | `0x04` | **allow the right trigger's adaptive effect** |
| | `0x08` | **allow the left trigger's adaptive effect** |
| `ucEnableBits2` | `0x04` | allow the LED colour |
| | `0x08` | reset/release the LEDs (mandatory over Bluetooth) |
| `ucEnableBits3` | `0x04` | improved rumble emulation (firmware ≥ `0x0224`) |

**The trigger effect itself** is 11 bytes: a **mode** byte, a 10-bit **zone
bitmap** (zone 0 = released, zone 9 = fully pressed), four bytes of **3-bit
per-zone force values**, and a **frequency in Hz** at byte 9. The modes are
`0x05` off, `0x21` feedback (constant resistance), `0x25` weapon (resistance
then a release), `0x26` vibration. The ten zones are confirmed by Sony's own
SDK header's `SCE_PAD_TRIGGER_EFFECT_CONTROL_POINT_NUM 10`.

The clearest public reference for the packing is Nielk1's MIT-licensed
`TriggerEffectGenerator` gist, which SDL's own test app cites.

### 5.3 HD haptics

HD haptics are **not** in the HID report. They are a separate USB audio
stream: the DualSense exposes a 4-channel USB Audio Class endpoint
(Left, Right, Left Surround, Right Surround), 16-bit at 48 000 Hz, isochronous,
392-byte packets. The front pair feeds the speaker and the headset jack; the
surround pair feeds the two haptic actuators.

SDL3 cannot do this at all — there is no API for it, and it would need
ALSA/PipeWire or WASAPI on the controller's own audio device. Over Bluetooth
it becomes a 3 kHz, 8-bit, 10.67 ms HID report with measured ~50 ms latency.

**For a PS2 port this is a big project for a small payoff**, because the game
has no rumble to carry it.

## 6. The gotchas that matter

### 6.1 Resistance eats the trigger's travel

The trigger is a motor-driven lever. If a resistance effect is applied over
the range the game reads, the player's pull stops at the resistance and **the
analog value the game sees stops advancing**. A pull that used to read 255
reads ~120.

**Consequence for a PS2 port:** never apply resistance over the analog range
the game uses. Put it in the top zones (7–9), or use the *weapon* mode
(resistance then a release, so the axis can still reach the top), or use
*vibration* (which does not stop travel).

This also means a DualSense's adaptive triggers are **not** a drop-in for the
DualShock 2's pressure-sensitive buttons — a different thing entirely.

### 6.2 SDL reports a digital trigger press as full scale

SDL's PS5 driver contains this:

```c
if (packet->ucTriggerLeft == 0 && (packet->rgucButtonsAndHat[1] & 0x04)) {
    axis = SDL_JOYSTICK_AXIS_MAX;
}
```

— so a *digital* L2 press is reported as a full-scale axis. For a
digital-button game that is exactly right; for one that reads the trigger as
a throttle it is a lie.

### 6.3 Sending a raw effect clobbers SDL's own state

SDL's driver builds its output packet from its own pending rumble and LED
state, and `SDL_SendGamepadEffect()` copies your buffer over it. If you also
call `SDL_RumbleGamepad()` or `SDL_SetGamepadLED()`, the two fight.

**Use one combined packet**: keep a persistent 47-byte buffer, mutate the
trigger bytes in place, and send everything — rumble, LED and triggers — in a
single `SDL_SendGamepadEffect()` call.

### 6.4 The update rate is about 100 Hz, not 60

SDL's HIDAPI rumble thread does an unconditional `SDL_Delay(10)` after each
output write (*"Make sure we're not starving report reads when there's lots
of rumble"*), so SDL-driven effects are capped at roughly 100 Hz. That is
enough for a resistance profile; it is not enough for per-frame audio-rate
modulation, which needs raw HID.

### 6.5 Steam Input hides everything

With Steam Input enabled (the default for many titles), Steam's overlay
hooks XInput, DirectInput, RawInput and Windows.Gaming.Input and injects an
emulated **Xbox** controller. A game that uses the plain gamepad API then sees
a generic pad: no touchpad, no gyro, no adaptive triggers, no HD haptics, no
light bar.

The community's advice for a game that wants the advanced features is to
**disable Steam Input for that title**. On Linux, a native game also needs the
hidraw path visible.

### 6.6 Linux permissions

SDL talks to a DualSense over **hidraw** through its bundled HIDAPI driver,
not over `/dev/input/event*`. That needs a udev rule:

```
# PS5 DualSense over USB
KERNEL=="hidraw*", ATTRS{idVendor}=="054c", ATTRS{idProduct}=="0ce6", MODE="0660", TAG+="uaccess"
# PS5 DualSense over Bluetooth
KERNEL=="hidraw*", KERNELS=="*054C:0CE6*", MODE="0660", TAG+="uaccess"
```

— the rules Valve ships in `steam-devices`, and the same ones in
`pydualsense`, `dualsensectl` and `hhd`. SDL itself ships only a sample rule
with a placeholder VID/PID.

### 6.7 Bluetooth is harder

- The output report is **78 bytes** with a **CRC32** trailer, against USB's
  48 bytes with none.
- The controller must be put into **enhanced report** mode, which needs the
  serial (`0x09`) and firmware (`0x20`) feature reports read first — otherwise
  **effects are rejected entirely**.
- SDL3 cannot enable enhanced reports over Bluetooth automatically
  (<https://github.com/libsdl-org/SDL/issues/10086>); the hint
  `SDL_JOYSTICK_ENHANCED_REPORTS=1` is needed, and the default is already
  `"1"`.
- Over Bluetooth, HD haptics are a 3 kHz hack with ~50 ms latency.

**Prefer USB.**

## 7. Legal: a clean-room implementation

Sony's `isteamdualsense.h` — the header Valve copied into the Steamworks SDK —
is marked *"SIE CONFIDENTIAL ... Copyright (C) 2019 Sony Interactive
Entertainment Inc. All Rights Reserved."* It lives in `public/steam/`, **not**
in `redistributable_bin/`, so the Steamworks SDK Access Agreement does not
authorise shipping it, and it must not go into an open-source tree.

**The clean-room path is easy, because the byte map is public in several
places with permissive licences:**

| Source | Licence | What for |
|---|---|---|
| SDL3's own driver, `src/joystick/hidapi/SDL_hidapi_ps5.c` | zlib | The 47-byte struct and the wire framing |
| SDL3's test app, `test/testcontroller.c` | zlib | A blessed `SDL_SendGamepadEffect()` payload |
| The Linux kernel's `hid-playstation.c` | GPL-2.0-or-later | The byte map and the enable-bit names (read it; do not copy GPL code into a differently-licensed project) |
| Nielk1's `TriggerEffectGenerator` gist | MIT | The trigger-effect packing algorithm |
| The controllers wiki and `nondebug/dualsense` | community docs | The USB/Bluetooth descriptors and the audio endpoints |

Write your own struct with your own names, derive the packing from observed
behaviour, and you are clean.

Two further notes: "DualSense", "PlayStation" and "PS5" are Sony's
trademarks — say *"compatible with DualSense"*. And Valve's guidance on
distributing open-source games on Steam is that permissive licences (MIT,
BSD, Apache-2.0) work, while copyleft is "problematic" alongside the
Steamworks SDK.

## 8. What to build, in order

Ordered by value per unit of work, for *this* game.

| # | What | Effort | Why |
|---|---|---|---|
| 1 | **Honest analog pressure** — thread `platform::Pad::pressure` through `openrac_game_pad` into `scePad2Read`, instead of re-deriving 0/0xFF from the button bits | **Small** | The port already measures the triggers' real values (`input.cpp:245-254`) and throws them away. With this, a DualSense's L2/R2 report their true travel to the game. It also stops the game believing every held button is pressed at maximum |
| 2 | **Report the pad's type** — expose the DualSense's identity so the port can say what it is | **Small** | `SDL_GetGamepadType()`, `SDL_GetGamepadVendor()`/`Product()`. The port currently maps by position and never asks what the pad is |
| 3 | **The light bar** — `SDL_SetGamepadLED()` | **Small** | Nothing in the game drives it, so this is a **port-level** nicety: a player indicator, or a colour that reflects the game state. It needs a player-index mapping first (the port assigns ports by connection order) |
| 4 | **The touchpad click and the Edge paddles as extra buttons** | **Small** | Two spare inputs the DS2 never had. The paddles are a genuine speedrun aid: weapon and quick-select switching without leaving the thumbstick. They need the port's rebinding work (`SETTINGS.md:305-306`) to be useful |
| 5 | **The gyroscope** — `SDL_SetGamepadSensorEnabled()` / `SDL_GetGamepadSensorData()` | **Medium** | The `keyboard-mouse` mod already turns the mouse into the camera; a gyro could be an alternative for players who prefer it. The game has no use for it, so this is a port-level extra |
| 6 | **Adaptive triggers** — `SDL_SendGamepadEffect()` with a filled trigger array | **Medium** | **No fidelity payoff**: nothing in R&C1 reads a trigger's travel. It could be a *mod* — a resistance cue on the Swingshot, the Bomb Glove, the Suck Cannon — which is the honest way to offer it, since it changes how the game plays |
| 7 | **HD haptics** | **Large** | A separate USB audio path SDL cannot do. Only worth it if the port grows a rumble feature the game does not have |
| 8 | **Rumble** — for the other games | **Medium** | rac2's retail code calls `scePadSetActAlign` and rac4's `scePadSetActDirect`; the port binds neither, and rac4's are no-ops. Worth doing when rac2 and rac3 are playable |

For **Ratchet & Clank 1** specifically, items 1 and 2 are the only ones that
change anything the game can feel. Item 1 is also the one that removes a small
lie the port currently tells the game.

## 9. The speedrun angle

The controller matters to a runner for reasons that have nothing to do with
adaptive triggers.

| | |
|---|---|
| **The stick dead zone is the game's own** | The game applies its own: `d >= 0x30` with a `/0x4C` scale (`games/rac1/src/game/pad.c:346-362`). SDL3 applies **no** software dead zone to a DualSense, so the game's own is what you get. A `render_scale`/`msaa` tweak will not change it; a **rebinding and sensitivity page** would |
| **Rebinding** | `SETTINGS.md:305-306` lists it as *later*: the port's table is hard-coded in `input.cpp:20-62`. The `keyboard-mouse` mod is the workaround today, and it is the comfortable option on a PC |
| **Sticks are read as 8-bit** | `stick_byte()` converts SDL's ±32767 to the console's byte (`port/platform/pad.cpp:75-79`), so the resolution the game sees is the console's, not the pad's |
| **The touchpad and the Edge paddles are free buttons** | Two inputs the DS2 did not have. Mapped to weapon switching, they would remove a thumbstick movement from a run |
| **Extra buttons the game has no use for** | The DS5's mic button (`SDL_GAMEPAD_BUTTON_MISC1`) and the touchpad are both spare. The port's rebinding work is what makes them usable |
| **Input latency** | The port polls SDL once per tick (`input.cpp:195`), and the tick is on the wall clock (`main.cpp:781-800`). The pad's own latency is the USB or Bluetooth report interval — 250 Hz or 1000 Hz per SDL's driver |
| **The mouse is not a game control** | The game does not use it; only mods do (`docs/port/DEBUGGING.md:52`). The `keyboard-mouse` mod is how a runner uses it |

See [speedrun/PRACTICE.md](speedrun/PRACTICE.md) and
[speedrun/REFERENCE.md](speedrun/REFERENCE.md) for the rest.

## 10. Sources

**The port and the game**

- `port/platform/pad.h`, `port/platform/pad.cpp`, `port/platform/input.cpp`
- `port/game/common/lib/pad.c`, `port/game/common/frontend.cpp`,
  `port/game/common/main.cpp`
- `port/game/rac1-pal/libraries.tsv`, `port/game/rac4-ntsc/host/pad.c`
- `games/rac1/src/game/pad.c`, `games/rac1/src/game/actuator.c`,
  `games/rac1/src/core/00125218.c`, `games/rac1/include/pad.h`,
  `games/rac1/docs/kb/data-structures.md`
- `games/rac1/src/overlays/*/help_*.c` (the swimming code and its copies)
- `games/rac1/ntsc/config/us/rnc1.us.yaml` (the retail binary's linked SDK)

**SDL3**

- The 3.2.24 headers: <https://github.com/libsdl-org/SDL/tree/release-3.2.24/include/SDL3>
- The PS5 driver: <https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_ps5.c>
- The test app: <https://github.com/libsdl-org/SDL/blob/main/test/testcontroller.c>
- The rumble thread: <https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_rumble.c>
- The wiki: <https://wiki.libsdl.org/SDL3/FrontPage>
- Issue #5125 (no adaptive-trigger API): <https://github.com/libsdl-org/SDL/issues/5125>
- Issue #15288 (re-opened): <https://github.com/libsdl-org/SDL/issues/15288>
- Issue #10086 (Bluetooth enhanced reports): <https://github.com/libsdl-org/SDL/issues/10086>

**The DualSense**

- The Linux kernel driver: <https://github.com/torvalds/linux/blob/master/drivers/hid/hid-playstation.c>
- Nielk1's trigger generator (MIT): <https://gist.github.com/Nielk1/6d54cc2c00d2201ccb8c2720ad7538db>
- `nondebug/dualsense` (USB descriptors, audio endpoints):
  <https://github.com/nondebug/dualsense>
- The controllers wiki: <https://controllers.fandom.com/wiki/Sony_DualSense>
- `flok/pydualsense` (MIT), `nowrep/dualsensectl` (GPL-2.0),
  `egormanga/SAxense` (MPL-2.0), `pogojotz/hid-playstation-atrigger` (GPL-2.0)
- Steam Input gamepad emulation:
  <https://partner.steamgames.com/doc/features/steam_controller/steam_input_gamepad_emulation_bestpractices>
- The Steamworks SDK Access Agreement:
  <https://partner.steamgames.com/documentation/sdk_access_agreement/>
- The list of games with adaptive-trigger support:
  <https://www.pcgamingwiki.com/wiki/List_of_games_that_support_PlayStation_adaptive_triggers>
- Valve's udev rules: <https://github.com/ValveSoftware/steam-devices>
