// SPDX-License-Identifier: MIT
// Copyright (c) 2026 the OpenRAC contributors
//
// The C++ layer of the mod interface (docs/port/MODS.md, section 11): header-only, C++17, compiled
// into the mod and never into the port. Everything here goes through openrac_mod.h's table, which
// stays the boundary; no exception crosses it.
//
//   #include "openrac/mod.hpp"
//   #include "openrac/games/rac1.hpp"
//
//   static void load(openrac::Mod& mod) {
//       if (mod.game() != "rac1") throw openrac::Error("this mod is for Ratchet & Clank");
//       const bool trace = mod.setting<bool>("trace");
//       mod.hook<openrac::rac1::Hero_TurnToHeading>(
//           [](auto original, int32_t m, float accel, float damping, float limit) {
//               original(m, accel, damping, limit);   // the game's own, or a mod's loaded earlier
//           });
//       mod.on_frame([] { /* once a game tick */ });
//   }
//   OPENRAC_MOD_CPP("my-mod", "0.1.0", load)
//
// One library is one mod: the layer keeps one openrac::Mod per library, and each hook point's
// callable in a static of its own (a load takes a hook point once; a later load of the same
// library starts afresh). An exception out of the
// load function becomes the mod's refusal (fail() with its message); one out of a hook or a
// callback is logged with the mod's id, and a hook then calls its original, so the game goes on
// (a hook that threw after calling the original has it called twice: do not throw there).
#ifndef OPENRAC_MOD_HPP
#define OPENRAC_MOD_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "openrac/keys.h"
#include "openrac/openrac_mod.h"

// Nothing of the layer is seen outside the mod's library, whatever the build's visibility: two mods
// that both use it never share its statics.
#if defined(_WIN32)
#define OPENRAC_MOD_HIDDEN
#else
#define OPENRAC_MOD_HIDDEN __attribute__((visibility("hidden")))
#endif

namespace openrac OPENRAC_MOD_HIDDEN {

using gaddr = openrac_gaddr;

// What load and the layer throw: its message is the mod's refusal, or the log's line.
class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class LogLevel : int {
    Debug = OPENRAC_LOG_DEBUG,
    Info = OPENRAC_LOG_INFO,
    Warn = OPENRAC_LOG_WARN,
    Error = OPENRAC_LOG_ERROR,
};

// Game memory: typed reads and writes at game addresses. Reads and writes copy (any alignment);
// ref() gives an lvalue where the address is aligned for T.
class Memory {
public:
    explicit Memory(std::uint8_t* base) : m_base(base) {}

    template <class T>
    T read(gaddr address) const {
        static_assert(std::is_trivially_copyable_v<T>, "game memory holds plain values");
        T value;
        std::memcpy(&value, m_base + address, sizeof value);
        return value;
    }

    template <class T>
    void write(gaddr address, const T& value) const {
        static_assert(std::is_trivially_copyable_v<T>, "game memory holds plain values");
        std::memcpy(m_base + address, &value, sizeof value);
    }

    template <class T>
    T& ref(gaddr address) const {
        return *reinterpret_cast<T*>(m_base + address);
    }

    void* pointer(gaddr address) const { return m_base + address; }

    std::uint8_t* base() const { return m_base; }

private:
    std::uint8_t* m_base;
};

// A bind (openrac/keys.h): a key, a mouse button or a wheel direction, by its name ("KeyW",
// "MouseLeft", "WheelUp"); bind_from_name gives OPENRAC_BIND_NONE (0) for a name of no bind.
inline int bind_from_name(std::string_view name) {
    return openrac_bind_from_name_n(name.data(), name.size());
}

inline std::string_view bind_name(int bind) {
    return openrac_bind_to_name(bind);
}

// The keyboard and mouse for the tick about to run (interface 1.2, openrac_input): a view of the
// port's state, renewed each tick.
class Input {
public:
    explicit Input(const openrac_input* state) : m_state(state) {}

    const openrac_input& raw() const { return *m_state; }

    // A bind (a key's HID usage, OPENRAC_BIND_*): held now; went down since the last tick.
    bool held(int bind) const { return openrac_bind_held(m_state, bind) != 0; }

    bool pressed(int bind) const { return openrac_bind_pressed(m_state, bind) != 0; }

    float mouse_dx() const { return m_state->mouse_dx; }

    float mouse_dy() const { return m_state->mouse_dy; }

    float wheel_x() const { return m_state->wheel_x; }

