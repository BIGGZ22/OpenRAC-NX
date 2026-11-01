// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/switch/launcher_artwork.h"
#include "common/json.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iterator>
#include <mutex>
#include <thread>
#include <unistd.h>
#ifdef _WIN32
#include <io.h>
#endif
#ifdef __SWITCH__
#include <switch.h>
#include <curl/curl.h>
#endif
// Private decoder avoids changing the game's PNG-only texture decoder.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#define STBI_MAX_DIMENSIONS 4096
#include <stb_image.h>
#include "launcher_art_builtin.h"
#include "launcher_heading_font.h"

namespace openrac::platform::artwork {
namespace {
constexpr std::size_t kMaxFile = 8 * 1024 * 1024;
bool document(const std::string& body, json::Value& doc, std::string& error) {
    if (!json::parse(body, doc, error) || !doc["success"].boolean() || !doc["data"].is_array()) {
        error = "SteamGridDB returned an invalid response."; return false;
    }
    return true;
}
std::vector<std::uint8_t> read(const std::filesystem::path& path) {
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size == 0 || size > kMaxFile) return {};
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    std::ifstream file(path, std::ios::binary);
    if (!file.read(reinterpret_cast<char*>(bytes.data()), bytes.size())) return {};
    return bytes;
}
bool write(const std::filesystem::path& path, std::span<const std::uint8_t> bytes) {
    const auto temp = path.string() + ".new";
    FILE* file = std::fopen(temp.c_str(), "wb");
    if (!file) return false;
    bool ok = std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size();
    if (std::fflush(file) != 0) ok = false;
#ifdef _WIN32
    if (_commit(fileno(file)) != 0) ok = false;
#else
    if (fsync(fileno(file)) != 0) ok = false;
#endif
    if (std::fclose(file) != 0) ok = false;
    if (ok && std::rename(temp.c_str(), path.string().c_str()) == 0) return true;
    // Some SD/host filesystems cannot replace an existing destination. Keep a
    // recovery copy while replacing an app-managed cache or preference file.
    if (ok) {
        const auto backup = path.string() + ".previous";
        std::remove(backup.c_str());
        if (std::rename(path.string().c_str(), backup.c_str()) == 0) {
            if (std::rename(temp.c_str(), path.string().c_str()) == 0) {
                std::remove(backup.c_str()); return true;
            }
            std::rename(backup.c_str(), path.string().c_str());
        }
    }
    std::remove(temp.c_str()); return false;
}
std::string escape(const std::string& text) {
    constexpr char digits[] = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : text) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
            || c == '-' || c == '_' || c == '.' || c == '~') out += char(c);
        else { out += '%'; out += digits[c >> 4]; out += digits[c & 15]; }
    }
    return out;
}
#ifdef __SWITCH__
struct Transfer { std::vector<std::uint8_t> bytes; std::size_t limit; std::atomic_bool* cancel; };
size_t receive(char* data, size_t size, size_t count, void* user) {
    auto& t = *static_cast<Transfer*>(user);
    if (size && count > t.limit / size) return 0;
    const auto n = size * count;
    if (t.cancel->load() || n > t.limit - t.bytes.size()) return 0;
    try { t.bytes.insert(t.bytes.end(), data, data + n); }
    catch (...) { return 0; }
    return n;
}
int progress(void* user, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    return static_cast<std::atomic_bool*>(user)->load() ? 1 : 0;
}
#endif
bool get(const std::string& url, const std::string& key, std::size_t limit,
         std::atomic_bool& cancel, std::vector<std::uint8_t>& bytes, std::string& error) {
#ifdef __SWITCH__
    // Only used by the launcher's worker; all sessions end before game launch.
    const Result sockets = socketInitializeDefault();
    if (R_FAILED(sockets)) { error = "Could not start networking. Try again."; return false; }
    struct Network { ~Network() { socketExit(); } } network;
    const CURLcode initialized = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (initialized != CURLE_OK) { error = "Could not start HTTPS."; return false; }
    struct CurlGlobal { ~CurlGlobal() { curl_global_cleanup(); } } global;
    CURL* curl = curl_easy_init();
    if (!curl) { error = "Could not start the download."; return false; }
    Transfer transfer{{}, limit, &cancel};
    curl_slist* headers = nullptr;
    if (!key.empty()) headers = curl_slist_append(headers, ("Authorization: Bearer " + key).c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "OpenRAC-NX/0.2.0-alpha artwork");
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 25L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    // Never forward an API key to a redirect destination.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &transfer);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancel);
    const CURLcode result = curl_easy_perform(curl);
    long status = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(headers); curl_easy_cleanup(curl);
    if (cancel.load()) error = "Download cancelled.";
    else if (status == 401 || status == 403) error = "SteamGridDB rejected the key. Check your API key.";
    else if (status == 429) error = "SteamGridDB is busy. Wait a little before trying again.";
    else if (result != CURLE_OK) error = "Network or HTTPS error. Check Wi-Fi and try again.";
    else if (status != 200) error = "SteamGridDB could not complete this request (HTTP " + std::to_string(status) + ").";
    else { bytes = std::move(transfer.bytes); return true; }
    return false;
#else
    (void)url; (void)key; (void)limit; (void)cancel; (void)bytes;
    error = "Networking is only enabled in the Switch launcher."; return false;
#endif
}
}

