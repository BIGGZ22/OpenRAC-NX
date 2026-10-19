// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

package dev.openrac.game;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;
import android.util.Log;
import android.view.Display;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

/**
 * The game on Android: SDL's activity around libopenrac-&lt;id&gt;.so, whose SDL_main is the
 * game's main (port/game/common/main.cpp). The launcher starts it with the library to load, the
 * command line it would give the program on a desktop and the environment to run it in
 * (docs/port/ANDROID.md); it runs in a process of its own (":game" in the manifest), which ends
 * with the game.
 */
public class GameActivity extends SDLActivity {
    /** The game's library, without lib and .so: "openrac-rac1-pal". */
    public static final String EXTRA_LIBRARY = "dev.openrac.game.LIBRARY";
    /** The program's arguments, as on a desktop: --data, --window, --levels... */
    public static final String EXTRA_ARGUMENTS = "dev.openrac.game.ARGUMENTS";
    /** NAME=VALUE pairs set before the game starts: HOME, the GPU driver's (platform/gpu_driver.h). */
    public static final String EXTRA_ENVIRONMENT = "dev.openrac.game.ENVIRONMENT";

    private static final String TAG = "OpenRAC";

    private String library = "openrac-rac1-pal";
    private String[] arguments = new String[0];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Intent intent = getIntent();
        String named = intent.getStringExtra(EXTRA_LIBRARY);
        if (named != null && named.matches("openrac-[a-z0-9-]+")) {
            library = named;
        }
        String[] given = intent.getStringArrayExtra(EXTRA_ARGUMENTS);
        if (given != null) {
            arguments = given;
        }
        String[] environment = intent.getStringArrayExtra(EXTRA_ENVIRONMENT);
        if (environment != null) {
            for (String pair : environment) {
                int at = pair.indexOf('=');
                if (at <= 0) {
                    continue;
                }
                try {
                    Os.setenv(pair.substring(0, at), pair.substring(at + 1), true);
                } catch (ErrnoException e) {
                    Log.w(TAG, "cannot set " + pair.substring(0, at) + ": " + e);
                }
            }
        }
        Log.i(TAG, "starting lib" + library + ".so with " + String.join(" ", arguments));
        super.onCreate(savedInstanceState);
        preferFastestRefresh();
    }

    /**
     * The display's fastest refresh, asked for while the game is in front. Without it a display
     * that changes its rate (Odin's 120 Hz panel) follows what the game draws: a while at a cap of
     * 60 frames a second sets it to 60 Hz, and then the display's rate (the "Display" frame rate,
     * the swap's wait) is 60 for good. The game's own frame cap paces it below the display's rate.
     */
    private void preferFastestRefresh() {
        @SuppressWarnings("deprecation")
        Display display = Build.VERSION.SDK_INT >= Build.VERSION_CODES.R ? getDisplay()
                                                                          : getWindowManager().getDefaultDisplay();
        if (display == null) {
            return;
        }
        Display.Mode current = display.getMode();
        Display.Mode fastest = current;
        for (Display.Mode mode : display.getSupportedModes()) {
            if (mode.getPhysicalWidth() == current.getPhysicalWidth()
                    && mode.getPhysicalHeight() == current.getPhysicalHeight()
                    && mode.getRefreshRate() > fastest.getRefreshRate()) {
                fastest = mode;
            }
        }
        WindowManager.LayoutParams attributes = getWindow().getAttributes();
        attributes.preferredDisplayModeId = fastest.getModeId();
        getWindow().setAttributes(attributes);
        Log.i(TAG, "display mode " + fastest.getPhysicalWidth() + "x" + fastest.getPhysicalHeight() + " at "
                + fastest.getRefreshRate() + " Hz");
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {"SDL3", library};
    }

    @Override
    protected String[] getArguments() {
        return arguments;
    }

    /**
     * A handheld's or a controller's Back key (Odin's, a Shield's) as the system's Back, which opens
     * the game's drawer (debug_overlay.h). SDL counts Back from a gamepad as the pad's Select, the
     * same button as KEYCODE_BUTTON_SELECT, and the game then opened its map; the pad's own Select
     * stays Select.
     */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        final int source = event.getSource();
        final boolean pad = (source & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD
                || (source & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK;
        if (event.getKeyCode() == KeyEvent.KEYCODE_BACK && pad) {
            if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0) {
                onNativeKeyDown(KeyEvent.KEYCODE_BACK);
            } else if (event.getAction() == KeyEvent.ACTION_UP) {
                onNativeKeyUp(KeyEvent.KEYCODE_BACK);
            }
            return true;
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // The game's state lives in its library's globals: a second play in the same process
        // would start from the first's. The process is the game's alone, so it ends with it.
        if (isFinishing()) {
            System.exit(0);
        }
    }
}