    float wheel_y() const { return m_state->wheel_y; }

    bool focused() const { return m_state->focused != 0; }

    bool captured() const { return m_state->captured != 0; }

    bool overlay() const { return m_state->overlay != 0; }

    std::uint32_t tick() const { return m_state->tick; }

    // Interface 1.3: the cursor over the game's picture, 0..1 (pointer_x() * 512, pointer_y() *
    // 416 is the game's 2D pixel); has_pointer() false on an older port.
    bool has_pointer() const { return m_state->size >= offsetof(openrac_input, reserved2); }

    float pointer_x() const { return has_pointer() ? m_state->pointer_x : 0.0f; }

    float pointer_y() const { return has_pointer() ? m_state->pointer_y : 0.0f; }

    bool pointer_inside() const { return has_pointer() && m_state->pointer_inside != 0; }

private:
    const openrac_input* m_state;
};

// A hook's way on: the function the replacement goes on to (the game's own, or the replacement of
// a mod loaded earlier), called like the hook point.
template <class Signature>
class Original;

template <class R, class... A>
class Original<R(A...)> {
public:
    using fn = R (*)(A...);

    explicit Original(fn f) : m_fn(f) {}

    R operator()(A... args) const { return m_fn(args...); }

    fn get() const { return m_fn; }

private:
    fn m_fn;
};

class Mod;

namespace detail {

// A hook point type's id, when it declares one (static constexpr const char* id), else its name.
template <class T, class = void>
struct HasId : std::false_type {};

template <class T>
struct HasId<T, std::void_t<decltype(T::id)>> : std::true_type {};

template <class T>
constexpr const char* hook_id() {
    if constexpr (HasId<T>::value) {
        return T::id;
    } else {
        return T::name;
    }
}

// The library's one mod, made at load and never destroyed: the port may call a hook or a callback
// while the program ends, after the library's statics would have gone.
inline Mod*& instance() {
    static Mod* mod = nullptr;
    return mod;
}

inline void report(const char* where, const char* what);

// What a load took, to be let go of before the library's next load (a load that the port called
// again on a library it opened again before the system had unloaded it, or a test's).
inline std::deque<void (*)()>& taken() {
    static auto* list = new std::deque<void (*)()>();
    return *list;
}

// Each hook point's replacement and original, one set per library.
template <class HookPoint, class Fn = typename HookPoint::fn>
struct Hook;

template <class HookPoint, class R, class... A>
struct Hook<HookPoint, R (*)(A...)> {
    using Callable = std::function<R(Original<R(A...)>, A...)>;
    using Fp = R (*)(A...);

    static Callable*& callable() {
        static Callable* c = nullptr;
        return c;
    }

    static Fp& original() {
        static Fp o = nullptr;
        return o;
    }

    static void release() {
        delete callable();
        callable() = nullptr;
        original() = nullptr;
    }

    static R thunk(A... args) {
        const auto go_on = original();

        try {
            return (*callable())(Original<R(A...)>(go_on), args...);
        } catch (const std::exception& e) {
            report(HookPoint::name, e.what());
        } catch (...) {
            report(HookPoint::name, "an exception that is not a std::exception");
        }

        return go_on(args...);
    }
};

using FrameCallable = std::function<void()>;
using PadCallable = std::function<void(int port, std::uint8_t* report, int size)>;

inline void frame_thunk(void* user) {
    try {
        (*static_cast<FrameCallable*>(user))();
    } catch (const std::exception& e) {
        report("on_frame", e.what());
    } catch (...) {
        report("on_frame", "an exception that is not a std::exception");
    }
}

inline void pad_thunk(void* user, int port, std::uint8_t* report_bytes, int size) {
    try {
        (*static_cast<PadCallable*>(user))(port, report_bytes, size);
    } catch (const std::exception& e) {
        report("on_pad", e.what());
    } catch (...) {
        report("on_pad", "an exception that is not a std::exception");
    }
}

}  // namespace detail

// The mod as the port loaded it: the table, the handle, and what the mod takes of the game.
class Mod {
public:
    Mod(const openrac_api* api, openrac_mod* handle) : m_api(api), m_handle(handle) {}

    Mod(const Mod&) = delete;
    Mod& operator=(const Mod&) = delete;

    const openrac_api& api() const { return *m_api; }

    openrac_mod* handle() const { return m_handle; }

    std::string_view game() const { return m_api->game; }

    std::string_view version() const { return m_api->version; }

    std::string_view platform() const { return m_api->platform; }

