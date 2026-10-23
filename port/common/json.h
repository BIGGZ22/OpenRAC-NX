// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A small JSON reader for the files the editor's port export writes
// (manifest.json, placements.json): a tree of values, parsed whole.

#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace openrac::json {

class Value {
public:
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    Value() = default;

    Type type() const { return m_type; }

    bool is_null() const { return m_type == Type::Null; }

    bool is_number() const { return m_type == Type::Number; }

    bool is_string() const { return m_type == Type::String; }

    bool is_array() const { return m_type == Type::Array; }

    bool is_object() const { return m_type == Type::Object; }

    double number(double fallback = 0.0) const {
        return m_type == Type::Number ? m_number : fallback;
    }

    bool boolean(bool fallback = false) const { return m_type == Type::Bool ? m_bool : fallback; }

    const std::string& string() const;

    // An array's items (empty for anything else).
    const std::vector<Value>& items() const;

    // An object's members, by key (empty for anything else).
    const std::map<std::string, Value, std::less<>>& members() const;

    // An object's member, or null when absent (or not an object).
    const Value& operator[](std::string_view key) const;

    // An array's item, or null when out of range.
    const Value& operator[](std::size_t index) const;

private:
    friend class Parser;

    Type m_type = Type::Null;
    bool m_bool = false;
    double m_number = 0.0;
    std::string m_string;
    std::vector<Value> m_items;
    std::map<std::string, Value, std::less<>> m_members;
};

// Parse a whole document. On error returns false and says where in `error`.
bool parse(std::string_view text, Value& out, std::string& error);

// Read and parse a file.
bool parse_file(const std::string& path, Value& out, std::string& error);

}  // namespace openrac::json
