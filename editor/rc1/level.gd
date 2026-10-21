extends Node3D
## An extracted level. Edit it as an ordinary scene; run it (F6) to fly around.
##
## Right mouse: look. WASD: move. Q/E: down/up. Shift: faster. Wheel: speed.
## F: frame the terrain. C: show or hide the collision layer. Esc: release the mouse.
## "-- --capture out.png" saves the first frame and quits.

# =============================================================================
# VARIABLES
# =============================================================================

## The fly camera, created in _ready.
var camera: Camera3D

## Movement speed in world units per second.
var speed: float = 30.0

## The middle of the terrain's bounding box.
var center: Vector3 = Vector3.ZERO

## The length of the terrain's bounding box diagonal, at least 10.
var extent: float = 100.0

# =============================================================================
# CONSTANTS
# =============================================================================

## Radians of rotation per pixel of mouse movement.
const LOOK_SENSITIVITY: float = 0.003

## How far the camera can pitch up or down, in radians.
const MAX_PITCH: float = 1.55

## Factor applied to the speed for each wheel notch.
const WHEEL_STEP: float = 1.25

## The slowest speed the wheel can set.
const MIN_SPEED: float = 0.1

## The fastest speed the wheel can set.
const MAX_SPEED: float = 2000.0

## Speed multiplier while Shift is held.
const BOOST: float = 4.0

## Where the framing view sits relative to the terrain's center, in extents.
const FRAME_OFFSET: Vector3 = Vector3(0.65, 0.55, 0.65)

# =============================================================================
# METHODS
# =============================================================================

func _ready() -> void:
	var bounds: AABB = terrain_bounds()
	center = bounds.get_center()
	extent = maxf(bounds.size.length(), 10.0)
	speed = extent * 0.12

	camera = Camera3D.new()
	camera.near = 0.05
	camera.far = maxf(4000.0, extent * 10.0)
	add_child(camera)
	camera.make_current()
	frame_terrain()

	var args: PackedStringArray = OS.get_cmdline_user_args()
	var at: int = args.find("--capture")

	# Only capture when the flag is followed by a file name.
	if at != -1 and at + 1 < args.size():
		await RenderingServer.frame_post_draw
		get_viewport().get_texture().get_image().save_png(args[at + 1])
		get_tree().quit()


## The world-space bounds of every terrain mesh.
func terrain_bounds() -> AABB:
	var bounds := AABB()
	var first: bool = true

	for node in find_children("Terrain_*", "MeshInstance3D", true, false):
		var mesh := node as MeshInstance3D
		var box: AABB = mesh.global_transform * mesh.get_aabb()

		# The empty starting box would pull the bounds to the origin.
		bounds = box if first else bounds.merge(box)
		first = false

	return bounds


## Puts the camera above one corner of the terrain, looking at its center.
func frame_terrain() -> void:
	camera.position = center + FRAME_OFFSET * extent
	camera.look_at(center)


## Shows the collision layers (the collision and the hero walls) if the collision is hidden,
## and hides them if it is shown.
func toggle_collision() -> void:
	var collision := get_node_or_null("Game/Collision") as Node3D

	# Not every level has a collision layer.
	if collision == null:
		return

	collision.visible = not collision.visible
	var hero_walls := get_node_or_null("Game/HeroWalls") as Node3D

	# Levels without hero groups have no hero walls.
	if hero_walls != null:
		hero_walls.visible = collision.visible


func _unhandled_input(event: InputEvent) -> void:
	# The wheel scales the speed.
	if event is InputEventMouseButton and event.pressed:
		if event.button_index == MOUSE_BUTTON_WHEEL_UP:
			speed = minf(speed * WHEEL_STEP, MAX_SPEED)
		elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			speed = maxf(speed / WHEEL_STEP, MIN_SPEED)

	# Holding the right button captures the mouse for looking.
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_RIGHT:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED if event.pressed else Input.MOUSE_MODE_VISIBLE
	# Mouse movement turns the camera, but only while it is captured.
	elif event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		camera.rotation.y -= event.relative.x * LOOK_SENSITIVITY
		camera.rotation.x = clampf(camera.rotation.x - event.relative.y * LOOK_SENSITIVITY, -MAX_PITCH, MAX_PITCH)
	# Keys release the mouse, reframe the view or toggle the collision layer.
	elif event is InputEventKey and event.pressed and not event.echo:
		match event.keycode:
			KEY_ESCAPE:
				Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
			KEY_F:
				frame_terrain()
			KEY_C:
				toggle_collision()


func _process(delta: float) -> void:
	var direction := Vector3(
		float(Input.is_physical_key_pressed(KEY_D)) - float(Input.is_physical_key_pressed(KEY_A)),
		float(Input.is_physical_key_pressed(KEY_E)) - float(Input.is_physical_key_pressed(KEY_Q)),
		float(Input.is_physical_key_pressed(KEY_S)) - float(Input.is_physical_key_pressed(KEY_W)))
	var boost: float = BOOST if Input.is_physical_key_pressed(KEY_SHIFT) else 1.0

	camera.position += camera.basis * direction.normalized() * speed * boost * delta
