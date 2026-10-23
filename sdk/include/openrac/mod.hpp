// SPDX-License-Identifier: MIT
// Copyright (c) 2026 the OpenRAC contributors
//
// The C++ layer of the mod interface (docs/port/MODS.md, section 11): header-only, C++17, compiled
// into the mod and never into the port. Everything here goes through openrac_mod.h's table, which
// stays the boundary; no exception crosses it.
//
// The header is laid out as: the includes, the error and log types, game memory, binds and the
// input state, the hook machinery (`detail`), the Mod itself, the stack frame, and last the macro
// that writes a mod's entry points. Each item says which interface version added it; the layer
// itself needs only the table of the interface the mod was built for.
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

// Keeps the layer's symbols out of the mod's exports: nothing of the layer is seen outside the mod's library, whatever the build's visibility: two mods
// that both use it never share its statics.
#if defined(_WIN32)
#define OPENRAC_MOD_HIDDEN
#else
#define OPENRAC_MOD_HIDDEN __attribute__((visibility("hidden")))
#endif

namespace openrac OPENRAC_MOD_HIDDEN {

// ----------------------------------------------------------------------------------------------
// Errors, log levels and game memory
// ----------------------------------------------------------------------------------------------

// A game address, as openrac_gaddr. Since 1.0.
using gaddr = openrac_gaddr;

// What load and the layer throw: its message is the mod's refusal, or the log's line. Since 1.0.
class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// The levels of Mod::log, from the most to the least talkative: the OPENRAC_LOG_* values. Since 1.0.
enum class LogLevel : int {
    Debug = OPENRAC_LOG_DEBUG,
    Info = OPENRAC_LOG_INFO,
    Warn = OPENRAC_LOG_WARN,
    Error = OPENRAC_LOG_ERROR,
};

// Game memory: typed reads and writes at game addresses. Reads and writes copy (any alignment);
// ref() gives an lvalue where the address is aligned for T. A view: it owns nothing, and the
// memory it views lasts for the life of the program. Since 1.0.
class Memory {
public:
    // Views the memory that begins at `base` (openrac_api.memory).
    explicit Memory(std::uint8_t* base) : m_base(base) {}

    // Returns the `T` at `address`, copied out; `T` is a plain value (trivially copyable).
    template <class T>
    T read(gaddr address) const {
        static_assert(std::is_trivially_copyable_v<T>, "game memory holds plain values");
        T value;
        std::memcpy(&value, m_base + address, sizeof value);
        return value;
    }

    // Copies `value` to `address`; `T` is a plain value (trivially copyable). It does not mark
    // anything the game caches: a value the game keeps elsewhere is the mod's to update.
    template <class T>
    void write(gaddr address, const T& value) const {
        static_assert(std::is_trivially_copyable_v<T>, "game memory holds plain values");
        std::memcpy(m_base + address, &value, sizeof value);
    }

    // Returns the `T` at `address` as an lvalue. The address must be aligned for `T`, and the
    // reference must not be kept across a level load (the level's program moves its globals).
    template <class T>
    T& ref(gaddr address) const {
        return *reinterpret_cast<T*>(m_base + address);
    }

    // Returns the host pointer to `address`.
    void* pointer(gaddr address) const { return m_base + address; }

    // Returns the host pointer to game address 0.
    std::uint8_t* base() const { return m_base; }

private:
    // The base of game memory: game address a is m_base + a. Not owned.
    std::uint8_t* m_base;
};

// ----------------------------------------------------------------------------------------------
// Binds and the input state (since 1.2)
// ----------------------------------------------------------------------------------------------

// Returns the bind (openrac/keys.h) a name is: a key, a mouse button or a wheel direction, by its
// name ("KeyW", "MouseLeft", "WheelUp"); OPENRAC_BIND_NONE (0) for a name of no bind. Since 1.2.
inline int bind_from_name(std::string_view name) {
    return openrac_bind_from_name_n(name.data(), name.size());
}

// Returns a bind's name, or an empty view for a bind of none. Since 1.2.
inline std::string_view bind_name(int bind) {
    return openrac_bind_to_name(bind);
}

// The keyboard and mouse for the tick about to run (interface 1.2, openrac_input): a view of the
// port's state, renewed each tick. Cheap to copy; do not keep it past the tick it was read in.
class Input {
public:
    // Views the port's `state` (openrac_api.input()'s result).
    explicit Input(const openrac_input* state) : m_state(state) {}

