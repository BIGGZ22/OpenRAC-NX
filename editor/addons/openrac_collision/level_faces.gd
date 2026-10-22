@tool
extends RefCounted
## A level's collision as the Collision dock edits it: the faces of Game/Collision's and
## Game/HeroWalls' nodes with the dock's edits applied, picking, selection helpers and the edits.
## No UI: the dock and the headless test (editor/test_collision_dock.gd) both use it.
##
## Points are game units in the collision layers' space (the Game node's: Z up). The tiles are
## never changed; the edits live on Game/Collision as metadata, saved with the scene, and the
## packer applies them (editor/collision_scene.py, layer_edits):
##
##   rc1_removed       ten ints per face: its corners in 1/64 game units, glTF's winding, then its type byte
##   rc1_added         the same form, on the game's grid (1/16 across, 1/64 up)
##   rc1_hero_removed  ten ints per face: the hero wall's number, then its corners
##
## A repaint or a move is the face removed and added again; an added face edited again is
## changed in place, and one that comes back to a removed face cancels both.

const Surfaces = preload("surfaces.gd")

# =============================================================================
# VARIABLES
# =============================================================================

## The level's Game node, whose space the points are in.
var game: Node3D

## Game/Collision, which carries the edits.
var layer: Node3D

## Game/HeroWalls, or null in a level without hero walls.
var hero_layer: Node3D

## The faces the nodes hold, ten ints each (corners in 1/64 units, type byte).
var base: PackedInt32Array = PackedInt32Array()

## Each base face's hero wall: its number, NO_WALL for the level's collision, or ADDED_WALL.
var base_walls: PackedInt32Array = PackedInt32Array()

## The removed faces of the level's collision: key -> record.
var removed: Dictionary = {}

## The removed faces of hero walls: key -> record (wall number first).
var hero_removed: Dictionary = {}

## The added faces, a PackedInt32Array record each.
var added: Array[PackedInt32Array] = []

## The faces as edited, ten ints each.
var records: PackedInt32Array = PackedInt32Array()

## Each face's hero wall (as base_walls).
var walls: PackedInt32Array = PackedInt32Array()

## Each face's origin: its base index, or -1 - its index in added.
var origins: PackedInt32Array = PackedInt32Array()

## Each face's key, the selection's currency.
var keys: PackedStringArray = PackedStringArray()

## Key -> face index.
var index_of: Dictionary = {}

## The selected faces: key -> true. Keys outlive an edit, indices do not.
var selection: Dictionary = {}

## Picking buckets: Vector2i column -> Array of faces.
var buckets: Dictionary = {}

## Each bucket's bounds.
var bucket_bounds: Dictionary = {}

## Faces of nodes the packer cannot read either (an ArrayMesh, a scene), left out.
var unread: PackedStringArray = PackedStringArray()

# =============================================================================
# CONSTANTS
# =============================================================================

## Positions are stored in 1/64 game units.
const UNIT: int = 64

## A face record's ints: nine corner coordinates and the type byte.
const RECORD: int = 10

## A base face of the level's collision.
const NO_WALL: int = -1

## A face of a hero wall the modder added (not HeroWall_NNN): the dock leaves it alone.
const ADDED_WALL: int = -2

## The side of a picking bucket, in game units.
const BUCKET: float = 16.0

## A collision cell's side in 1/64 units (4 game units).
const CELL: int = 256

## The game's limit of vertices in one cell (collision.validate).
const CELL_LIMIT: int = 255

## What the simplification aims at per cell, below the limit: a large face is cut in pieces at
## packing, which adds corners.
const CELL_TARGET: int = 230

## The game answers queries inside 0 to 1024 units on each axis only.
const WORLD: int = 1024 * UNIT

## The metadata names of the edits.
const EDIT_KEYS: Array[String] = ["rc1_removed", "rc1_added", "rc1_hero_removed"]

## A type byte in a material name, as the export names them.
const MATERIAL_PATTERN: String = "rc1_surface_([0-9a-fA-F]{2})"

# =============================================================================
# METHODS
# =============================================================================

## Reads the collision of a level scene's root; false when it has no editable collision.
func load_level(root: Node) -> bool:
	game = root.get_node_or_null("Game") as Node3D if root != null else null

	# A scene that is not an extracted Ratchet & Clank level (or an older export) has none.
	if game == null:
		return false

	layer = game.get_node_or_null("Collision") as Node3D

	if layer == null or not layer.has_meta("rc1_collision"):
		return false

	hero_layer = game.get_node_or_null("HeroWalls") as Node3D
	reload()
	return true


