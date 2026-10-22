// SPDX-License-Identifier: MIT
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac/mod.hpp against a host table of the test's own: the entry points, settings, hooks as
// lambdas chained to the original, exceptions caught at the boundary, callbacks, the stack frame.
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "openrac/games/rac1.hpp"
#include "openrac/mod.hpp"

namespace {

int g_failures = 0;
#define CHECK(c)                                                                \
    do {                                                                        \
        if (!(c)) {                                                             \
            std::fprintf(stderr, "%s:%d: CHECK(%s)\n", __FILE__, __LINE__, #c); \
            ++g_failures;                                                       \
        }                                                                       \
    } while (0)

// A hook point that returns something, for the original's result.
struct Test_Twice {
    static constexpr const char* name = "Test_Twice";
    static constexpr const char* signature = "i32(i32)";
    using fn = int32_t (*)(int32_t);
};

std::vector<std::string> g_log;
std::string g_failed;
std::map<std::string, void*> g_slots;  // hook point -> what the port calls
std::vector<float> g_game_angles;
openrac_frame_fn g_frame_fn = nullptr;
void* g_frame_user = nullptr;
openrac_pad_fn g_pad_fn = nullptr;
void* g_pad_user = nullptr;
uint8_t g_memory[256];
openrac_gaddr g_sp = 0x100;

void game_set_planar_velocity(float angle) {
    g_game_angles.push_back(angle);
}
int32_t game_twice(int32_t x) {
    return 2 * x;
}

void host_log(openrac_mod*, int level, const char* text) {
    g_log.push_back(std::to_string(level) + " " + text);
}
void host_fail(openrac_mod*, const char* reason) {
    g_failed = reason;
}
int host_setting_bool(openrac_mod*, const char* key) {
    return std::strcmp(key, "trace") == 0;
}
int64_t host_setting_int(openrac_mod*, const char* key) {
    return std::strcmp(key, "count") == 0 ? 42 : 0;
}
double host_setting_float(openrac_mod*, const char* key) {
    return std::strcmp(key, "scale") == 0 ? 1.5 : 0.0;
}
const char* host_setting_string(openrac_mod*, const char* key) {
    return std::strcmp(key, "mode") == 0 ? "lock" : "";
}
int host_hook(openrac_mod*, const char* name, const char* signature, void* replacement, void** original) {
    auto known = g_slots.find(name);
    if (known == g_slots.end()) {
        return OPENRAC_HOOK_UNKNOWN;
    }
    const std::string want = std::strcmp(name, "Test_Twice") == 0 ? "i32(i32)" : "void(f32)";
    if (want != signature) {
        return OPENRAC_HOOK_SIGNATURE;
    }
    *original = known->second;
    known->second = replacement;
    return OPENRAC_HOOK_OK;
}
int host_on_frame(openrac_mod*, openrac_frame_fn fn, void* user) {
    g_frame_fn = fn;
    g_frame_user = user;
    return 0;
}
int host_on_pad(openrac_mod*, openrac_pad_fn fn, void* user) {
    g_pad_fn = fn;
    g_pad_user = user;
    return 0;
}
void* host_function(const char* name) {
    return std::strcmp(name, "func_twice") == 0 ? reinterpret_cast<void*>(&game_twice) : nullptr;
}
void* host_function_at(openrac_gaddr) {
    return nullptr;
}
openrac_gaddr host_relocate(openrac_gaddr a) {
    return a + 0x10;
}
openrac_gaddr host_relocate_level(int level, openrac_gaddr a) {
    return a + static_cast<openrac_gaddr>(level);
}
int host_loaded_level() {
    return 3;
}
openrac_gaddr host_stack_push(uint32_t size, openrac_gaddr* saved) {
    *saved = g_sp;
    g_sp -= (size + 15u) & ~15u;
    return g_sp;
}
void host_stack_pop(openrac_gaddr saved) {
    g_sp = saved;
}

openrac_input g_input{};
uint32_t g_taken = 0;
int g_capture = -1;
const openrac_input* host_input() {
    return &g_input;
}
int host_take_input(openrac_mod*, uint32_t devices) {
    g_taken |= devices;
    return 0;
}
void host_capture_mouse(openrac_mod*, int capture) {
    g_capture = capture;
}

openrac_api make_api() {
    openrac_api api{};
    api.size = sizeof api;
    api.api_major = OPENRAC_MOD_API_MAJOR;
    api.api_minor = OPENRAC_MOD_API_MINOR;
    api.game = "rac1";
    api.version = "pal";
    api.openrac_version = "0.0.0";
    api.platform = "test";
    api.memory = g_memory;
    api.log = host_log;
    api.fail = host_fail;
    api.setting_bool = host_setting_bool;
    api.setting_int = host_setting_int;
    api.setting_float = host_setting_float;
    api.setting_string = host_setting_string;
    api.hook = host_hook;
    api.on_frame = host_on_frame;
    api.on_pad = host_on_pad;
    api.function = host_function;
    api.function_at = host_function_at;
    api.relocate_data = host_relocate;
    api.relocate_level_data = host_relocate_level;
    api.loaded_level = host_loaded_level;
    api.stack_push = host_stack_push;
    api.stack_pop = host_stack_pop;
    api.input = host_input;
    api.take_input = host_take_input;
    api.capture_mouse = host_capture_mouse;
    return api;
}

int g_frames = 0;
bool g_throw_in_hook = false;

void load(openrac::Mod& mod) {
    CHECK(mod.game() == "rac1");
    CHECK(mod.version() == "pal");
    CHECK(mod.setting<bool>("trace"));
    CHECK(mod.setting<int>("count") == 42);
    CHECK(mod.setting<int64_t>("count") == 42);
    CHECK(mod.setting<double>("scale") == 1.5);
    CHECK(mod.setting<float>("scale") == 1.5f);
    CHECK(mod.setting<std::string>("mode") == "lock");

    mod.hook<openrac::rac1::Hero_SetPlanarVelocity>([](auto original, float angle) {
        if (g_throw_in_hook) {
            throw std::runtime_error("on purpose");
        }
        original(angle + 1.0f);
    });
    const int32_t offset = 5;
    mod.hook<Test_Twice>([offset](auto original, int32_t x) { return original(x) + offset; });

    // A hook point the port lacks, or one taken twice: an Error, and nothing hooked.
    bool threw = false;
    try {
        mod.hook<openrac::rac1::Hero_TurnToHeading>(
            [](auto original, int32_t m, float a, float d, float l) { original(m, a, d, l); });
    } catch (const openrac::Error& e) {
        threw = std::string(e.what()).find("no such hook point") != std::string::npos;
    }
    CHECK(threw);
    threw = false;
    try {
        mod.hook<Test_Twice>([](auto original, int32_t x) { return original(x); });
    } catch (const openrac::Error&) {
        threw = true;
    }
    CHECK(threw);

    mod.on_frame([] {
        if (++g_frames == 2) {
            throw std::runtime_error("frame two");
        }
    });
    mod.on_pad([](int port, uint8_t* report, int size) {
        CHECK(port == 0 && size == 4);
        report[0] ^= 0xFF;
    });

    auto twice = mod.required_function<int32_t (*)(int32_t)>("func_twice");
    CHECK(twice(4) == 8);
    CHECK(mod.function<void (*)()>("nothing") == nullptr);
    CHECK(mod.relocate(0x100) == 0x110);
    CHECK(mod.relocate_level(2, 0x100) == 0x102);
    CHECK(mod.loaded_level() == 3);
    CHECK(mod.has_input());
    mod.take_input(OPENRAC_INPUT_KEYBOARD);
    mod.info("loaded");
}

}  // namespace

OPENRAC_MOD_CPP("test-cpp", "1.2.3", load)

// A hook point's type by its permanent id is the same type (games/rac1.hpp, hooks::), and asks for
// the id.
static_assert(std::is_same_v<openrac::rac1::hooks::player::set_planar_velocity, openrac::rac1::Hero_SetPlanarVelocity>);
static_assert(std::string_view(openrac::rac1::hooks::moby::bolt::update::id) == "rac1.moby.bolt.update");
static_assert(std::string_view(openrac::detail::hook_id<Test_Twice>()) == "Test_Twice");

int main() {
    g_slots[openrac::rac1::Hero_SetPlanarVelocity::id] = reinterpret_cast<void*>(&game_set_planar_velocity);
    g_slots["Test_Twice"] = reinterpret_cast<void*>(&game_twice);
    openrac_api api = make_api();
    auto* handle = reinterpret_cast<openrac_mod*>(&api);

    openrac_mod_info info{};
    info.size = sizeof info;
    openrac_mod_query(&info);
    CHECK(info.api_major == OPENRAC_MOD_API_MAJOR && info.api_minor == OPENRAC_MOD_API_MINOR);
    CHECK(std::string(info.id) == "test-cpp" && std::string(info.version) == "1.2.3");

    CHECK(openrac_mod_load(&api, handle) == 0);
    CHECK(g_failed.empty());

    // The port calls what the slot holds: the lambda, which goes on to the game's function.
    auto velocity = reinterpret_cast<void (*)(float)>(g_slots[openrac::rac1::Hero_SetPlanarVelocity::id]);
    velocity(1.0f);
    CHECK(g_game_angles.size() == 1 && g_game_angles[0] == 2.0f);
    auto twice = reinterpret_cast<int32_t (*)(int32_t)>(g_slots["Test_Twice"]);
    CHECK(twice(10) == 25);
    CHECK(openrac::detail::instance()->original<Test_Twice>() == &game_twice);

    // An exception in the hook stops at the boundary: logged with where, and the original runs.
    g_throw_in_hook = true;
    velocity(7.0f);
    CHECK(g_game_angles.size() == 2 && g_game_angles[1] == 7.0f);
    CHECK(!g_log.empty() && g_log.back().find("Hero_SetPlanarVelocity: on purpose") != std::string::npos);
    CHECK(g_log.back().rfind("3 ", 0) == 0);  // OPENRAC_LOG_ERROR

    // Callbacks: an exception in one is logged and returns.
    g_frame_fn(g_frame_user);
    g_frame_fn(g_frame_user);
    CHECK(g_frames == 2);
    CHECK(g_log.back().find("on_frame: frame two") != std::string::npos);
    uint8_t report[4] = {0x0F, 0, 0, 0};
    g_pad_fn(g_pad_user, 0, report, 4);
    CHECK(report[0] == 0xF0);

    // Memory and the stack frame.
    openrac::Mod& mod = *openrac::detail::instance();
    mod.memory().write<float>(0x21, 3.5f);  // unaligned on purpose
    CHECK(mod.memory().read<float>(0x21) == 3.5f);
    mod.memory().ref<uint32_t>(0x40) = 0xDEADBEEFu;
    CHECK(mod.memory().read<uint32_t>(0x40) == 0xDEADBEEFu);
    {
        openrac::StackFrame frame(mod, 20);
        CHECK(frame.address() == 0x100 - 32);
        CHECK(g_sp == 0x100 - 32);
    }
    CHECK(g_sp == 0x100);

    // The keyboard and mouse (1.2): binds by name, held and pressed, the wheel, the capture.
    CHECK(g_taken == OPENRAC_INPUT_KEYBOARD);
    CHECK(openrac::bind_from_name("KeyW") == 26 && openrac::bind_name(26) == "KeyW");
    CHECK(openrac::bind_from_name("MouseRight") == OPENRAC_BIND_MOUSE_RIGHT);
    CHECK(openrac::bind_from_name("WheelDown") == OPENRAC_BIND_WHEEL_DOWN);
    CHECK(openrac::bind_from_name("NoSuchKey") == OPENRAC_BIND_NONE);
    CHECK(openrac::bind_from_name(std::string_view("Space,KeyW").substr(0, 5)) == 44);
    for (int i = 0; i < OPENRAC_BIND_NAME_COUNT; ++i) {  // every name goes there and back
        CHECK(openrac::bind_from_name(openrac_bind_names[i].name) == openrac_bind_names[i].bind);
    }
    g_input.keys[26] = OPENRAC_KEY_HELD | OPENRAC_KEY_PRESSED;
    g_input.keys[44] = OPENRAC_KEY_PRESSED;  // tapped between two ticks
    g_input.mouse_held = OPENRAC_MOUSE_LEFT;
    g_input.mouse_pressed = OPENRAC_MOUSE_RIGHT;
    g_input.wheel_y = -1.0f;
    g_input.mouse_dx = 3.0f;
    g_input.size = sizeof g_input;
    g_input.pointer_x = 0.25f;
    g_input.pointer_y = 0.5f;
    g_input.pointer_inside = 1;
    const openrac::Input in = mod.input();
    CHECK(in.has_pointer() && in.pointer_x() == 0.25f && in.pointer_y() == 0.5f && in.pointer_inside());
    {
        openrac_input old = g_input;  // a port of 1.2: the struct ends before the pointer
        old.size = static_cast<uint32_t>(offsetof(openrac_input, pointer_x));
        const openrac::Input before(&old);
        CHECK(!before.has_pointer() && before.pointer_x() == 0.0f && !before.pointer_inside());
    }
    CHECK(in.held(26) && in.pressed(26) && !in.held(44) && in.pressed(44) && !in.held(4));
    CHECK(in.held(OPENRAC_BIND_MOUSE_LEFT) && !in.pressed(OPENRAC_BIND_MOUSE_LEFT));
    CHECK(!in.held(OPENRAC_BIND_MOUSE_RIGHT) && in.pressed(OPENRAC_BIND_MOUSE_RIGHT));
    CHECK(in.pressed(OPENRAC_BIND_WHEEL_DOWN) && !in.pressed(OPENRAC_BIND_WHEEL_UP));
    CHECK(in.mouse_dx() == 3.0f && !in.held(OPENRAC_BIND_NONE));
    mod.capture_mouse(true);
    CHECK(g_capture == 1);
    mod.capture_mouse(false);
    CHECK(g_capture == 0);

    // A load that throws refuses the mod with the message.
    g_failed.clear();
    CHECK(openrac::detail::load(&api, handle, [](openrac::Mod&) { throw openrac::Error("not for this disc"); }) == 1);
    CHECK(g_failed == "not for this disc");
    g_failed.clear();
    CHECK(openrac::detail::load(&api, handle, [](openrac::Mod&) { throw 7; }) == 1);
    CHECK(!g_failed.empty());

    // A later load of the same library starts afresh: the hook point can be taken again.
    g_slots["Test_Twice"] = reinterpret_cast<void*>(&game_twice);
    g_failed.clear();
    CHECK(openrac::detail::load(&api, handle, [](openrac::Mod& m) {
              m.hook<Test_Twice>([](auto original, int32_t x) { return original(x) + 100; });
          }) == 0);
    CHECK(g_failed.empty());
    twice = reinterpret_cast<int32_t (*)(int32_t)>(g_slots["Test_Twice"]);
    CHECK(twice(1) == 102);

    if (g_failures == 0) {
        std::puts("mod_hpp: all passed");
    }
    return g_failures == 0 ? 0 : 1;
}
