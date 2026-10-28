// SPDX-License-Identifier: GPL-3.0-or-later
// Host integration runner: never packages or distributes the supplied disc.
#include "scene/disc_import.h"
#include <iostream>
int main(int argc,char** argv) {
    if (argc != 3 && argc != 4) { std::cerr << "usage: native-setup ISO DATA [CANCEL_AFTER_LEVELS]\n"; return 2; }
    try {
        std::string last;
        openrac::scene::prepare_switch_disc(argv[1],argv[2],[&](const std::string& text,unsigned n,unsigned total) {
            if (text != last) { std::cout << n << "/" << total << " " << text << std::endl; last = text; }
            return argc < 4 || text.find("Checking level ") != 0 || n < static_cast<unsigned>(std::stoi(argv[3]));
        });
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << std::endl; return 1; }
}