## Reads the nodes' faces and the stored edits again (after any change to the scene).
func reload() -> void:
	base = PackedInt32Array()
	base_walls = PackedInt32Array()
	unread = PackedStringArray()

	for child in layer.get_children():
		gather(child, layer, NO_WALL)

	# Levels without hero groups have no hero walls.
	if hero_layer != null:
		for child in hero_layer.get_children():
			var name: String = String(child.name)
			var number: int = name.trim_prefix("HeroWall_").to_int() if name.begins_with("HeroWall_") else ADDED_WALL
			gather(child, hero_layer, number)

	read_edits()


# --- Reading the nodes ---

## Adds the faces of one child of a collision layer to base, as the packer reads that node.
func gather(node: Node, top: Node, wall: int) -> void:
	var forced: int = int(node.get_meta("rc1_surface", -1))
	var is_hero: bool = wall != NO_WALL

	# An instance of a glTF file: every mesh in it, with its materials' type bytes.
	if node.scene_file_path.get_extension().to_lower() in ["glb", "gltf"]:
		for found in node.find_children("*", "MeshInstance3D", true, false):
			var mesh_node := found as MeshInstance3D

			# A mesh instance without a mesh holds nothing.
			if mesh_node.mesh == null:
				continue

			var to_layer: Transform3D = relative(mesh_node, top)

			for surface in mesh_node.mesh.get_surface_count():
				var kind: int = forced if forced >= 0 else material_kind(mesh_node.mesh.surface_get_material(surface))
				add_base(to_layer * mesh_triangles(mesh_node.mesh, surface), kind, wall, is_hero)
		return

	# A box, plane or quad the modder added: its triangles as the packer makes them.
	if node is MeshInstance3D and primitive_triangles((node as MeshInstance3D).mesh).size() > 0:
		var kind: int = forced if forced >= 0 else Surfaces.DEFAULT_SURFACE
		add_base(relative(node, top) * primitive_triangles((node as MeshInstance3D).mesh), kind, wall, is_hero)
		return

	# Anything else with a mesh is not read by the packer either.
	if node is MeshInstance3D or not node.scene_file_path.is_empty():
		unread.append(String(node.name))


## Adds triangles (game units, glTF's winding) as base faces: on the leaves' grid for the level's
## collision (a flattened one is left out, as the packer leaves it), at 1/64 for a hero wall.
func add_base(triangles: PackedVector3Array, kind: int, wall: int, is_hero: bool) -> void:
	for i in range(0, triangles.size() - 2, 3):
		var corners: Array[Vector3i] = []

		for k in 3:
			corners.append(hero_point(triangles[i + k]) if is_hero else grid_point(triangles[i + k]))

		# The packer drops a triangle the grid turns into a line; hero walls keep theirs.
		if not is_hero and is_degenerate(corners):
			continue

		base.append_array(record_of(corners, Surfaces.DEFAULT_SURFACE if is_hero else kind))
		base_walls.append(wall)


## The type byte a material's name gives (rc1_surface_XX), else the default surface.
static func material_kind(material: Material) -> int:
	# Surfaces without a named material get the default, as the packer gives them.
	if material == null:
		return Surfaces.DEFAULT_SURFACE

	var pattern := RegEx.create_from_string(MATERIAL_PATTERN)
	var found: RegExMatch = pattern.search(material.resource_name)
	return found.get_string(1).hex_to_int() if found != null else Surfaces.DEFAULT_SURFACE


## A node's transform relative to an ancestor (that ancestor's own transform left out).
static func relative(node: Node, top: Node) -> Transform3D:
	var result := Transform3D.IDENTITY
	var current: Node = node

	while current != null and current != top:
		# Only 3D nodes have a transform; others pass their parent's through.
		if current is Node3D:
			result = (current as Node3D).transform * result

		current = current.get_parent()

	return result


## A surface's triangles in glTF's winding: Godot turns the import's triangles the other way.
static func mesh_triangles(mesh: Mesh, surface: int) -> PackedVector3Array:
	var arrays: Array = mesh.surface_get_arrays(surface)
	var vertices: PackedVector3Array = arrays[Mesh.ARRAY_VERTEX]
	var indices: PackedInt32Array = arrays[Mesh.ARRAY_INDEX] if arrays[Mesh.ARRAY_INDEX] != null else PackedInt32Array()
	var out := PackedVector3Array()

	# A surface without indices lists three vertices per triangle.
	if indices.is_empty():
		indices.resize(vertices.size())

		for i in vertices.size():
			indices[i] = i

	for i in range(0, indices.size() - 2, 3):
		out.append(vertices[indices[i]])
		out.append(vertices[indices[i + 2]])
		out.append(vertices[indices[i + 1]])

	return out