    // Returns the C state, for what the accessors do not cover.
    const openrac_input& raw() const { return *m_state; }

    // Returns whether a bind (a key's HID usage, OPENRAC_BIND_*) is held now.
    bool held(int bind) const { return openrac_bind_held(m_state, bind) != 0; }

    // Returns whether a bind went down since the last tick.
    bool pressed(int bind) const { return openrac_bind_pressed(m_state, bind) != 0; }

    // Return the mouse's motion since the last tick, in the system's pixels (raw while captured).
    float mouse_dx() const { return m_state->mouse_dx; }

    float mouse_dy() const { return m_state->mouse_dy; }

    // Return the wheel's turn since the last tick, in notches (x > 0 right, y > 0 up).
    float wheel_x() const { return m_state->wheel_x; }

    float wheel_y() const { return m_state->wheel_y; }

    // Return whether the game's window has the keyboard, whether the cursor is captured, and
    // whether the port's own overlay has the input now (leave it alone then).
    bool focused() const { return m_state->focused != 0; }

    bool captured() const { return m_state->captured != 0; }

    bool overlay() const { return m_state->overlay != 0; }

    // Returns the tick counter: a new value is a new state.
    std::uint32_t tick() const { return m_state->tick; }

    // Since 1.3: returns whether the port's state reaches the pointer fields. False on a 1.2 port,
    // whose pointer_x(), pointer_y() and pointer_inside() then give 0, 0 and false.
    bool has_pointer() const { return m_state->size >= offsetof(openrac_input, reserved2); }

    // Since 1.3: return where the cursor is over the game's picture, 0..1 across and down
    // (pointer_x() * 512, pointer_y() * 416 is the game's 2D pixel), and whether it is over it.
    float pointer_x() const { return has_pointer() ? m_state->pointer_x : 0.0f; }

    float pointer_y() const { return has_pointer() ? m_state->pointer_y : 0.0f; }

    bool pointer_inside() const { return has_pointer() && m_state->pointer_inside != 0; }

private:
    // The port's state; not owned.
    const openrac_input* m_state;
};

// ----------------------------------------------------------------------------------------------
// Hooks: what the layer keeps for each hook point, and its thunks
// ----------------------------------------------------------------------------------------------

// A hook's way on: the function the replacement goes on to (the game's own, or the replacement of
// a mod loaded earlier), called like the hook point. Since 1.0.
template <class Signature>
class Original;

template <class R, class... A>
class Original<R(A...)> {
public:
    using fn = R (*)(A...);

    // Wraps the function `f` that the replacement goes on to.
    explicit Original(fn f) : m_fn(f) {}

    // Calls the original with `args`, as the game would have called the hook point.
    R operator()(A... args) const { return m_fn(args...); }

    // Returns the original as a plain function pointer.
    fn get() const { return m_fn; }

private:
    // What the replacement goes on to.
    fn m_fn;
};

class Mod;

namespace detail {

// Tells whether a hook point type declares an id (`static constexpr const char* id`).
template <class T, class = void>
struct HasId : std::false_type {};

template <class T>
struct HasId<T, std::void_t<decltype(T::id)>> : std::true_type {};

// Returns a hook point type's permanent id when it declares one, else its name.
template <class T>
constexpr const char* hook_id() {
    // The generated hook point types have an id that outlasts renames of the game's functions; a
    // hand-written one (a test's) has only a name.
    if constexpr (HasId<T>::value) {
        return T::id;
    } else {
        return T::name;
    }
}

// Returns the library's one mod (nullptr before load): made at load and never destroyed, because
// the port may call a hook or a callback while the program ends, after the library's statics
// would have gone. Only the game's thread touches it.
inline Mod*& instance() {
    static Mod* mod = nullptr;
    return mod;
}

// Logs an exception that a hook or a callback let out, under the mod's id; defined after Mod.
inline void report(const char* where, const char* what);

// Returns the list of what a load took (one release function per hook point), to be let go of
// before the library's next load: a load that the port called again on a library it opened again
// before the system had unloaded it, or a test's. Never destroyed, for the reason of instance().
inline std::deque<void (*)()>& taken() {
    static auto* list = new std::deque<void (*)()>();
    return *list;
}

// Each hook point's replacement and original, one set per library: the statics of the specialisation
// below are the state, and the game's thread is the only one that touches them. Since 1.0.
template <class HookPoint, class Fn = typename HookPoint::fn>
struct Hook;

template <class HookPoint, class R, class... A>
struct Hook<HookPoint, R (*)(A...)> {
    // What a mod's replacement is: called with the way on, then the hook point's arguments.
    using Callable = std::function<R(Original<R(A...)>, A...)>;
    // The hook point's signature as a function pointer.
    using Fp = R (*)(A...);

