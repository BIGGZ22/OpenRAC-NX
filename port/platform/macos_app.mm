// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// macOS only: the program's name in the menu bar, and menus of its own there.

#import <Cocoa/Cocoa.h>

#include <SDL3/SDL.h>

#include "platform/window.h"

// A menu item presses a key: its tag is the SDL keycode, which the port's loops read as they read
// the keyboard's.
@interface OpenRACMenuTarget : NSObject
- (void)press:(id)sender;
@end

@implementation OpenRACMenuTarget
- (void)press:(id)sender {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = static_cast<SDL_Keycode>([sender tag]);
    event.key.down = true;
    event.key.windowID = SDL_GetWindowID(SDL_GetKeyboardFocus());
    SDL_PushEvent(&event);
    event.type = SDL_EVENT_KEY_UP;
    event.key.down = false;
    SDL_PushEvent(&event);
}
@end

namespace openrac::platform {

// The program is a plain executable, not an application bundle, so the menu bar names it after
// its file. The name is read from the main bundle's dictionary when the application object is
// made: putting it there before SDL starts gives the menu bar that name.
void macos_set_app_name(const char* name) {
    @autoreleasepool {
        NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
        if ([info isKindOfClass:[NSMutableDictionary class]]) {
            NSString* value = [NSString stringWithUTF8String:name];
            [(NSMutableDictionary*)info setObject:value forKey:@"CFBundleName"];
            [(NSMutableDictionary*)info setObject:value forKey:@"CFBundleDisplayName"];
        }
        [[NSProcessInfo processInfo] setProcessName:[NSString stringWithUTF8String:name]];
    }
}

namespace {

OpenRACMenuTarget* g_target = nil;

struct Item {
    const char* title;  // nullptr: a separator
    SDL_Keycode key;
    const char* shown;  // the key, as text after the title
};

void add_menu(NSString* title, const Item* items, int count) {
    NSMenu* menu = [[NSMenu alloc] initWithTitle:title];
    [menu setAutoenablesItems:NO];
    for (int i = 0; i < count; ++i) {
        if (items[i].title == nullptr) {
            [menu addItem:[NSMenuItem separatorItem]];
            continue;
        }
        NSString* text = [NSString stringWithFormat:@"%s\t%s", items[i].title, items[i].shown];
        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:text action:@selector(press:) keyEquivalent:@""];
        [item setTarget:g_target];
        [item setTag:static_cast<NSInteger>(items[i].key)];
        [menu addItem:item];
    }
    NSMenuItem* top = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
    [top setSubmenu:menu];
    // Before the Window menu, which the system keeps last but one.
    NSMenu* bar = [NSApp mainMenu];
    const NSInteger at = [bar numberOfItems] > 1 ? [bar numberOfItems] - 1 : [bar numberOfItems];
    [bar insertItem:top atIndex:at];
}

}  // namespace

void install_app_menus(bool developer) {
    @autoreleasepool {
        if (g_target != nil || [NSApp mainMenu] == nil) {
            return;
        }
        g_target = [[OpenRACMenuTarget alloc] init];
        const Item view[] = {
            {"Full Screen", SDLK_F11, "F11"},
            {"Frame Counter", SDLK_F2, "F2"},
        };
        add_menu(@"View", view, 2);
        const Item sound[] = {
            {"Volume Up", SDLK_F10, "F10"},
            {"Volume Down", SDLK_F9, "F9"},
        };
        add_menu(@"Sound", sound, 2);
        // The level select is the launcher's developer tools' (OPENRAC_DEVELOPER): offered with them.
        const Item tools[] = {
            {"Debug Overlay", SDLK_F3, "F3"},
            {nullptr, 0, nullptr},
            {"Pause or Resume", SDLK_F5, "F5"},
            {"Step One Tick", SDLK_F6, "F6"},
            {"Slower", SDLK_F7, "F7"},
            {"Faster", SDLK_F8, "F8"},
            {nullptr, 0, nullptr},
            {"Screenshot", SDLK_F12, "F12"},
            {nullptr, 0, nullptr},
            {"Level Select", SDLK_F1, "F1"},
        };
        add_menu(@"Developer", tools, developer ? 10 : 8);
    }
}

}  // namespace openrac::platform