## Whether a mesh's surface is triangles (a primitive mesh's always are).
static func is_triangles(mesh: Mesh, surface: int) -> bool:
	# Only an ArrayMesh says; Godot's primitive meshes are triangles.
	if mesh is ArrayMesh:
		return (mesh as ArrayMesh).surface_get_primitive_type(surface) == Mesh.PRIMITIVE_TRIANGLES

	return true


## A BoxMesh, PlaneMesh or QuadMesh as the packer's triangles (collision_scene.primitive_triangles),
## wound outward in glTF's way; empty for any other mesh.
static func primitive_triangles(mesh: Mesh) -> PackedVector3Array:
	var out := PackedVector3Array()

	# A box: two triangles per side.
	if mesh is BoxMesh:
		var half: Vector3 = (mesh as BoxMesh).size / 2.0

		for axis in 3:
			for side in [-1.0, 1.0]:
				var u: int = (axis + 1) % 3
				var v: int = (axis + 2) % 3
				var corners: Array[Vector3] = []

				for pair in [[-1.0, -1.0], [1.0, -1.0], [1.0, 1.0], [-1.0, 1.0]]:
					var p := Vector3.ZERO
					p[axis] = side * half[axis]
					p[u] = pair[0] * half[u]
					p[v] = pair[1] * half[v]
					corners.append(p)

				# The far sides turn the other way, so that every side faces out.
				if side < 0:
					corners.reverse()

				out.append_array(PackedVector3Array([corners[0], corners[1], corners[2], corners[0], corners[2],
					corners[3]]))
		return out

	# A plane or a quad (a QuadMesh is a PlaneMesh facing Z): one quad.
	if mesh is PlaneMesh:
		var plane := mesh as PlaneMesh
		var facing: int = plane.orientation
		var axes: Array = {0: [2, 1], 1: [0, 2], 2: [0, 1]}[facing]
		var quad: Array[Vector3] = []

		for pair in [[-1.0, -1.0], [1.0, -1.0], [1.0, 1.0], [-1.0, 1.0]]:
			var p: Vector3 = plane.center_offset
			p[axes[0]] += pair[0] * plane.size.x / 2.0
			p[axes[1]] += pair[1] * plane.size.y / 2.0
			quad.append(p)

		# Counter-clockwise seen from the normal's side: FACE_X's and FACE_Y's axes turn the other way.
		if facing in [0, 1]:
			quad.reverse()

		out.append_array(PackedVector3Array([quad[0], quad[1], quad[2], quad[0], quad[2], quad[3]]))

	return out


# --- Points and records ---

## A point in game units as the leaves store it, in 1/64 units: X and Y to 1/16, Z to 1/64.
static func grid_point(p: Vector3) -> Vector3i:
	return Vector3i(roundi(p.x * 16.0) * 4, roundi(p.y * 16.0) * 4, roundi(p.z * UNIT))


## A point in game units as a hero wall stores it: 1/64 on every axis.
static func hero_point(p: Vector3) -> Vector3i:
	return Vector3i(roundi(p.x * UNIT), roundi(p.y * UNIT), roundi(p.z * UNIT))


## Whether three corners make a line or a point.
static func is_degenerate(corners: Array[Vector3i]) -> bool:
	var a: Vector3i = corners[1] - corners[0]
	var b: Vector3i = corners[2] - corners[0]
	return (a.y * b.z - a.z * b.y == 0 and a.z * b.x - a.x * b.z == 0
		and a.x * b.y - a.y * b.x == 0)


## Whether a comes before b, axis by axis (as Python compares tuples).
static func is_before(a: Vector3i, b: Vector3i) -> bool:
	# The first axis that differs decides.
	if a.x != b.x:
		return a.x < b.x
	if a.y != b.y:
		return a.y < b.y

	return a.z < b.z


## A face record: its corners turned to start at the least one (one record per triangle, as the
## packer's canonical()), then its type byte.
static func record_of(corners: Array[Vector3i], kind: int) -> PackedInt32Array:
	var first: int = 0

	for k in [1, 2]:
		# The least corner leads; the turn keeps the winding.
		if is_before(corners[k], corners[first]):
			first = k

	var out := PackedInt32Array()

	for k in 3:
		var p: Vector3i = corners[(first + k) % 3]
		out.append_array([p.x, p.y, p.z])

	out.append(kind)
	return out