    // Returns the slot of the mod's replacement; null until Mod::hook takes the point. Owned: the
    // slot is deleted by release().
    static Callable*& callable() {
        static Callable* c = nullptr;
        return c;
    }

    // Returns the slot of what the replacement goes on to; null until the point is taken.
    static Fp& original() {
        static Fp o = nullptr;
        return o;
    }

    // Lets go of the replacement and forgets the original, so the next load starts afresh.
    static void release() {
        delete callable();
        callable() = nullptr;
        original() = nullptr;
    }

    // The function the port calls in the hook point's place: runs the mod's replacement and, when
    // that throws, calls the original, so the game goes on.
    static R thunk(A... args) {
        const auto go_on = original();

        // An exception must not cross into the game's frames: log it, then fall through to the
        // game's own function.
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

// What a mod's on_frame callback is.
using FrameCallable = std::function<void()>;

// What a mod's on_pad callback is.
using PadCallable = std::function<void(int port, std::uint8_t* report, int size)>;

// The openrac_frame_fn the port calls: runs the FrameCallable that `user` points at, and logs what
// it throws instead of letting it into the game's frame loop.
inline void frame_thunk(void* user) {
    // An exception must not cross into the game's frame loop.
    try {
        (*static_cast<FrameCallable*>(user))();
    } catch (const std::exception& e) {
        report("on_frame", e.what());
    } catch (...) {
        report("on_frame", "an exception that is not a std::exception");
    }
}

// The openrac_pad_fn the port calls: runs the PadCallable that `user` points at, and logs what it
// throws instead of letting it into the game's pad reading.
inline void pad_thunk(void* user, int port, std::uint8_t* report_bytes, int size) {
    // An exception must not cross into the game's pad reading.
    try {
        (*static_cast<PadCallable*>(user))(port, report_bytes, size);
    } catch (const std::exception& e) {
        report("on_pad", e.what());
    } catch (...) {
        report("on_pad", "an exception that is not a std::exception");
    }
}

}  // namespace detail

// ----------------------------------------------------------------------------------------------
// The mod
// ----------------------------------------------------------------------------------------------

// The mod as the port loaded it: the table, the handle, and what the mod takes of the game. One per
// library, made by OPENRAC_MOD_CPP at load; the game's thread is the only one that uses it.
// Not copyable: the port holds pointers into the callbacks it keeps. Since 1.0.
class Mod {
public:
    // Wraps the port's table and this mod's handle, as openrac_mod_load received them.
    Mod(const openrac_api* api, openrac_mod* handle) : m_api(api), m_handle(handle) {}

    Mod(const Mod&) = delete;
    Mod& operator=(const Mod&) = delete;

    // Returns the C table, for what the layer does not wrap.
    const openrac_api& api() const { return *m_api; }

    // Returns the port's handle for this mod, the first argument of the table's own functions.
    openrac_mod* handle() const { return m_handle; }

    // Return where the mod runs: the game's id ("rac1"), the disc's version ("pal", "ntsc"), the
    // platform ("macos-arm64", ...) and OpenRAC's version. Valid for the life of the program.
    std::string_view game() const { return m_api->game; }

    std::string_view version() const { return m_api->version; }

    std::string_view platform() const { return m_api->platform; }

    std::string_view openrac_version() const { return m_api->openrac_version; }

    // Return the port's interface version: at least what the mod was built for, else it is not
    // loaded. Check api_minor() before using anything newer than the minor the mod was built for.
    unsigned api_major() const { return m_api->api_major; }

    unsigned api_minor() const { return m_api->api_minor; }

    // Writes `text` to the session's log at `level`, after the mod's id.
    void log(LogLevel level, const std::string& text) const {
        m_api->log(m_handle, static_cast<int>(level), text.c_str());
    }

