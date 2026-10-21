--!strict
-- A script mod for OpenRAC's native port (docs/port/MODS.md, section 11). The game runs this file
-- once at start, in a sandbox of its own: `openrac` is all it sees of the game. Hooks and callbacks
-- are taken here, while it first runs.

local every = openrac.setting("every") :: number -- the manifest's setting, as the launcher set it
local ticks = 0

-- After each game tick (50 a second on PAL, 60 on NTSC-U).
openrac.on_frame(function()
	ticks += 1
	if ticks % (every * 50) == 0 then
		openrac.log(`still here after {ticks} ticks`)
	end
end)

-- Each pad report, before the game reads it: `report` is a buffer the script may change (rac1-mods'
-- button-swap shows its layout). Uncomment to see the buttons.
-- openrac.on_pad(function(port: number, report: buffer)
-- 	openrac.log(`port {port}: buttons {buffer.readu16(report, 2)}`)
-- end)

-- A hook point by name, wrapped: the game's own call goes on through `original`.
-- openrac.hook("Hero_SetPlanarVelocity", function(original, angle: number)
-- 	original(angle)
-- end)