## The corners of the record at a face index in a record array.
static func corners_of(array: PackedInt32Array, face: int) -> Array[Vector3i]:
	var at: int = face * RECORD
	return [Vector3i(array[at], array[at + 1], array[at + 2]), Vector3i(array[at + 3], array[at + 4], array[at + 5]),
		Vector3i(array[at + 6], array[at + 7], array[at + 8])]


## The key of a record (and its hero wall): what identifies a face across edits.
static func key_of(record: PackedInt32Array, wall: int) -> String:
	var text: String = str(record)
	return text if wall == NO_WALL else "h%d:%s" % [wall, text]


# --- The edits ---

## Reads the edits stored on Game/Collision.
func read_edits() -> void:
	removed = {}
	hero_removed = {}
	added = []
	var stored: Dictionary = stored_edits_of(layer)

	for record in split(stored.rc1_removed):
		removed[key_of(record, NO_WALL)] = record

	for record in split(stored.rc1_hero_removed):
		# A hero face's key has the default surface for its type byte, as its face has.
		var face: PackedInt32Array = record.slice(1)
		face.append(Surfaces.DEFAULT_SURFACE)
		hero_removed[key_of(face, record[0])] = record

	for record in split(stored.rc1_added):
		added.append(record)

	rebuild()


## The edits as stored on a collision layer: the three arrays, empty when absent.
static func stored_edits_of(node: Node) -> Dictionary:
	var out: Dictionary = {}

	for name in EDIT_KEYS:
		out[name] = PackedInt32Array(node.get_meta(name, PackedInt32Array()))

	return out


## The edits as they are now, in the stored form.
func stored_edits() -> Dictionary:
	var gone := PackedInt32Array()
	var hero_gone := PackedInt32Array()
	var fresh := PackedInt32Array()

	for key in sorted(removed.keys()):
		gone.append_array(removed[key])

	for key in sorted(hero_removed.keys()):
		hero_gone.append_array(hero_removed[key])

	for record in added:
		fresh.append_array(record)

	return {"rc1_removed": gone, "rc1_added": fresh, "rc1_hero_removed": hero_gone}


## Puts edits (the stored form) on a collision layer; an empty one leaves no metadata, so a scene
## whose edits were all undone is the export's again.
static func write_edits(node: Node, edits: Dictionary) -> void:
	for name in EDIT_KEYS:
		var values: PackedInt32Array = edits.get(name, PackedInt32Array())

		# No metadata rather than an empty array: an untouched scene stays as exported.
		if values.is_empty():
			if node.has_meta(name):
				node.remove_meta(name)
		else:
			node.set_meta(name, values)


## Records of ten ints from a flat array.
static func split(values: PackedInt32Array) -> Array[PackedInt32Array]:
	var out: Array[PackedInt32Array] = []

	for at in range(0, values.size() - RECORD + 1, RECORD):
		out.append(values.slice(at, at + RECORD))

	return out


## Keys in a fixed order, so the stored arrays do not churn.
static func sorted(values: Array) -> Array:
	var copy: Array = values.duplicate()
	copy.sort()
	return copy


## Makes the faces as edited, the key index and the picking buckets again.
func rebuild() -> void:
	normalise()
	records = PackedInt32Array()
	walls = PackedInt32Array()
	origins = PackedInt32Array()
	keys = PackedStringArray()
	index_of = {}

	for face in base_walls.size():
		var record: PackedInt32Array = base.slice(face * RECORD, (face + 1) * RECORD)
		var wall: int = base_walls[face]
		var key: String = key_of(record, wall)

		# Removed faces are gone, and a face two nodes share is one face (the packer's set).
		if removed.has(key) or hero_removed.has(key) or index_of.has(key):
			continue

		append_face(record, wall, face, key)

	for i in added.size():
		var key: String = key_of(added[i], NO_WALL)

		# normalise() already dropped duplicates; this keeps the index exact regardless.
		if not index_of.has(key):
			append_face(added[i], NO_WALL, -1 - i, key)

	for key in selection.keys():
		# Selected faces an edit took away leave the selection.
		if not index_of.has(key):
			selection.erase(key)

	make_buckets()