    std::string_view openrac_version() const { return m_api->openrac_version; }

    // The port's interface version: at least what the mod was built for, else it is not loaded.
    unsigned api_major() const { return m_api->api_major; }

    unsigned api_minor() const { return m_api->api_minor; }

    void log(LogLevel level, const std::string& text) const {
        m_api->log(m_handle, static_cast<int>(level), text.c_str());
    }

    void debug(const std::string& text) const { log(LogLevel::Debug, text); }

    void info(const std::string& text) const { log(LogLevel::Info, text); }

    void warn(const std::string& text) const { log(LogLevel::Warn, text); }

    void error(const std::string& text) const { log(LogLevel::Error, text); }

    // During load: why the mod refuses (throwing an openrac::Error from load does the same).
    void fail(const std::string& reason) const { m_api->fail(m_handle, reason.c_str()); }

    // A setting of the manifest's, as T: bool for "bool"; an integer type for "int"; float or
    // double for "float"; std::string for "choice" and "string".
    template <class T>
    T setting(const char* key) const {
        if constexpr (std::is_same_v<T, bool>) {
            return m_api->setting_bool(m_handle, key) != 0;
        } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(m_api->setting_int(m_handle, key));
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(m_api->setting_float(m_handle, key));
        } else if constexpr (std::is_same_v<T, std::string>) {
            const char* text = m_api->setting_string(m_handle, key);
            return text != nullptr ? std::string(text) : std::string();
        } else {
            static_assert(
                sizeof(T) == 0, "setting<T>: bool, an integer, a floating type or std::string"
            );
        }
    }

    // Takes the hook point (a type of openrac/games/<game>.hpp) for `callable`, called as
    // callable(original, args...). Throws openrac::Error when the port has no such hook point, a
    // different signature, or the mod already took it. Only during load.
    template <class HookPoint, class F>
    void hook(F&& callable) {
        using H = detail::Hook<HookPoint>;
        // By its permanent id where the type has one (the generated games/<game>.hpp do), which
        // lasts through renames of the game's functions; else by its name (docs/port/MODS.md,
        // section 4).
        const char* const asked = detail::hook_id<HookPoint>();

        if (H::callable() != nullptr) {
            throw Error(std::string(HookPoint::name) + ": this mod has already taken it");
        }

        H::callable() = new typename H::Callable(std::forward<F>(callable));
        void* original = nullptr;
        const int result = m_api->hook(
            m_handle, asked, HookPoint::signature, reinterpret_cast<void*>(&H::thunk), &original
        );

        if (result != OPENRAC_HOOK_OK) {
            delete H::callable();
            H::callable() = nullptr;
            throw Error(std::string(HookPoint::name) + ": " + hook_error(result));
        }

        H::original() = reinterpret_cast<typename H::Fp>(original);
        detail::taken().push_back(&H::release);
    }

    // What a hook point this mod took goes on to (as the callable's `original`), for calling it
    // from elsewhere in the mod (another hook, a callback); nullptr before the hook is taken.
    template <class HookPoint>
    typename HookPoint::fn original() const {
        return detail::Hook<HookPoint>::original();
    }

    // callable() after each game tick, in load order. Only during load.
    template <class F>
    void on_frame(F&& callable) {
        auto& stored = m_frames.emplace_back(std::forward<F>(callable));

        if (m_api->on_frame(m_handle, &detail::frame_thunk, &stored) != 0) {
            throw Error("on_frame: refused (only during load)");
        }
    }

    // callable(port, report, size) for each pad report, before the game reads it. Only during load.
    template <class F>
    void on_pad(F&& callable) {
        auto& stored = m_pads.emplace_back(std::forward<F>(callable));

        if (m_api->on_pad(m_handle, &detail::pad_thunk, &stored) != 0) {
            throw Error("on_pad: refused (only during load)");
        }
    }

    // A game function by its name in this disc's build, as Fn (a function pointer type); nullptr
    // when there is none. required_function throws instead.
    template <class Fn>
    Fn function(const char* name) const {
        static_assert(
            std::is_pointer_v<Fn> && std::is_function_v<std::remove_pointer_t<Fn>>,
            "function<Fn>: Fn is a function pointer type"
        );
        return reinterpret_cast<Fn>(m_api->function(name));
    }

    template <class Fn>
    Fn required_function(const char* name) const {
        Fn f = function<Fn>(name);

        if (f == nullptr) {
            throw Error(std::string("this disc's build has no function ") + name);
        }

        return f;
    }