bool image_url(const std::string& url) {
    constexpr const char* prefix = "https://";
    if (!url.starts_with(prefix)) return false;
    const auto end = url.find('/', 8);
    if (end == std::string::npos) return false;
    const auto host = url.substr(8, end - 8);
    return host == "cdn.steamgriddb.com" || host == "cdn2.steamgriddb.com";
}
bool parse_games(const std::string& body, std::vector<Entry>& out, std::string& error) {
    out.clear(); json::Value doc;
    if (!document(body, doc, error)) return false;
    for (const auto& row : doc["data"].items()) {
        const double id = row["id"].number();
        if (id < 1 || id > 2147483647 || row["name"].string().empty()) continue;
        out.push_back({static_cast<int>(id), row["name"].string(), {}, {}});
        if (out.size() == 30) break;
    }
    if (out.empty()) { error = "No matching games found. Try another search."; return false; }
    return true;
}
bool parse_images(const std::string& body, std::vector<Entry>& out, std::string& error) {
    out.clear(); json::Value doc;
    if (!document(body, doc, error)) return false;
    for (const auto& row : doc["data"].items()) {
        const double id = row["id"].number();
        const auto& url = row["url"].string();
        if (id < 1 || id > 2147483647 || !image_url(url) || row["nsfw"].boolean()
            || row["humor"].boolean() || row["epilepsy"].boolean()) continue;
        out.push_back({static_cast<int>(id), "Artwork " + std::to_string(out.size() + 1), url,
            row["author"]["name"].string()});
        if (out.size() == 40) break;
    }
    if (out.empty()) { error = "No suitable static banners found for this game."; return false; }
    return true;
}
std::shared_ptr<const Image> decode(std::span<const std::uint8_t> bytes, std::string& error) {
    int w = 0, h = 0, channels = 0;
    if (bytes.empty() || bytes.size() > kMaxFile
        || !stbi_info_from_memory(bytes.data(), int(bytes.size()), &w, &h, &channels)
        || w < 1 || h < 1 || w > 4096 || h > 4096 || std::uint64_t(w) * h > 12'000'000) {
        error = "Artwork must be a PNG/JPEG up to 8 MiB and 4096 pixels per side."; return {};
    }
    unsigned char* rgba = stbi_load_from_memory(bytes.data(), int(bytes.size()), &w, &h, &channels, 4);
    if (!rgba) { error = "Could not decode this artwork. Try another image."; return {}; }
    auto image = std::make_shared<Image>();
    const double scale = std::min({1.0, 1280.0 / w, 720.0 / h});
    image->width = std::max(1, int(w * scale)); image->height = std::max(1, int(h * scale));
    image->pixels.resize(std::size_t(image->width) * image->height);
    for (int y = 0; y < image->height; ++y) for (int x = 0; x < image->width; ++x) {
        const auto src = (std::size_t(y * h / image->height) * w + x * w / image->width) * 4;
        image->pixels[std::size_t(y) * image->width + x] = std::uint32_t(rgba[src])
            | std::uint32_t(rgba[src + 1]) << 8 | std::uint32_t(rgba[src + 2]) << 16
            | std::uint32_t(rgba[src + 3]) << 24;
    }
    stbi_image_free(rgba); return image;
}
std::shared_ptr<const Image> bundled() {
    std::string error;
    return decode({kLauncherArt, sizeof(kLauncherArt)}, error);
}
std::span<const std::uint8_t> heading_font() {
    return {kLauncherHeadingFont, sizeof(kLauncherHeadingFont)};
}
std::shared_ptr<const Image> load_image(const std::filesystem::path& path, std::string& error) {
    return decode(read(path), error);
}