## Appends one face to the edited arrays.
func append_face(record: PackedInt32Array, wall: int, origin: int, key: String) -> void:
	index_of[key] = walls.size()
	records.append_array(record)
	walls.append(wall)
	origins.append(origin)
	keys.append(key)


## Drops an added face that is a face the nodes still have, and an added face that is a removed
## one cancels both: what an undone repaint or a move back leaves.
func normalise() -> void:
	var present: Dictionary = {}

	for face in base_walls.size():
		# Only the level's collision gets faces added.
		if base_walls[face] == NO_WALL:
			present[key_of(base.slice(face * RECORD, (face + 1) * RECORD), NO_WALL)] = true

	var kept: Array[PackedInt32Array] = []
	var seen: Dictionary = {}

	for record in added:
		var key: String = key_of(record, NO_WALL)

		# Back where it was: neither edit is needed.
		if removed.has(key):
			removed.erase(key)
			continue

		# Already there, or added twice: one face.
		if (present.has(key) and not removed.has(key)) or seen.has(key):
			continue

		seen[key] = true
		kept.append(record)

	added = kept


## The number of faces as edited.
func face_count() -> int:
	return walls.size()


## Whether anything is edited.
func has_edits() -> bool:
	return not (removed.is_empty() and hero_removed.is_empty() and added.is_empty())


## A face's corners in game units.
func face_points(face: int) -> PackedVector3Array:
	var out := PackedVector3Array()

	for p in corners_of(records, face):
		out.append(Vector3(p) / UNIT)

	return out


## A face's type byte (hero walls have none: the default surface).
func face_kind(face: int) -> int:
	return records[face * RECORD + 9]


## Whether a face belongs to a hero wall.
func is_hero(face: int) -> bool:
	return walls[face] != NO_WALL


# --- Picking and selecting ---

## Sorts the faces into columns of BUCKET units, for picking.
func make_buckets() -> void:
	buckets = {}
	bucket_bounds = {}

	for face in face_count():
		var points: PackedVector3Array = face_points(face)
		var middle: Vector3 = (points[0] + points[1] + points[2]) / 3.0
		var column := Vector2i(floori(middle.x / BUCKET), floori(middle.y / BUCKET))
		var box := AABB(points[0], Vector3.ZERO).expand(points[1]).expand(points[2])

		# The first face of a column starts its bounds.
		if not buckets.has(column):
			buckets[column] = []
			bucket_bounds[column] = box

		buckets[column].append(face)
		bucket_bounds[column] = (bucket_bounds[column] as AABB).merge(box)


## The face a ray (game space) meets first, or -1; hit, when given, gets the point.
func pick(from: Vector3, direction: Vector3, hit: Array = []) -> int:
	var best: int = -1
	var nearest: float = INF

	for column in buckets:
		var box: AABB = (bucket_bounds[column] as AABB).grow(0.01)

		# A column the ray misses has nothing to hit.
		if box.intersects_ray(from, direction) == null and not box.has_point(from):
			continue

		for face in buckets[column]:
			var points: PackedVector3Array = face_points(face)
			var at = Geometry3D.ray_intersects_triangle(from, direction, points[0], points[1], points[2])

			# Faces are hit from either side here: the game's are one-sided, the editor's are not.
			if at == null:
				at = Geometry3D.ray_intersects_triangle(from, direction, points[0], points[2], points[1])

			if at != null and from.distance_to(at) < nearest:
				nearest = from.distance_to(at)
				best = face
				hit.assign([at])

	return best


## Selects faces: in place of the selection, or added to it (toggled when only one is clicked).
func select(faces: PackedInt32Array, is_adding: bool) -> void:
	# A plain click starts again.
	if not is_adding:
		selection = {}

	for face in faces:
		var key: String = keys[face]

		# Shift-clicking a selected face takes it back out.
		if is_adding and faces.size() == 1 and selection.has(key):
			selection.erase(key)
		else:
			selection[key] = true


## The selected faces' indices.
func selected() -> PackedInt32Array:
	var out := PackedInt32Array()

	for key in selection:
		out.append(index_of[key])

	out.sort()
	return out


