// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A small JSON reader for the files the port reads: the editor's port export (manifest.json,
// placements.json), the mods' manifests, list and status, and the probe tools' profiles. A document
// is parsed whole into a tree of values its caller owns.
//
// It reads JSON without extensions: no comments, no trailing commas. Strings are kept as UTF-8, with
// `\u` escapes and surrogate pairs decoded; numbers are doubles; of an object's repeated keys the
// last one wins; a value nested more than 64 levels inside the top one is refused. There is no
// writer.

#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace openrac::json {

// One JSON value: null, a boolean, a number, a string, an array or an object, with its children.
//
// Copied and moved as a whole tree. Reading a value as the wrong type gives an empty or fallback
// result rather than failing, so a caller checks the type only where a wrong one is an error.
class Value {
public:
    // Which of JSON's six kinds the value is.
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    // Makes a null.
    Value() = default;

    // Returns the value's kind.
    Type type() const { return m_type; }

    // Returns whether the value is null; a missing member or item reads as null too.
    bool is_null() const { return m_type == Type::Null; }

    // Returns whether the value is a number.
    bool is_number() const { return m_type == Type::Number; }

    // Returns whether the value is a string.
    bool is_string() const { return m_type == Type::String; }

    // Returns whether the value is an array.
    bool is_array() const { return m_type == Type::Array; }

    // Returns whether the value is an object.
    bool is_object() const { return m_type == Type::Object; }

    // Returns the number, or `fallback` for anything else.
    double number(double fallback = 0.0) const {
        return m_type == Type::Number ? m_number : fallback;
    }

    // Returns the boolean, or `fallback` for anything else.
    bool boolean(bool fallback = false) const { return m_type == Type::Bool ? m_bool : fallback; }

    // Returns the string, in UTF-8 (empty for anything else).
    const std::string& string() const;

    // Returns an array's items, in order (empty for anything else).
    const std::vector<Value>& items() const;

    // Returns an object's members, by key (empty for anything else).
    const std::map<std::string, Value, std::less<>>& members() const;

    // Returns an object's member, or a null when it is absent or this is not an object.
    const Value& operator[](std::string_view key) const;

    // Returns an array's item, or a null when it is out of range or this is not an array.
    const Value& operator[](std::size_t index) const;

private:
    friend class Parser;

    Type m_type = Type::Null;    // Which of the fields below holds the value.
    bool m_bool = false;         // A boolean's value.
    double m_number = 0.0;       // A number's value.
    std::string m_string;        // A string's value, in UTF-8.
    std::vector<Value> m_items;  // An array's items, in order.
    std::map<std::string, Value, std::less<>> m_members;  // An object's members, by key.
};

// Parses `text` as one whole document into `out`. False when it is not JSON, or has text after the
// value: `error` then says what and at which byte offset, and `out` is left incomplete.
bool parse(std::string_view text, Value& out, std::string& error);

// Reads the file at `path` and parses it as parse() does. False when the file cannot be read or is
// not JSON: `error` then names the file and says why.
bool parse_file(const std::string& path, Value& out, std::string& error);

}  // namespace openrac::json
