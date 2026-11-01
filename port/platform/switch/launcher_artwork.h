// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace openrac::platform::artwork {
struct Image { int width = 0, height = 0; std::vector<std::uint32_t> pixels; };
struct Entry { int id = 0; std::string name, url, author; };
bool image_url(const std::string& url);
bool parse_games(const std::string& body, std::vector<Entry>& out, std::string& error);
bool parse_images(const std::string& body, std::vector<Entry>& out, std::string& error);
std::shared_ptr<const Image> decode(std::span<const std::uint8_t> bytes, std::string& error);
std::shared_ptr<const Image> bundled();
// The launcher's display face for titles (Chakra Petch Bold, SIL OFL 1.1; platform/switch/fonts/).
std::span<const std::uint8_t> heading_font();
std::shared_ptr<const Image> load_image(const std::filesystem::path& path, std::string& error);

struct State {
    bool busy = false, key_set = false, custom = false;
    std::uint64_t revision = 0;
    std::string message, query = "Ratchet & Clank";
    std::vector<Entry> games, images;
    int game = 0, image = 0;
    std::shared_ptr<const Image> art;
};
class Library {
public:
    explicit Library(std::filesystem::path root);
    ~Library();
    State state() const;
    bool set_key(std::string key);
    void search(std::string query);
    void select_game(int by);
    void browse();
    void select_image(int by);
    void download();
    void reload_local(bool prefer_local = true);
    void reset();
    void cancel();
private:
    struct Impl;
    std::unique_ptr<Impl> p;
};
}