## The faces connected to the given ones through shared corners, with the same type byte and on
## the same layer (the level's collision or one hero wall).
func connected(faces: PackedInt32Array) -> PackedInt32Array:
	var by_point: Dictionary = {}

	for face in face_count():
		for p in corners_of(records, face):
			# Faces meet at corners; each corner lists its faces.
			if not by_point.has(p):
				by_point[p] = []

			by_point[p].append(face)

	var found: Dictionary = {}
	var todo: Array[int] = []

	for face in faces:
		found[face] = true
		todo.append(face)

	while not todo.is_empty():
		var face: int = todo.pop_back()

		for p in corners_of(records, face):
			for other in by_point[p]:
				# Only faces of the same kind and layer spread the selection.
				if found.has(other) or face_kind(other) != face_kind(face) or walls[other] != walls[face]:
					continue

				found[other] = true
				todo.append(other)

	var out := PackedInt32Array(found.keys())
	out.sort()
	return out


## The faces whose three corners are inside a box (game space): an object's baked collision.
func faces_within(box: AABB) -> PackedInt32Array:
	var out := PackedInt32Array()

	for face in face_count():
		var points: PackedVector3Array = face_points(face)

		# Every corner inside: the face is the object's, not the ground it stands on.
		if box.has_point(points[0]) and box.has_point(points[1]) and box.has_point(points[2]):
			out.append(face)

	return out


## The faces whose middle a projection (game point -> screen point, or null behind the camera)
## puts inside a rectangle: the dock's box select.
func faces_in_rect(project: Callable, rect: Rect2) -> PackedInt32Array:
	var out := PackedInt32Array()

	for face in face_count():
		var points: PackedVector3Array = face_points(face)
		var screen = project.call((points[0] + points[1] + points[2]) / 3.0)

		# Faces behind the camera project to nowhere.
		if screen != null and rect.has_point(screen):
			out.append(face)

	return out


## Counts of the faces as edited per type byte, and the hero walls' faces under -1.
func kinds_present() -> Dictionary:
	var out: Dictionary = {}

	for face in face_count():
		var kind: int = -1 if is_hero(face) else face_kind(face)
		out[kind] = out.get(kind, 0) + 1

	return out


# --- Edits ---

## Deletes faces; returns how many (hero faces of walls the modder added are left alone).
func delete(faces: PackedInt32Array) -> int:
	var count: int = 0
	var dropped: Dictionary = {}

	for face in faces:
		var record: PackedInt32Array = records.slice(face * RECORD, (face + 1) * RECORD)

		# An added face simply goes; a face of the nodes is recorded as removed.
		if origins[face] < 0:
			dropped[-1 - origins[face]] = true
		elif walls[face] == NO_WALL:
			removed[keys[face]] = record
		elif walls[face] >= 0:
			var stored := PackedInt32Array([walls[face]])
			stored.append_array(record.slice(0, 9))
			hero_removed[keys[face]] = stored
		else:
			continue

		count += 1

	drop_added(dropped)
	rebuild()
	return count


## Removes added faces by their indices in added.
func drop_added(dropped: Dictionary) -> void:
	var kept: Array[PackedInt32Array] = []

	for i in added.size():
		# The others keep their order.
		if not dropped.has(i):
			kept.append(added[i])

	added = kept


## Gives faces a new type byte; returns how many (hero walls have none). They stay selected.
func repaint(faces: PackedInt32Array, kind: int) -> int:
	return replace(faces, Vector3i.ZERO, kind)


## Moves faces by an offset in game units, onto the grid; returns how many. They stay selected.
func move(faces: PackedInt32Array, offset: Vector3) -> int:
	return replace(faces, grid_point(offset), -1)


## Replaces faces of the level's collision by changed ones (moved by step, in 1/64 units, with the
## type byte kind or, for -1, their own), keeping them selected; returns how many changed.
func replace(faces: PackedInt32Array, step: Vector3i, kind: int) -> int:
	var dropped: Dictionary = {}
	var fresh: Array[PackedInt32Array] = []
	var count: int = 0

	for face in faces:
		# Hero walls have no type byte and are not moved here: delete and redraw them instead.
		if is_hero(face):
			continue

		var corners: Array[Vector3i] = []

		for p in corners_of(records, face):
			corners.append(p + step)

		# A move that flattens the face on the grid would lose it.
		if is_degenerate(corners):
			continue

		var record: PackedInt32Array = record_of(corners, face_kind(face) if kind < 0 else kind)

		# An added face is changed in place; a face of the nodes is removed and added again.
		if origins[face] < 0:
			dropped[-1 - origins[face]] = true
		else:
			removed[keys[face]] = records.slice(face * RECORD, (face + 1) * RECORD)

		selection.erase(keys[face])
		fresh.append(record)
		count += 1

	drop_added(dropped)
	added.append_array(fresh)
	rebuild()

	for record in fresh:
		var key: String = key_of(record, NO_WALL)

		# A face that came back to a removed one is the node's face again.
		if index_of.has(key):
			selection[key] = true

	return count


