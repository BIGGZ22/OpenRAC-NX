// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/json.h"

#include <cstdlib>
#include <format>
#include <fstream>
#include <sstream>

namespace openrac::json {

namespace {

const Value& null_value() {
    static const Value value;
    return value;
}

}  // namespace

const std::string& Value::string() const {
    static const std::string empty;
    return m_type == Type::String ? m_string : empty;
}

const std::vector<Value>& Value::items() const {
    static const std::vector<Value> empty;
    return m_type == Type::Array ? m_items : empty;
}

const std::map<std::string, Value, std::less<>>& Value::members() const {
    static const std::map<std::string, Value, std::less<>> empty;
    return m_type == Type::Object ? m_members : empty;
}

const Value& Value::operator[](std::string_view key) const {
    if (m_type != Type::Object) {
        return null_value();
    }
    auto it = m_members.find(key);
    return it != m_members.end() ? it->second : null_value();
}

const Value& Value::operator[](std::size_t index) const {
    if (m_type != Type::Array || index >= m_items.size()) {
        return null_value();
    }
    return m_items[index];
}

class Parser {
public:
    explicit Parser(std::string_view text) : m_text(text) {}

    bool document(Value& out, std::string& error) {
        if (!value(out, 0)) {
            error = m_error;
            return false;
        }
        skip_space();
        if (m_at != m_text.size()) {
            error = std::format("unexpected text at offset {}", m_at);
            return false;
        }
        return true;
    }

private:
    static constexpr int kMaxDepth = 64;

    bool fail(std::string_view what) {
        m_error = std::format("{} at offset {}", what, m_at);
        return false;
    }

    void skip_space() {
        while (m_at < m_text.size()
               && (m_text[m_at] == ' ' || m_text[m_at] == '\n' || m_text[m_at] == '\r'
                   || m_text[m_at] == '\t')) {
            ++m_at;
        }
    }

    bool literal(std::string_view word) {
        if (m_text.substr(m_at, word.size()) != word) {
            return fail("unknown literal");
        }
        m_at += word.size();
        return true;
    }

    bool value(Value& out, int depth) {
        if (depth > kMaxDepth) {
            return fail("nesting too deep");
        }
        skip_space();
        if (m_at >= m_text.size()) {
            return fail("unexpected end");
        }
        const char c = m_text[m_at];
        if (c == '{') {
            return object(out, depth);
        }
        if (c == '[') {
            return array(out, depth);
        }
        if (c == '"') {
            out.m_type = Value::Type::String;
            return string(out.m_string);
        }
        if (c == 't') {
            out.m_type = Value::Type::Bool;
            out.m_bool = true;
            return literal("true");
        }
        if (c == 'f') {
            out.m_type = Value::Type::Bool;
            out.m_bool = false;
            return literal("false");
        }
        if (c == 'n') {
            out.m_type = Value::Type::Null;
            return literal("null");
        }
        return number(out);
    }

    bool number(Value& out) {
        const std::size_t start = m_at;
        while (m_at < m_text.size()) {
            const char c = m_text[m_at];
            if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.' || c == 'e'
                || c == 'E') {
                ++m_at;
            } else {
                break;
            }
        }
        // strtod rather than from_chars: older C++ libraries (macOS's)
        // lack the floating-point from_chars. The program never changes the
        // C locale, so the decimal point is '.'.
        const std::string text(m_text.substr(start, m_at - start));
        char* end = nullptr;
        const double v = std::strtod(text.c_str(), &end);
        if (text.empty() || end != text.c_str() + text.size()) {
            m_at = start;
            return fail("invalid number");
        }
        out.m_type = Value::Type::Number;
        out.m_number = v;
        return true;
    }

    static void append_utf8(std::string& s, std::uint32_t code) {
        if (code < 0x80) {
            s += static_cast<char>(code);
        } else if (code < 0x800) {
            s += static_cast<char>(0xC0 | (code >> 6));
            s += static_cast<char>(0x80 | (code & 0x3F));
        } else if (code < 0x10000) {
            s += static_cast<char>(0xE0 | (code >> 12));
            s += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            s += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            s += static_cast<char>(0xF0 | (code >> 18));
            s += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
            s += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            s += static_cast<char>(0x80 | (code & 0x3F));
        }
    }

