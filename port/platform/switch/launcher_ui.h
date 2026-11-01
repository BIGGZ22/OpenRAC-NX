// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include "platform/switch/launcher_artwork.h"
#include "platform/switch/launcher_appearance.h"

namespace openrac::platform::launcher_ui {
constexpr int kWidth = 1280, kHeight = 720;
// Pages ("tabs"), as the launcher's logic numbers them.
constexpr int kTabCount = 11, kArtworkTab = 9, kAppearanceTab = 10;
inline bool settings_tab(int tab) { return (tab >= 1 && tab <= 4) || tab == 7 || tab == kAppearanceTab; }

// Sections along the top bar; L / R moves between them. Settings and Customize hold several pages,
// listed down the left (the page list), the others one page each.
constexpr int kSectionCount = 6;
inline const char* const kSectionNames[kSectionCount] = {"Play", "Settings", "Saves", "Customize", "Setup", "About"};
inline constexpr int kSettingsPages[] = {1, 2, 3, 7, 4};
inline constexpr int kCustomizePages[] = {kArtworkTab, kAppearanceTab};
inline int section_of(int tab) {
    switch (tab) {
        case 0: return 0;
        case 5: return 2;
        case kArtworkTab: case kAppearanceTab: return 3;
        case 6: return 4;
        case 8: return 5;
        default: return 1;
    }
}
inline std::span<const int> section_pages(int section) {
    static constexpr int single[kSectionCount] = {0, 1, 5, kArtworkTab, 6, 8};
    if (section == 1) return kSettingsPages;
    if (section == 3) return kCustomizePages;
    return {&single[section], 1};
}
inline const char* page_name(int tab) {
    switch (tab) {
        case 1: return "Display";
        case 2: return "Audio";
        case 3: return "Gameplay";
        case 4: return "Game fixes";
        case 7: return "Controller";
        case kArtworkTab: return "Artwork";
        case kAppearanceTab: return "Appearance";
        default: return kSectionNames[section_of(tab)];
    }
}

// Layout shared by the canvas (what is drawn) and the launcher (what a touch hits).
constexpr int kTopBar = 76, kFooterY = 662;
constexpr int kSectionX = 286, kSectionStep = 122, kSectionW = 118, kSectionY = 16, kSectionH = 44;
constexpr int kPageX = 40, kPageW = 240, kPageY = 128, kPageStep = 58, kPageH = 52;
constexpr int kRowsX = 300, kRowsY = 98, kRowsW = 590, kRowsH = 534;
constexpr int kRowX = 310, kRowY = 110, kRowW = 570, kRowStep = 56, kRowH = 52, kVisibleRows = 9;
constexpr int kRowValueX = 640;  // a touch right of this on a row changes it (A)
constexpr int kHelpX = 910, kHelpW = 330;
constexpr int kHeroX = 40, kHeroY = 98, kHeroW = 1200, kHeroH = 400;
constexpr int kLaunchX = 88, kLaunchY = 326, kLaunchW = 262, kLaunchH = 64;
constexpr int kBootX = 366, kBootW = 254;
constexpr int kStatusY = 516, kStatusH = 116;
constexpr int kSetupButtonX = 88, kSetupButtonY = 530, kSetupButtonW = 320, kSetupButtonH = 60;
constexpr int kCancelX = 258, kCancelY = 470, kCancelW = 260, kCancelH = 56;
inline int first_visible_row(int selected) { return selected > kVisibleRows - 1 ? selected - (kVisibleRows - 1) : 0; }

// Full-screen layout.
constexpr int kMenuWidth = kWidth, kMenuHeight = kHeight;
constexpr int kMenuX = (kWidth - kMenuWidth) / 2, kMenuY = (kHeight - kMenuHeight) / 2;
inline bool menu_touch(int& x, int& y) {
    if (x < kMenuX || y < kMenuY || x >= kMenuX + kMenuWidth || y >= kMenuY + kMenuHeight) return false;
    x = (x - kMenuX) * kWidth / kMenuWidth;
    y = (y - kMenuY) * kHeight / kMenuHeight;
    return true;
}

struct Row {
    std::string label, value, help;
};

struct View {
    int tab = 0, selected = 0, save_files = 0;
    bool pages_focus = false;  // Settings / Customize: the page list has the focus, not the rows
    bool disc = false, executable = false, levels = false, writable = false;
    std::vector<Row> rows;
    std::string notice;
    std::shared_ptr<const artwork::Image> art;
    bool custom_art = false;
    appearance::Settings appearance;
    std::shared_ptr<const artwork::Image> banner_background, window_art;
};

class Canvas {
public:
    Canvas();
    ~Canvas();
    bool font(std::span<const std::uint8_t> bytes);
    // The display face for titles (optional; the interface font is used without it).
    bool heading_font(std::span<const std::uint8_t> bytes);
    void menu(const View& view);
    void progress(const std::string& stage, unsigned done, unsigned total);
    void binding(const std::string& action, const std::string& prompt);
    const std::vector<std::uint32_t>& pixels() const;

private:
    struct Impl;
    std::unique_ptr<Impl> p;
};

class Graphics {
public:
    Graphics();
    ~Graphics();
    bool open(std::string& error);
    bool show(const View& view, std::string& error);
    bool progress(const std::string& stage, unsigned done, unsigned total, std::string& error);
    bool binding(const std::string& action, const std::string& prompt, std::string& error);

private:
    struct Impl;
    std::unique_ptr<Impl> p;
};
}  // namespace openrac::platform::launcher_ui