## Adds triangles (game units, glTF's winding: counter-clockwise from the solid side) with a type
## byte, on the grid, and selects them; returns how many were added.
func add_triangles(triangles: PackedVector3Array, kind: int) -> int:
	var fresh: Array[PackedInt32Array] = []

	for i in range(0, triangles.size() - 2, 3):
		var corners: Array[Vector3i] = [grid_point(triangles[i]), grid_point(triangles[i + 1]),
			grid_point(triangles[i + 2])]

		# A sliver the grid flattens is not kept.
		if not is_degenerate(corners):
			fresh.append(record_of(corners, kind))

	added.append_array(fresh)
	rebuild()
	selection = {}

	for record in fresh:
		var key: String = key_of(record, NO_WALL)

		# A face the level already had is not added twice.
		if index_of.has(key):
			selection[key] = true

	return fresh.size()


## A floor quad (facing up) centred on a point, SIZE across in X and Y.
static func floor_quad(centre: Vector3, size: Vector2) -> PackedVector3Array:
	var h: Vector2 = size / 2.0
	var a := centre + Vector3(-h.x, -h.y, 0.0)
	var b := centre + Vector3(h.x, -h.y, 0.0)
	var c := centre + Vector3(h.x, h.y, 0.0)
	var d := centre + Vector3(-h.x, h.y, 0.0)
	return PackedVector3Array([a, b, c, a, c, d])


## A box's twelve triangles, facing out, centred on a point.
static func box_triangles(centre: Vector3, size: Vector3) -> PackedVector3Array:
	var mesh := BoxMesh.new()
	mesh.size = size
	var out := PackedVector3Array()

	for p in primitive_triangles(mesh):
		out.append(p + centre)

	return out


## The triangles of a mesh node and the meshes under it, in the collision layer's space (glTF's
## winding): what "Make collision from this mesh" takes.
func node_triangles(node: Node) -> PackedVector3Array:
	var out := PackedVector3Array()
	var found: Array = node.find_children("*", "MeshInstance3D", true, false)

	# The node itself is one of the meshes when it is a mesh instance.
	if node is MeshInstance3D:
		found.append(node)

	for item in found:
		var mesh_node := item as MeshInstance3D

		# Meshes without data have nothing to take.
		if mesh_node.mesh == null:
			continue

		var to_layer: Transform3D = relative(mesh_node, game)

		for surface in mesh_node.mesh.get_surface_count():
			# Only triangle surfaces are surfaces to stand on.
			if is_triangles(mesh_node.mesh, surface):
				out.append_array(to_layer * mesh_triangles(mesh_node.mesh, surface))

	return out


## Triangles (game units) with their corners merged on a coarser grid until no 4-unit cell they
## reach holds more than CELL_TARGET corners, counting the level's own there; the result and the
## largest count reached (above the limit when it could not get there).
func simplify(triangles: PackedVector3Array) -> Dictionary:
	var existing: Dictionary = cell_corners(records, face_count(), bounds_of(triangles))
	var step: int = 4

	while true:
		var out := PackedVector3Array()
		var seen: Dictionary = {}
		var cells: Dictionary = {}

		for i in range(0, triangles.size() - 2, 3):
			var corners: Array[Vector3i] = []

			for k in 3:
				var p: Vector3i = grid_point(triangles[i + k])
				corners.append(Vector3i(snappedi(p.x, step), snappedi(p.y, step), snappedi(p.z, maxi(step >> 2, 1))))

			var key: String = key_of(record_of(corners, 0), NO_WALL)

			# Merged corners can flatten a triangle or make two the same.
			if is_degenerate(corners) or seen.has(key):
				continue

			seen[key] = true
			count_corners(cells, corners)

			for p in corners:
				out.append(Vector3(p) / UNIT)

		var most: int = 0

		for cell in cells:
			most = maxi(most, cells[cell].size() + existing.get(cell, {}).size())

		# Under the target, or merged as far as it goes (one corner per cell).
		if most <= CELL_TARGET or step >= CELL:
			return {"triangles": out, "most": most, "step": step}

		step *= 2

	return {}


