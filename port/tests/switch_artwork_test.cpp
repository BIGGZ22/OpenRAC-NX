// SPDX-License-Identifier: GPL-3.0-or-later
#include "check.h"
#include "platform/switch/launcher_artwork.h"
#include <filesystem>
#include <fstream>
#include <iterator>
using namespace openrac::platform::artwork;
int main(int argc, char** argv) {
    CHECK(argc == 3);
    if (argc != 3) return 1;
    std::vector<Entry> entries; std::string error;
    CHECK(image_url("https://cdn2.steamgriddb.com/hero/test.png"));
    CHECK(!image_url("http://cdn2.steamgriddb.com/a.png"));
    CHECK(!image_url("https://cdn2.steamgriddb.com.evil.test/a.png"));
    CHECK(!image_url("https://cdn2.steamgriddb.com@evil.test/a.png"));
    CHECK(!image_url("https://cdn2.steamgriddb.com:444/a.png"));
    CHECK(parse_games(R"({"success":true,"data":[{"id":2,"name":"Ratchet & Clank"},{"id":-1,"name":"Bad"}]})", entries, error));
    CHECK(entries.size() == 1 && entries[0].id == 2);
    CHECK(!parse_games(R"({"success":false,"data":[]})", entries, error));
    CHECK(!parse_games("not json", entries, error));
    CHECK(parse_images(R"({"success":true,"data":[{"id":5,"url":"https://cdn2.steamgriddb.com/hero/a.png","author":{"name":"Artist"}},{"id":6,"url":"https://evil.test/a.png"},{"id":7,"url":"https://cdn2.steamgriddb.com/a.png","nsfw":true}]})", entries, error));
    CHECK(entries.size() == 1 && entries[0].author == "Artist");
    CHECK(!decode({}, error));
    const std::uint8_t invalid[] = {0, 1, 2, 3};
    CHECK(!decode(invalid, error));
    CHECK(bundled() && bundled()->width == 512 && bundled()->height == 512);
    const std::filesystem::path root(argv[1]);
    std::filesystem::create_directories(root);
    // Test only in a newly created, caller-selected fixture directory.
    CHECK(!std::filesystem::exists(root / "selection.txt"));
    std::filesystem::copy_file(argv[2], root / "custom.png");
    {
        Library lib(root);
        CHECK(lib.state().custom);
        CHECK(lib.set_key("abc123"));
        CHECK(lib.state().key_set);
        CHECK(!lib.set_key("bad\r\nheader"));
        lib.reset(); CHECK(!lib.state().custom);
    }
    {
        Library lib(root); CHECK(!lib.state().custom); CHECK(lib.state().key_set);
        lib.reload_local(); CHECK(lib.state().custom);
        CHECK(lib.set_key("")); CHECK(!lib.state().key_set);
    }
    {
        Library lib(root); CHECK(lib.state().custom); CHECK(!lib.state().key_set);
        CHECK(lib.set_key("recover123"));
    }
    std::filesystem::rename(root / "steamgriddb.key", root / "steamgriddb.key.previous");
    std::filesystem::rename(root / "selection.txt", root / "selection.txt.previous");
    { Library lib(root); CHECK(lib.state().custom); CHECK(lib.state().key_set); }
    CHECK(std::filesystem::exists(root / "steamgriddb.key"));
    CHECK(std::filesystem::exists(root / "selection.txt"));
    CHECK(std::filesystem::exists(root / "custom.png"));
    CHECK(!std::filesystem::exists(root / "banner.img"));
    return openrac::test::result();
}