    // The function at a code address of the level loaded now, as Fn; nullptr when there is none.
    template <class Fn>
    Fn function_at(gaddr address) const {
        return reinterpret_cast<Fn>(m_api->function_at(address));
    }

    // Interface 1.2: the keyboard and mouse. has_input() says whether the port offers them (a
    // mod built for 1.2 is loaded only by a port that does).
    bool has_input() const {
        return m_api->api_minor >= 2
               && m_api->size >= offsetof(openrac_api, capture_mouse) + sizeof(void*);
    }

    // The state for the tick about to run.
    Input input() const { return Input(m_api->input()); }

    // Takes the keyboard (OPENRAC_INPUT_KEYBOARD) and or the mouse from the port's own use of them
    // in play: the mod presses pad 0's buttons through on_pad. Only during load.
    void take_input(std::uint32_t devices) {
        if (m_api->take_input(m_handle, devices) != 0) {
            throw Error("take_input: refused (only during load)");
        }
    }

    // Asks for the cursor captured (hidden, motion relative) or let go; any time.
    void capture_mouse(bool capture) const { m_api->capture_mouse(m_handle, capture ? 1 : 0); }

    Memory memory() const { return Memory(m_api->memory); }

    gaddr relocate(gaddr address) const { return m_api->relocate_data(address); }

    gaddr relocate_level(int level, gaddr address) const {
        return m_api->relocate_level_data(level, address);
    }

    int loaded_level() const { return m_api->loaded_level(); }

private:
    static const char* hook_error(int result) {
        switch (result) {
            case OPENRAC_HOOK_UNKNOWN:
                return "this OpenRAC has no such hook point on this disc";
            case OPENRAC_HOOK_SIGNATURE:
                return "the hook point's signature is another";
            case OPENRAC_HOOK_TOO_LATE:
                return "hooks are taken during load only";
            default:
                return "refused";
        }
    }

    const openrac_api* m_api;
    openrac_mod* m_handle;
    // Stable addresses: the port holds pointers to these.
    std::deque<detail::FrameCallable> m_frames;
    std::deque<detail::PadCallable> m_pads;
};

// Room on the game's stack for a call that takes the address of a local, given back when it goes.
class StackFrame {
public:
    StackFrame(const Mod& mod, std::uint32_t size) : m_api(&mod.api()) {
        m_address = m_api->stack_push(size, &m_saved);
    }

    ~StackFrame() { m_api->stack_pop(m_saved); }

    StackFrame(const StackFrame&) = delete;
    StackFrame& operator=(const StackFrame&) = delete;

    gaddr address() const { return m_address; }

private:
    const openrac_api* m_api;
    gaddr m_saved = 0;
    gaddr m_address = 0;
};

namespace detail {

inline void report(const char* where, const char* what) {
    if (Mod* mod = instance()) {
        mod->error(std::string("exception in ") + where + ": " + what);
    }
}

template <class Load>
int load(const openrac_api* api, openrac_mod* handle, Load&& load_fn) {
    for (auto release : taken()) {
        release();
    }

    taken().clear();
    Mod* mod = nullptr;

    try {
        mod = new Mod(api, handle);
        instance() = mod;
        load_fn(*mod);
        return 0;
    } catch (const std::exception& e) {
        api->fail(handle, e.what());
    } catch (...) {
        api->fail(handle, "the mod's load threw an exception that is not a std::exception");
    }

    return 1;
}

}  // namespace detail

}  // namespace openrac OPENRAC_MOD_HIDDEN

// The two entry points of a C++ mod: openrac_mod_query (the id and version, as its manifest's) and
// openrac_mod_load, which calls load_fn(openrac::Mod&). A throw out of load_fn refuses the load with
// its message.
#define OPENRAC_MOD_CPP(mod_id, mod_version, load_fn)                                              \
    extern "C" OPENRAC_MOD_EXPORT void openrac_mod_query(openrac_mod_info* info) {                 \
        info->api_major = OPENRAC_MOD_API_MAJOR;                                                   \
        info->api_minor = OPENRAC_MOD_TARGET_MINOR;                                                \
        info->id = (mod_id);                                                                       \
        info->version = (mod_version);                                                             \
    }                                                                                              \
    extern "C" OPENRAC_MOD_EXPORT int openrac_mod_load(const openrac_api* api, openrac_mod* mod) { \
        return ::openrac::detail::load(api, mod, load_fn);                                         \
    }

#endif  // OPENRAC_MOD_HPP