    // Write `text` to the log at the level the name says.
    void debug(const std::string& text) const { log(LogLevel::Debug, text); }

    void info(const std::string& text) const { log(LogLevel::Info, text); }

    void warn(const std::string& text) const { log(LogLevel::Warn, text); }

    void error(const std::string& text) const { log(LogLevel::Error, text); }

    // Says why the mod refuses to load, for status.json and the launcher. Only during load;
    // throwing an openrac::Error from load does the same.
    void fail(const std::string& reason) const { m_api->fail(m_handle, reason.c_str()); }

    // Returns a setting of the manifest's, as T: bool for "bool"; an integer type for "int"; float
    // or double for "float"; std::string for "choice" and "string". A key the manifest does not
    // declare, or of another type, gives 0, 0.0, false or "".
    template <class T>
    T setting(const char* key) const {
        // One branch per kind of setting the manifest has; the others are compiled out, and a
        // type that is none of them does not compile.
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

        // Ask by the permanent id where the type has one (the generated games/<game>.hpp do),
        // which lasts through renames of the game's functions; else by its name
        // (docs/port/MODS.md, section 4).
        const char* const asked = detail::hook_id<HookPoint>();

        // The statics of Hook hold one replacement per hook point, so a second take would
        // overwrite the first.
        if (H::callable() != nullptr) {
            throw Error(std::string(HookPoint::name) + ": this mod has already taken it");
        }

        H::callable() = new typename H::Callable(std::forward<F>(callable));

        void* original = nullptr;
        const int result = m_api->hook(
            m_handle, asked, HookPoint::signature, reinterpret_cast<void*>(&H::thunk), &original
        );

        // The port refused: drop the replacement so a later take can try again.
        if (result != OPENRAC_HOOK_OK) {
            delete H::callable();
            H::callable() = nullptr;
            throw Error(std::string(HookPoint::name) + ": " + hook_error(result));
        }

        H::original() = reinterpret_cast<typename H::Fp>(original);
        detail::taken().push_back(&H::release);
    }

    // Returns what a hook point this mod took goes on to (as the callable's `original`), for
    // calling it from elsewhere in the mod (another hook, a callback); nullptr before the hook is
    // taken.
    template <class HookPoint>
    typename HookPoint::fn original() const {
        return detail::Hook<HookPoint>::original();
    }

    // Calls `callable()` after each game tick, in load order. Throws openrac::Error when refused.
    // Only during load.
    template <class F>
    void on_frame(F&& callable) {
        // A deque keeps the stored callable where it is as more are added: the port holds its
        // address.
        auto& stored = m_frames.emplace_back(std::forward<F>(callable));

        // The port refuses a callback once load is over.
        if (m_api->on_frame(m_handle, &detail::frame_thunk, &stored) != 0) {
            throw Error("on_frame: refused (only during load)");
        }
    }

    // Calls `callable(port, report, size)` for each pad report, before the game reads it: the
    // callable may change the report's bytes. Throws openrac::Error when refused. Only during load.
    template <class F>
    void on_pad(F&& callable) {
        // As in on_frame, the port holds the address of the stored callable.
        auto& stored = m_pads.emplace_back(std::forward<F>(callable));

        // The port refuses a callback once load is over.
        if (m_api->on_pad(m_handle, &detail::pad_thunk, &stored) != 0) {
            throw Error("on_pad: refused (only during load)");
        }
    }

    // Returns a game function by its name in this disc's build, as Fn (a function pointer type);
    // nullptr when there is none. required_function throws openrac::Error instead.
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

        // The disc's build has no such function: the mod cannot run on it, so refuse the load.
        if (f == nullptr) {
            throw Error(std::string("this disc's build has no function ") + name);
        }

        return f;
    }

    // Returns the function at a code address of the level loaded now, as Fn; nullptr when there is
    // none.
    template <class Fn>
    Fn function_at(gaddr address) const {
        return reinterpret_cast<Fn>(m_api->function_at(address));
    }

    // Since 1.2: returns whether the port offers the keyboard and mouse (input(), take_input(),
    // capture_mouse()): a mod built for 1.2 is loaded only by a port that does, so a mod that
    // wants to run on 1.1 checks this first.
    bool has_input() const {
        return m_api->api_minor >= 2
               && m_api->size >= offsetof(openrac_api, capture_mouse) + sizeof(void*);
    }