    bool hex4(std::uint32_t& code) {
        if (m_at + 4 > m_text.size()) {
            return fail("short \\u escape");
        }
        code = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = m_text[m_at++];
            code <<= 4;
            if (c >= '0' && c <= '9') {
                code |= static_cast<std::uint32_t>(c - '0');
            } else if (c >= 'a' && c <= 'f') {
                code |= static_cast<std::uint32_t>(c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                code |= static_cast<std::uint32_t>(c - 'A' + 10);
            } else {
                return fail("invalid \\u escape");
            }
        }
        return true;
    }

    bool string(std::string& out) {
        ++m_at;  // the opening quote
        while (m_at < m_text.size()) {
            const char c = m_text[m_at++];
            if (c == '"') {
                return true;
            }
            if (c != '\\') {
                out += c;
                continue;
            }
            if (m_at >= m_text.size()) {
                break;
            }
            const char e = m_text[m_at++];
            switch (e) {
                case '"':
                case '\\':
                case '/':
                    out += e;
                    break;
                case 'b':
                    out += '\b';
                    break;
                case 'f':
                    out += '\f';
                    break;
                case 'n':
                    out += '\n';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 't':
                    out += '\t';
                    break;
                case 'u': {
                    std::uint32_t code = 0;
                    if (!hex4(code)) {
                        return false;
                    }
                    if (code >= 0xD800 && code < 0xDC00 && m_text.substr(m_at, 2) == "\\u") {
                        m_at += 2;
                        std::uint32_t low = 0;
                        if (!hex4(low)) {
                            return false;
                        }
                        code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                    }
                    append_utf8(out, code);
                    break;
                }
                default:
                    return fail("invalid escape");
            }
        }
        return fail("unterminated string");
    }

    bool array(Value& out, int depth) {
        ++m_at;
        out.m_type = Value::Type::Array;
        skip_space();
        if (m_at < m_text.size() && m_text[m_at] == ']') {
            ++m_at;
            return true;
        }
        while (true) {
            out.m_items.emplace_back();
            if (!value(out.m_items.back(), depth + 1)) {
                return false;
            }
            skip_space();
            if (m_at < m_text.size() && m_text[m_at] == ',') {
                ++m_at;
                continue;
            }
            if (m_at < m_text.size() && m_text[m_at] == ']') {
                ++m_at;
                return true;
            }
            return fail("expected , or ]");
        }
    }

    bool object(Value& out, int depth) {
        ++m_at;
        out.m_type = Value::Type::Object;
        skip_space();
        if (m_at < m_text.size() && m_text[m_at] == '}') {
            ++m_at;
            return true;
        }
        while (true) {
            skip_space();
            if (m_at >= m_text.size() || m_text[m_at] != '"') {
                return fail("expected a key");
            }
            std::string key;
            if (!string(key)) {
                return false;
            }
            skip_space();
            if (m_at >= m_text.size() || m_text[m_at] != ':') {
                return fail("expected :");
            }
            ++m_at;
            Value item;
            if (!value(item, depth + 1)) {
                return false;
            }
            out.m_members.insert_or_assign(std::move(key), std::move(item));
            skip_space();
            if (m_at < m_text.size() && m_text[m_at] == ',') {
                ++m_at;
                continue;
            }
            if (m_at < m_text.size() && m_text[m_at] == '}') {
                ++m_at;
                return true;
            }
            return fail("expected , or }");
        }
    }

    std::string_view m_text;
    std::size_t m_at = 0;
    std::string m_error;
};

bool parse(std::string_view text, Value& out, std::string& error) {
    out = Value{};
    return Parser(text).document(out, error);
}

bool parse_file(const std::string& path, Value& out, std::string& error) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        error = std::format("cannot read {}", path);
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    if (!parse(text.str(), out, error)) {
        error = std::format("{}: {}", path, error);
        return false;
    }
    return true;
}

}  // namespace openrac::json
