# Space-flight canopy: source-only candidate

User reported the windshield moving separately from the ship in planet travel
and explicitly requested no game build. No Switch build was run. Existing NROs
and their matching ELFs are unchanged and do not contain this candidate.

Extracted additional flight frames from 2026102918291900 into
build/deko3d-test/480p-review/flight-detail.jpg and flight-late.jpg. In the close
flight frame, a translucent blue canopy is visible ahead of the ship's body.
This review alone does not establish whether there is also a pose/matrix bug.

Found a concrete timing inconsistency: scene_interpolation.cpp interpolates
native model instances and joint palettes between ticks. scene_hud.cpp
set_packets supplies captured effect vertices and effect quads unchanged;
there are no previous effect vertices to blend. The canopy is implemented as
separate effect geometry (see the documented ship-glass callback in
games/rac1/nonmatching/game/func_L00_002D2E60.c). Drawing an intermediate ship
pose beside a current-tick canopy can detach it during fast flight.

Candidate change:

- geometry.h world_pose_alpha forces exact captured pose for world -2 (the
  space-flight world selected by frame.cpp world_to_draw/use_level).
- SceneFrame stores pose_alpha separately from presentation alpha.
- render_scene sets it once. The camera, instance transforms, joint palettes
  and capsule shadows all use this same value.
- Other worlds retain their requested interpolation fraction. No shader pack,
  graphics driver, material, effect UV, audio or frame pacing changes.

This conservative fix retains game-tick motion during space flight rather
than interpolating only half of an attached assembly. Future smooth flight
interpolation requires previous/current effect geometry plus stable effect
identity, or a native canopy attached to the same interpolated joint palette.
Do not interpolate arbitrary effect vertices by array position across ticks.

Local frontend_geometry_test compiled and passed, including flight/non-flight
alpha selection and the existing matrix/camera/envelope checks. This is a
host unit test, not a game build or hardware confirmation. The complete game
translation units were not compiled after this source change.

On the next authorized build, repeat the close ship flyby and landing. If the
canopy remains displaced on exact-tick frames, inspect effect capture generation
versus scene capture, and the ship callback's cockpit-joint matrix. The current
candidate addresses interpolation mismatch, not proven incorrect canopy bind
coordinates or one-tick-old effect packet capture. Do not mark hardware fixed
until the visual test confirms it.
