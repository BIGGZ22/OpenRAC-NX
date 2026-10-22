@tool
extends RefCounted
## What is known of the collision's type bytes, and their colours (editor/collision.py's colour()).
## A type byte: bits 0-4 the surface id, bits 5-6 the footstep sound class, bit 7 keeps the face
## out of some of the game's queries. The game has no table of surfaces: the names below are
## inferences from how its code tests them (editor/README.md, Collision).

# =============================================================================
# CONSTANTS
# =============================================================================

## The surface id without anything special about it, most of every level.
const DEFAULT_SURFACE: int = 0x1f

## The type byte's bits for the surface id.
const SURFACE_MASK: int = 0x1f

## How far the footstep sound class is shifted in the type byte.
const SOUND_SHIFT: int = 5

## The query exclusion bit.
const EXCLUDED: int = 0x80

## The colour the hero walls are drawn in (they have no type byte).
const HERO_COLOUR: Color = Color(1.0, 0.85, 0.2)

## What is known of each surface id; ids not listed have no known meaning.
const KNOWN: Dictionary = {
	0x1f: "no special surface; most of every level",
	0: "water surface (the ground probe looks for the floor below it)",
	8: "common; the game's wall and ledge checks test for it",
	9: "common; the game's wall and ledge checks test for it",
	10: "common; the game's wall and ledge checks test for it",
	12: "common; the game's wall and ledge checks test for it",
	11: "the ground snap is skipped (some special floor)",
	13: "liquid-like (level 12 only)",
}

## The golden-ratio step that spreads the other ids around the colour wheel.
const HUE_STEP: float = 0.381966

# =============================================================================
# METHODS
# =============================================================================

## The display colour of a type byte's surface id, as the tiles' materials have it.
static func colour(kind: int) -> Color:
	var surface: int = kind & SURFACE_MASK

	# The default surface is grey and water blue; the others take a hue from their id.
	if surface == DEFAULT_SURFACE:
		return Color(0.55, 0.55, 0.55)
	if surface == 0:
		return Color(0.2, 0.45, 1.0)

	return Color.from_hsv(fmod(surface * HUE_STEP, 1.0), 0.8, 0.95)


## The surface id of a type byte.
static func surface_of(kind: int) -> int:
	return kind & SURFACE_MASK


## The footstep sound class of a type byte, 0 to 3.
static func sound_of(kind: int) -> int:
	return (kind >> SOUND_SHIFT) & 3


## Whether a type byte keeps its face out of some of the game's queries.
static func is_excluded(kind: int) -> bool:
	return kind & EXCLUDED != 0


## A type byte from its three parts.
static func type_byte(surface: int, sound: int, excluded: bool) -> int:
	return (surface & SURFACE_MASK) | ((sound & 3) << SOUND_SHIFT) | (EXCLUDED if excluded else 0)


## What is known of a surface id, in a few words.
static func describe(surface: int) -> String:
	return KNOWN.get(surface, "meaning unknown")


## A type byte in words: its surface id, sound class and exclusion.
static func spell(kind: int) -> String:
	return "0x%02x: surface %d, sound class %d%s" % [kind, surface_of(kind), sound_of(kind),
		", excluded from some queries" if is_excluded(kind) else ""]
