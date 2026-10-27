# Views of the game (VR and other displays)

Interface 1.4 lets a native mod draw the game a second way besides the
window: the two eyes of a VR headset, a panel with the HUD, a second screen.
The port draws each view from the same scene as the window's picture and
copies it where the mod says. The mod owns everything else: the headset, its
runtime (OpenXR), its controllers. OpenRAC links no VR library.

Nothing here runs unless a mod asks for it. A game with no such mod draws as
before.

## What the port offers

| Call | When | What |
|---|---|---|
| `graphics()` | any time | What the port draws with (`openrac_graphics`): OpenGL, OpenGL ES or Vulkan. OpenGL gives `gl_proc`. Vulkan gives the instance, physical device, device and queue, with the queue's family and index |
| `on_views(mod, &source)` | during load | Draw views each frame through `source` (`openrac_view_source`): `begin`, `end`, optional `closing`. One mod at a time |
| `setup_vulkan(mod, &setup)` | during load | Make the Vulkan device through `setup` (`openrac_vulkan_setup`), so the mod can share it. One mod at a time |

In C++: `mod.has_views()`, `mod.graphics()`, `mod.on_views(source)` and
`mod.setup_vulkan(setup)`, which throw `openrac::Error` when refused.

## A frame

Each frame the port draws the scene (each refresh, between ticks too) goes
like this:

1. The port records the window's picture.
2. It calls `begin(user, views, max)`. The mod fills up to `max` views
   (`OPENRAC_VIEW_MAX`, 4) and returns how many it filled. With OpenXR this
   is where `xrWaitFrame`, `xrBeginFrame`, `xrLocateViews` and
   `xrAcquireSwapchainImage` go.
3. The port draws each view and copies it into the view's `texture` (OpenGL)
   or `image` (Vulkan).
4. The port shows the window's picture and submits the frame to the GPU.
5. It calls `end(user, views, count)` with the same views. `drawn` is 1 when
   the view was drawn anew. With OpenXR this is where
   `xrReleaseSwapchainImage` and `xrEndFrame` go.

`end` follows every `begin`, even one that returned 0. Frames that show a
movie or a still picture instead of the scene call neither.

## A view

| Field | Meaning |
|---|---|
| `kind` | `OPENRAC_VIEW_FULL` (world and 2D), `OPENRAC_VIEW_WORLD` (world only) or `OPENRAC_VIEW_2D` (the 2D alone over transparent black, for a panel; give it a 4:3 shape) |
| `width`, `height` | 1 to 8192 pixels |
| `position`, `orientation` | The eye relative to the game's camera, in its view space (X right, Y up, looking down -Z, as OpenXR's), in game units. Orientation is a quaternion x, y, z, w; all zero is the identity |
| `angle_left`, `angle_right`, `angle_up`, `angle_down` | The field of view as OpenXR's `XrFovf` gives it, in radians. All zero: the game's own vertical one |
| `texture` | OpenGL: a `GL_TEXTURE_2D` of the port's context, RGBA8 or SRGB8_ALPHA8, at least the view's size |
| `image`, `image_layout` | Vulkan: a `VkImage` of the port's device, `R8G8B8A8_UNORM` or `_SRGB`, exactly the view's size, usable as a transfer destination; the layout it is in, which it is left in |
| `drawn`, `picture` | Set by the port: drawn anew this frame; the port's own GL texture of the view |

The picture is copied byte for byte. An sRGB image gets the game's picture as
the window shows it.

The game decides what it draws (mobys out of its view are not drawn), so a
mod turning the view far from the game's camera should also move the game's
camera through its hooks. The eye's pose is for the small offsets: the eyes'
distance, the head's last movement.

## Vulkan

OpenXR needs the Vulkan device made with it
(`xrCreateVulkanInstanceKHR`, `xrGetVulkanGraphicsDevice2KHR`,
`xrCreateVulkanDeviceKHR`). The window makes the device, so a mod that does
this:

- says `"vulkan_device": true` in its manifest, so that the mods load before
  the window opens (without it they load after, as before);
- calls `setup_vulkan` in its load with the functions that stand in for
  `vkCreateInstance`, the choice of the physical device and `vkCreateDevice`;
  each is optional;
- gets the device back from `graphics()` once the window is open, for
  `XrGraphicsBindingVulkanKHR`.

When `create_instance` or `create_device` fails, or the chosen device cannot
draw the game, the game draws with OpenGL instead. Check `graphics()`.

While a mod has views, the Vulkan renderer draws on the game's thread: the
port then submits to the queue only from there, where the mod's callbacks
run, so OpenXR calls that use the queue need no lock.

The player picks Vulkan with `OPENRAC_GL_API=vulkan`; OpenGL is the default on
desktops.

## Closing

`closing` is called once, before the graphics context goes: destroy the
OpenXR session and swapchains there. The views end with it.

## Limits

- A view skips the window's ambient occlusion and temporal upscaling. With
  enhanced rendering on, a view of another size than the window remakes the
  enhanced lighting's targets each frame, which costs time.
- The views' textures are found by their GL name: keep a texture alive while
  it is used (an OpenXR swapchain's images are, for the session).
- Script mods have no views.