    // Since 1.2: returns the keyboard and mouse for the tick about to run.
    Input input() const { return Input(m_api->input()); }

    // Since 1.2: takes the keyboard (OPENRAC_INPUT_KEYBOARD) and or the mouse from the port's own
    // use of them in play: the mod presses pad 0's buttons through on_pad. Throws openrac::Error
    // when refused. Only during load.
    void take_input(std::uint32_t devices) {
        // The port hands the devices over during load only.
        if (m_api->take_input(m_handle, devices) != 0) {
            throw Error("take_input: refused (only during load)");
        }
    }

    // Since 1.2: asks for the cursor captured (hidden, motion relative) or let go. The port may let
    // it go anyway (window not focused, overlay open). Any time.
    void capture_mouse(bool capture) const { m_api->capture_mouse(m_handle, capture ? 1 : 0); }

    // Returns a view of game memory.
    Memory memory() const { return Memory(m_api->memory); }

    // Return a global's address for the code loaded now: the executable's (`relocate`) or level
    // `level`'s program's (`relocate_level`). Ask each time, because a level moves its globals.
    gaddr relocate(gaddr address) const { return m_api->relocate_data(address); }

    gaddr relocate_level(int level, gaddr address) const {
        return m_api->relocate_level_data(level, address);
    }

    // Returns the level whose program is loaded, -1 for none (the executable's).
    int loaded_level() const { return m_api->loaded_level(); }

private:
    // Returns the words for a failed openrac_api.hook, for the Error that load throws.
    static const char* hook_error(int result) {
        // Each OPENRAC_HOOK_* failure has its own words; any other code is the port's and unnamed.
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

    // The port's table and this mod's handle; not owned, valid for the life of the program.
    const openrac_api* m_api;
    openrac_mod* m_handle;

    // The callbacks the mod gave on_frame and on_pad, at stable addresses (a deque): the port holds
    // pointers to these.
    std::deque<detail::FrameCallable> m_frames;
    std::deque<detail::PadCallable> m_pads;
};

// Room on the game's stack for a call that takes the address of a local, given back when it goes.
// Not copyable; frames nest and must end innermost first, which scopes do. Since 1.0.
class StackFrame {
public:
    // Pushes `size` bytes (aligned to 16) on the game's stack through `mod`'s table.
    StackFrame(const Mod& mod, std::uint32_t size) : m_api(&mod.api()) {
        m_address = m_api->stack_push(size, &m_saved);
    }

    // Pops what the constructor pushed.
    ~StackFrame() { m_api->stack_pop(m_saved); }

    StackFrame(const StackFrame&) = delete;
    StackFrame& operator=(const StackFrame&) = delete;

    // Returns the game address of the room.
    gaddr address() const { return m_address; }

private:
    // The port's table; not owned.
    const openrac_api* m_api;

    // The stack as it was before the push, for the pop.
    gaddr m_saved = 0;

    // The room's game address.
    gaddr m_address = 0;
};

// ----------------------------------------------------------------------------------------------
// Entry points
// ----------------------------------------------------------------------------------------------

namespace detail {

// Logs an exception that a hook or a callback let out, as an error under the mod's id.
inline void report(const char* where, const char* what) {
    // Before load made the mod there is nobody to log under.
    if (Mod* mod = instance()) {
        mod->error(std::string("exception in ") + where + ": " + what);
    }
}

// Runs `load_fn(mod)` for openrac_mod_load: makes the library's Mod and turns an exception out of
// `load_fn` into the mod's refusal. Returns 0 to load, 1 to refuse.
template <class Load>
int load(const openrac_api* api, openrac_mod* handle, Load&& load_fn) {
    // The port may load a library again before the system has unloaded it: let go of what the
    // earlier load took first.
    for (auto release : taken()) {
        release();
    }

    taken().clear();

    Mod* mod = nullptr;

    // A throw out of load_fn refuses the load with its message, as the interface has it.
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

// Writes the two entry points of a C++ mod: openrac_mod_query (the id and version, as its
// manifest's) and openrac_mod_load, which calls load_fn(openrac::Mod&). A throw out of load_fn
// refuses the load with its message. `mod_id` and `mod_version` are string literals. Since 1.0.
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