struct Library::Impl {
    std::filesystem::path root;
    mutable std::mutex mutex;
    State state;
    std::string key;
    std::thread worker;
    std::atomic_bool cancelled = false;
    explicit Impl(std::filesystem::path path) : root(std::move(path)) {}
    ~Impl() { cancelled = true; if (worker.joinable()) worker.join(); }
    void update(const std::function<void(State&)>& fn) {
        std::lock_guard lock(mutex); fn(state); ++state.revision;
    }
    void start(const std::function<void()>& job) {
        { std::lock_guard lock(mutex); if (state.busy) return; }
        if (worker.joinable()) worker.join();
        cancelled = false;
        update([](State& s) { s.busy = true; s.message = "Connecting to SteamGridDB..."; });
        worker = std::thread([this, job] {
            try { job(); }
            catch (...) { update([](State& s) { s.message = "Artwork operation failed. Your current art was kept."; }); }
            update([](State& s) { s.busy = false; });
        });
    }
    bool api(const std::string& endpoint, std::string& body, std::string& error) {
        if (key.empty()) { error = "Add your own SteamGridDB API key first."; return false; }
        std::vector<std::uint8_t> bytes;
        if (!get("https://www.steamgriddb.com/api/v2/" + endpoint, key, 1024 * 1024, cancelled, bytes, error)) return false;
        body.assign(bytes.begin(), bytes.end()); return true;
    }
    void result(const std::string& message) { update([&](State& s) { s.message = message; }); }
};
Library::Library(std::filesystem::path root) : p(std::make_unique<Impl>(std::move(root))) {
    std::error_code ec; std::filesystem::create_directories(p->root, ec);
    for (const char* name : {"steamgriddb.key", "selection.txt", "banner.img"}) {
        const auto path = p->root / name;
        if (!std::filesystem::exists(path, ec) && std::filesystem::exists(path.string() + ".previous", ec))
            std::filesystem::rename(path.string() + ".previous", path, ec);
    }
    std::ifstream key(p->root / "steamgriddb.key"); std::getline(key, p->key);
    if (p->key.size() > 128 || !std::all_of(p->key.begin(), p->key.end(), [](unsigned char c) { return std::isalnum(c); })) p->key.clear();
    p->state.key_set = !p->key.empty();
    reload_local(false);
}
Library::~Library() = default;
State Library::state() const { std::lock_guard lock(p->mutex); return p->state; }
bool Library::set_key(std::string key) {
    if (state().busy) return false;
    if (key.size() > 128 || !std::all_of(key.begin(), key.end(), [](unsigned char c) { return std::isalnum(c); })) {
        p->result("The API key should contain only letters and numbers."); return false;
    }
    if (!write(p->root / "steamgriddb.key", {reinterpret_cast<const std::uint8_t*>(key.data()), key.size()})) {
        p->result("Could not save the API key to the SD card."); return false;
    }
    p->key = std::move(key);
    p->update([&](State& s) { s.key_set = !p->key.empty(); s.message = s.key_set ? "API key saved locally." : "API key removed."; }); return true;
}
void Library::search(std::string query) {
    if (state().busy || query.empty() || query.size() > 100) return;
    p->start([this, query] {
        std::string body, error; std::vector<Entry> games;
        if (!p->api("search/autocomplete/" + escape(query), body, error) || !parse_games(body, games, error)) { p->result(error); return; }
        p->update([&](State& s) { s.games = std::move(games); s.game = 0; s.images.clear(); s.image = 0;
            s.query = query; s.message = "Choose a game, then select Find banners."; });
    });
}
void Library::select_game(int by) {
    if (state().busy) return;
    p->update([&](State& s) { if (!s.games.empty()) { s.game = (s.game + by + int(s.games.size())) % int(s.games.size()); s.images.clear(); s.image = 0; } });
}
void Library::browse() {
    const auto s = state(); if (s.busy) return;
    if (s.games.empty()) { p->result("Search for a game first."); return; }
    const int id = s.games[s.game].id;
    p->start([this, id] {
        std::string body, error; std::vector<Entry> images;
        if (!p->api("heroes/game/" + std::to_string(id) + "?types=static&mimes=image%2Fpng,image%2Fjpeg&nsfw=false&humor=false", body, error)
            || !parse_images(body, images, error)) { p->result(error); return; }
        p->update([&](State& s) { s.images = std::move(images); s.image = 0; s.message = "Choose a banner with Left / Right, then Download and apply."; });
    });
}
void Library::select_image(int by) {
    if (state().busy) return;
    p->update([&](State& s) { if (!s.images.empty()) s.image = (s.image + by + int(s.images.size())) % int(s.images.size()); });
}
void Library::download() {
    const auto s = state(); if (s.busy) return;
    if (s.images.empty()) { p->result("Find banners first."); return; }
    const Entry entry = s.images[s.image];
    p->start([this, entry] {
        std::vector<std::uint8_t> bytes; std::string error;
        if (!get(entry.url, {}, kMaxFile, p->cancelled, bytes, error)) { p->result(error); return; }
        const auto image = decode(bytes, error);
        if (!image) { p->result(error); return; }
        if (p->cancelled) { p->result("Download cancelled."); return; }
        if (!write(p->root / "banner.img", bytes)) { p->result("Could not cache the artwork. Previous art was kept."); return; }
        const std::string choice = "download";
        if (!write(p->root / "selection.txt", {reinterpret_cast<const std::uint8_t*>(choice.data()), choice.size()})) {
            p->result("Artwork downloaded, but the selection could not be saved. Try again."); return;
        }
        p->update([&](State& s) { s.art = image; s.custom = true; s.message = "Artwork applied and cached for offline use."; });
        const std::string credit = "SteamGridDB hero " + std::to_string(entry.id) + "\nAuthor: " + entry.author + "\n" + entry.url + "\n";
        write(p->root / "banner-credit.txt", {reinterpret_cast<const std::uint8_t*>(credit.data()), credit.size()});
    });
}
void Library::reload_local(bool prefer_local) {
    if (state().busy) return;
    std::shared_ptr<const Image> image; std::string error;
    std::string choice;
    { std::ifstream selection(p->root / "selection.txt"); std::getline(selection, choice); }
    if (!prefer_local && choice == "builtin") {
        p->update([](State& s) { s.custom = false; s.art = bundled(); s.message = "Included game artwork selected."; });
        return;
    }
    const std::vector<const char*> names = !prefer_local && choice == "download"
        ? std::vector<const char*>{"banner.img", "custom.png", "custom.jpg"}
        : std::vector<const char*>{"custom.png", "custom.jpg", "banner.img"};
    for (const char* name : names) {
        const auto bytes = read(p->root / name);
        if (!bytes.empty()) { image = decode(bytes, error); if (image) break; }
    }
    if (prefer_local && image) {
        const std::string chosen = "local";
        if (!write(p->root / "selection.txt", {reinterpret_cast<const std::uint8_t*>(chosen.data()), chosen.size()})) {
            p->result("Could not save the artwork selection. Check the SD card."); return;
        }
    }
    p->update([&](State& s) { s.custom = bool(image); s.art = image ? image : bundled();
        s.message = image ? "Local artwork loaded." : "Using included game art. Add custom.png or custom.jpg for your own."; });
}
void Library::reset() {
    if (state().busy) return;
    const std::string choice = "builtin";
    if (!write(p->root / "selection.txt", {reinterpret_cast<const std::uint8_t*>(choice.data()), choice.size()})) {
        p->result("Could not save the artwork selection. Check the SD card."); return;
    }
    p->update([](State& s) { s.art = bundled(); s.custom = false; s.message = "Included game artwork selected."; });
}
void Library::cancel() { p->cancelled = true; }
}
