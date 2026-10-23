// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac-extractor IMAGE --game GAME [--proj-path DIR] [--extract] [--validate]
//                   [--decompile] [--compile] [--json]
//
// Sets a game up from the image of the player's own disc (extractor.h). The
// command line is tools/extractor.py's; --json prints the launcher's JSON
// lines (report.h). Exit status 0, or 1 after "error NNNN: ...".

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

#include "extract/extractor.h"

namespace {

// The help text, printed for -h and after every command-line error.
constexpr const char* kUsage =
    "usage: openrac-extractor IMAGE --game GAME [--proj-path DIR] [--extract] [--validate]\n"
    "                         [--decompile] [--compile] [--json]\n"
    "\n"
    "Sets a game up from the image (.iso) of your own disc. With no step named, runs\n"
    "them all. GAME is rac1, rac2, rac3 or rac4; DIR defaults to the current folder.\n";

// Prints `problem` and the help text to the error stream; returns the exit status 2 for main.
int usage(const char* problem) {
    std::fprintf(stderr, "openrac-extractor: %s\n%s", problem, kUsage);
    return 2;
}

}  // namespace

int main(int argc, char** argv) {
    openrac::extract::Options options;
    bool json = false;

    // Every argument is a step flag, an option with a value, or the image: each flag has a short
    // and a long form.
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        std::string value;
        auto take = [&](std::string_view name) {
            // The "--name=value" form: the value follows the equals sign.
            if (arg.starts_with(name) && arg.size() > name.size() && arg[name.size()] == '=') {
                value = std::string(arg.substr(name.size() + 1));
                return true;
            }

            // The "--name value" form: the value is the next argument, which must exist.
            if (arg == name && i + 1 < argc) {
                value = argv[++i];
                return true;
            }

            return false;
        };

        // Help is printed and the program ends with success, before anything is checked.
        if (arg == "-h" || arg == "--help") {
            std::fputs(kUsage, stdout);
            return 0;
        } else if (take("--game") || take("-g")) {
            options.game = value;
        } else if (take("--proj-path")) {
            options.proj = value;
        } else if (arg == "-e" || arg == "--extract") {
            options.extract = true;
        } else if (arg == "-v" || arg == "--validate") {
            options.validate = true;
        } else if (arg == "-d" || arg == "--decompile") {
            options.decompile = true;
        } else if (arg == "-c" || arg == "--compile") {
            options.compile = true;
        } else if (arg == "--json") {
            json = true;
        } else if (!arg.starts_with('-') && options.image.empty()) {
            options.image = std::string(arg);
        } else {
            return usage(("unknown argument " + std::string(arg)).c_str());
        }
    }

    // The image is the only required argument; without it there is nothing to set up.
    if (options.image.empty()) {
        return usage("no image named");
    }

    // Only the games the extractor has a table for are accepted.
    if (options.game != "rac1" && options.game != "rac2" && options.game != "rac3"
        && options.game != "rac4") {
        return usage("--game must be rac1, rac2, rac3 or rac4");
    }

    openrac::extract::Reporter report(json);
    return openrac::extract::run(options, report);
}