## The bounds of triangles in 1/64 units, a cell wider on each side.
static func bounds_of(triangles: PackedVector3Array) -> AABB:
	var box := AABB(triangles[0] * UNIT, Vector3.ZERO) if triangles.size() > 0 else AABB()

	for p in triangles:
		box = box.expand(p * UNIT)

	return box.grow(CELL)


## The corners of faces per 4-unit cell their bounds reach, for the cells inside a box (1/64 units).
static func cell_corners(array: PackedInt32Array, count: int, inside: AABB) -> Dictionary:
	var cells: Dictionary = {}

	for face in count:
		var corners: Array[Vector3i] = corners_of(array, face)

		# Faces far from the box do not share its cells.
		if inside.has_point(Vector3(corners[0])) or inside.has_point(Vector3(corners[1])) or inside.has_point(Vector3(corners[2])):
			count_corners(cells, corners)

	return cells


## Adds a face's corners to every cell its bounds reach (an estimate of the cells the packer puts
## it in, never fewer).
static func count_corners(cells: Dictionary, corners: Array[Vector3i]) -> void:
	var low := Vector3i(mini(corners[0].x, mini(corners[1].x, corners[2].x)),
		mini(corners[0].y, mini(corners[1].y, corners[2].y)), mini(corners[0].z, mini(corners[1].z, corners[2].z)))
	var high := Vector3i(maxi(corners[0].x, maxi(corners[1].x, corners[2].x)),
		maxi(corners[0].y, maxi(corners[1].y, corners[2].y)), maxi(corners[0].z, maxi(corners[1].z, corners[2].z)))

	for z in range(floori(low.z / float(CELL)), floori(high.z / float(CELL)) + 1):
		for y in range(floori(low.y / float(CELL)), floori(high.y / float(CELL)) + 1):
			for x in range(floori(low.x / float(CELL)), floori(high.x / float(CELL)) + 1):
				var cell := Vector3i(x, y, z)

				# Each cell keeps its own set of corners.
				if not cells.has(cell):
					cells[cell] = {}

				for p in corners:
					cells[cell][p] = true


## What the packer would refuse, estimated: cells over the vertex limit (by their faces' bounds,
## so a count can be high) and faces outside 0-1024 units. Lines of text, empty when it looks fine.
func check_limits() -> PackedStringArray:
	var out := PackedStringArray()
	var level: Array[int] = []

	for face in face_count():
		# Hero walls are not in the cells.
		if not is_hero(face):
			level.append(face)

	var flat := PackedInt32Array()

	for face in level:
		flat.append_array(records.slice(face * RECORD, (face + 1) * RECORD))

	var cells: Dictionary = cell_corners(flat, level.size(), AABB(Vector3(-1e9, -1e9, -1e9), Vector3(2e9, 2e9, 2e9)))

	for cell in sorted(cells.keys()):
		# Only the cells over the game's limit matter.
		if cells[cell].size() > CELL_LIMIT:
			out.append("cell %s (at %s units): about %d corners, %d at most" % [cell, Vector3(cell) * 4.0,
				cells[cell].size(), CELL_LIMIT])

	for face in level:
		for p in corners_of(records, face):
			# The game answers no query outside its world.
			if p.x < 0 or p.y < 0 or p.z < 0 or p.x >= WORLD or p.y >= WORLD or p.z >= WORLD:
				out.append("a face at %s is outside 0-1024 units" % (Vector3(p) / UNIT))
				break

	return out


# --- Drawing ---

## A mesh of faces in their surface colours (or one colour), for the dock's overlay.
func overlay_mesh(faces: PackedInt32Array, single: Color = Color(0, 0, 0, 0)) -> ArrayMesh:
	var vertices := PackedVector3Array()
	var colours := PackedColorArray()

	for face in faces:
		var colour: Color = single
		var points: PackedVector3Array = face_points(face)

		# Without one colour given, each face takes its surface's (hero walls their own).
		if single.a == 0.0:
			colour = Surfaces.HERO_COLOUR if is_hero(face) else Surfaces.colour(face_kind(face))

		vertices.append_array(points)
		colours.append_array(PackedColorArray([colour, colour, colour]))

	var mesh := ArrayMesh.new()

	# An empty surface is an error in Godot; no faces, no surface.
	if vertices.is_empty():
		return mesh

	var arrays: Array = []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = vertices
	arrays[Mesh.ARRAY_COLOR] = colours
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
	return mesh


## Every face as edited.
func all_faces() -> PackedInt32Array:
	var out := PackedInt32Array()
	out.resize(face_count())

	for face in face_count():
		out[face] = face

	return out
