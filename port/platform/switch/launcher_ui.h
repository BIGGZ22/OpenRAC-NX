// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace openrac::platform::launcher_ui {
constexpr int kWidth = 1280, kHeight = 720;

struct Row {
    std::string label, value;
};

struct View {
    int tab = 0, selected = 0, save_files = 0;
    bool disc = false, executable = false, levels = false, writable = false;
    std::vector<Row> rows;
    std::string notice;
};

class Canvas {
public:
    Canvas();
    ~Canvas();
    bool font(std::span<const std::uint8_t> bytes);
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
