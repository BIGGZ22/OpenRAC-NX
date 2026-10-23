// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac::Result<T>: what a public function that can fail returns, either its value or an Error
// whose message says what went wrong (CODING_CONVENTIONS.md, section 13.3).
//
// It is the port's own std::expected<T, Error>, which is C++23 while the port's floor is C++20
// (section 4.1). Its members have std::expected's names and meanings, and a failure is returned as
// `unexpected(Error{...})`, as std::unexpected spells it, so moving to std::expected once the floor
// allows it is a rename. It keeps only what the port uses: no error type other than Error, no
// monadic operations, no value_or.

#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace openrac {

// Why a function failed: a message in the form of section 17.3 (lower case, the values, no full
// stop), ready for the log or for the caller's own message.
struct Error {
    std::string message;  // What happened, to what, with the values.
};

// An Error on its way into a Result, as std::unexpected carries one into a std::expected.
//
// Made by unexpected(); a Result built from it holds the error.
struct Unexpected {
    Error error;  // The error the Result will hold.
};

// Wraps `error` so that a Result built from it holds the error: `return unexpected(Error{...});`.
inline Unexpected unexpected(Error error) {
    return Unexpected{std::move(error)};
}

// Either a value of type T or the Error that kept the function from making one.
//
// Copied and moved as T and Error are (a Result of a move-only T is move-only). `value()` and
// `error()` on the wrong side throw std::bad_variant_access, where std::expected throws its own
// exception: either way a mistake of the caller's, never a way to learn of the failure.
template <typename T>
class [[nodiscard]] Result {
public:
    static_assert(!std::is_reference_v<T>, "a Result holds a value, not a reference");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, Error>, "a Result's value is not an Error");

    // Holds `value`: the function succeeded. Not explicit, so `return bank;` makes the Result, as
    // it makes a std::expected.
    explicit(false) Result(T value) : m_state(std::in_place_index<kValue>, std::move(value)) {}

    // Holds `failure.error`: the function failed. Not explicit, so `return unexpected(...);` makes
    // the Result.
    explicit(false) Result(Unexpected failure)
        : m_state(std::in_place_index<kError>, std::move(failure.error)) {}

    // True when the Result holds a value.
    bool has_value() const noexcept { return m_state.index() == kValue; }

    // True when the Result holds a value, as has_value(): `if (auto bank = load_bank(path))`.
    explicit operator bool() const noexcept { return has_value(); }

    // The value; throws std::bad_variant_access when the Result holds an error.
    T& value() & { return std::get<kValue>(m_state); }

    // The value, read only; throws std::bad_variant_access when the Result holds an error.
    const T& value() const& { return std::get<kValue>(m_state); }

    // The value, moved out; throws std::bad_variant_access when the Result holds an error.
    T&& value() && { return std::get<kValue>(std::move(m_state)); }

    // The value, as value() gives it.
    T& operator*() & { return value(); }

    // The value, read only, as value() gives it.
    const T& operator*() const& { return value(); }

    // The value, moved out, as value() gives it.
    T&& operator*() && { return std::move(*this).value(); }

    // The value's members: `result->size()`.
    T* operator->() { return &value(); }

    // The value's members, read only.
    const T* operator->() const { return &value(); }

    // The error; throws std::bad_variant_access when the Result holds a value.
    Error& error() & { return std::get<kError>(m_state); }

    // The error, read only; throws std::bad_variant_access when the Result holds a value.
    const Error& error() const& { return std::get<kError>(m_state); }

    // The error, moved out; throws std::bad_variant_access when the Result holds a value.
    Error&& error() && { return std::get<kError>(std::move(m_state)); }

private:
    // The alternatives of m_state, by index: what std::get reads and index() returns.
    static constexpr std::size_t kValue = 0;
    static constexpr std::size_t kError = 1;

    std::variant<T, Error> m_state;  // The value or the error, never neither.
};

// What a function that has no value to give returns: success, or the Error that stopped it.
//
// `return {};` is success. Copied and moved as Error is. `value()` and `error()` on the wrong side
// throw std::bad_variant_access, as Result<T>'s do.
template <>
class [[nodiscard]] Result<void> {
public:
    // Success.
    Result() = default;

    // Holds `failure.error`: the function failed; not explicit, as Result<T>'s.
    explicit(false) Result(Unexpected failure)
        : m_state(std::in_place_index<kError>, std::move(failure.error)) {}

    // True on success.
    bool has_value() const noexcept { return m_state.index() == kValue; }

    // True on success, as has_value().
    explicit operator bool() const noexcept { return has_value(); }

    // Returns on success; throws std::bad_variant_access when the Result holds an error, as
    // std::expected<void, E>::value() throws.
    void value() const { static_cast<void>(std::get<kValue>(m_state)); }

    // Does nothing, as std::expected<void, E>'s operator* does.
    void operator*() const noexcept {}

    // The error; throws std::bad_variant_access when the Result holds success.
    Error& error() & { return std::get<kError>(m_state); }

    // The error, read only; throws std::bad_variant_access when the Result holds success.
    const Error& error() const& { return std::get<kError>(m_state); }

    // The error, moved out; throws std::bad_variant_access when the Result holds success.
    Error&& error() && { return std::get<kError>(std::move(m_state)); }

private:
    // The alternatives of m_state, by index, as Result<T> has them.
    static constexpr std::size_t kValue = 0;
    static constexpr std::size_t kError = 1;

    std::variant<std::monostate, Error> m_state;  // Success (monostate) or the error.
};

}  // namespace openrac
