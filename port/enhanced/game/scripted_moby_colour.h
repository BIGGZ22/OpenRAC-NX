// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <unordered_map>

namespace openrac {
// Explicit gameplay RGB writes, independent of environmental/point-light changes.
class ScriptedMobyColours {
public:
    void set(int level, std::uint32_t address, unsigned colour) {
        if (level != m_level) {
            m_colours.clear();
            m_level = level;
        }
        m_colours[address] = colour & 0xffffffu;
    }

    bool matches(int level, std::uint32_t address, unsigned current) const {
        const auto found = m_colours.find(address);
        return level == m_level && found != m_colours.end()
               && found->second == (current & 0xffffffu);
    }

private:
    int m_level = -999;
    std::unordered_map<std::uint32_t, unsigned> m_colours;
};
}  // namespace openrac
