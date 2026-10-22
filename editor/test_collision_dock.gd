extends SceneTree
## The Collision dock's logic (addons/openrac_collision) on a real exported level, headless:
##
##   godot --headless --path PROJECT --script /path/to/editor/test_collision_dock.gd -- LEVEL OUT.json
##
## Loads levels/level_LEVEL/level_LEVEL.tscn, picks faces by ray, selects, repaints, deletes,
## moves and adds faces, checks the stored edits survive a reload, makes collision from a mesh and
## a class's collision from its model, saves the edited scenes and writes OUT.json: what the
## packer must find (test_collision_dock.py packs the project and compares). Exits 1 on a failure.

const LevelFaces = preload("res://addons/openrac_collision/level_faces.gd")
const ClassCollision = preload("res://addons/openrac_collision/class_collision.gd")

# =============================================================================
# VARIABLES
# =============================================================================

## Failed checks, printed at the end.
var failures: PackedStringArray = PackedStringArray()

# =============================================================================
# CONSTANTS
# =============================================================================

## The type byte the repaint gives: surface 10, sound class 0, excluded.
const REPAINT: int = 0x8a

## The type byte of the added faces.
const ADDED: int = 0x0c

# =============================================================================
# METHODS
# =============================================================================

func _initialize() -> void:
	var args: PackedStringArray = OS.get_cmdline_user_args()

	# Both arguments are needed: the level and where the expectations go.
	if args.size() < 2:
		printerr("usage: -- LEVEL OUT.json")
		quit(2)
		return

	var level: String = "level_%02d" % args[0].to_int()
	var path: String = "res://levels/%s/%s.tscn" % [level, level]
	var scene: Node = (load(path) as PackedScene).instantiate()
	var info: Dictionary = JSON.parse_string(FileAccess.get_file_as_string("res://levels/%s/level.json" % level))
	var expected: Dictionary = test_level(scene, info)
	var classes: Dictionary = test_class("res://levels/%s/mobys" % level)
	expected["classes"] = classes

	# The edits into the level scene's text, written as the editor writes metadata (var_to_str is
	# its writer); a scene saved outside the editor would also inline the editable terrain.
	var text: String = FileAccess.get_file_as_string(path)
	var lines: PackedStringArray = PackedStringArray(["metadata/rc1_collision = 1"])
	var stored: Dictionary = LevelFaces.stored_edits_of(scene.get_node("Game/Collision"))

	for name in LevelFaces.EDIT_KEYS:
		# Empty arrays are not stored at all.
		if not (stored[name] as PackedInt32Array).is_empty():
			lines.append("metadata/%s = %s" % [name, var_to_str(stored[name])])

	check(text.count("metadata/rc1_collision = 1") == 1, "the scene has one collision layer")
	var file := FileAccess.open(path, FileAccess.WRITE)
	file.store_string(text.replace("metadata/rc1_collision = 1", "\n".join(lines)))
	file.close()
	scene.free()

	var out := FileAccess.open(args[1], FileAccess.WRITE)
	out.store_string(JSON.stringify(expected, " "))
	out.close()

	for failure in failures:
		printerr("FAILED: " + failure)

	print("collision dock test: %s" % ("ok" if failures.is_empty() else "%d failures" % failures.size()))
	quit(1 if not failures.is_empty() else 0)


## Records a failed check.
func check(ok: bool, what: String) -> void:
	# Passing checks are silent.
	if not ok:
		failures.append(what)


## The level's collision through the dock's logic; returns the expectations for the packer.
func test_level(scene: Node, info: Dictionary) -> Dictionary:
	var faces := LevelFaces.new()
	check(faces.load_level(scene), "the level has editable collision")
	var triangles: int = int(info.collision.triangles)
	check(faces.face_count() == triangles + int(info.collision.get("hero_triangles", 0)),
		"faces read: %d, level.json: %d" % [faces.face_count(), triangles])
	check(not faces.has_edits(), "an exported level has no edits")
	check(faces.unread.is_empty(), "every collision node is read")

	# Three faces of the default surface, far apart, and the ray that finds each.
	var chosen: Array[int] = []

	for face in faces.face_count():
		# Faces of the default surface, not hero walls; a face every 997th keeps them apart.
		if not faces.is_hero(face) and faces.face_kind(face) == 0x1f and face % 997 == 0:
			chosen.append(face)

		if chosen.size() == 3:
			break

	var keys: Array[String] = []

	for face in chosen:
		var points: PackedVector3Array = faces.face_points(face)
		var middle: Vector3 = (points[0] + points[1] + points[2]) / 3.0
		var normal: Vector3 = (points[1] - points[0]).cross(points[2] - points[0]).normalized()
		var hit: Array = []
		var picked: int = faces.pick(middle + normal * 0.05, -normal, hit)
		check(picked == face, "the ray onto face %d picks it (got %d)" % [face, picked])
		check(not hit.is_empty() and (hit[0] as Vector3).distance_to(middle) < 0.1, "the hit point is on the face")
		keys.append(faces.keys[face])

	var originals: Array = []

	for face in chosen:
		originals.append(Array(faces.records.slice(face * 10, face * 10 + 10)))

	# Selection: a click, a shift-click, a shift-click again (out), connected faces.
	faces.select(PackedInt32Array([chosen[0]]), false)
	faces.select(PackedInt32Array([chosen[1]]), true)
	check(faces.selected().size() == 2, "shift-click adds a face")
	faces.select(PackedInt32Array([chosen[1]]), true)
	check(faces.selected() == PackedInt32Array([chosen[0]]), "shift-click on a selected face takes it out")
	var spread: PackedInt32Array = faces.connected(PackedInt32Array([chosen[0]]))
	check(spread.size() >= 1 and chosen[0] in spread, "connected faces include the start")

	for face in spread:
		check(faces.face_kind(face) == 0x1f, "connected faces share the surface")

	# Repaint the first face, delete the second, move the third.
	faces.select(PackedInt32Array([chosen[0]]), false)
	check(faces.repaint(faces.selected(), REPAINT) == 1, "repaint one face")
	check(faces.selected().size() == 1 and faces.face_kind(faces.selected()[0]) == REPAINT,
		"the repainted face stays selected")
	var repainted: Array = record_list(faces, faces.selected()[0])
	check(faces.delete(PackedInt32Array([faces.index_of[keys[1]]])) == 1, "delete one face")
	check(not faces.index_of.has(keys[1]), "the deleted face is gone")
	var offset := Vector3(0.5, -0.25, 0.75)
	faces.select(PackedInt32Array([faces.index_of[keys[2]]]), false)
	check(faces.move(faces.selected(), offset) == 1, "move one face")
	check(faces.selected().size() == 1, "the moved face stays selected")
	var moved: Array = record_list(faces, faces.selected()[0])

	# Undo by hand: a repaint back to the old byte cancels the edit.
	var probe := LevelFaces.new()
	probe.load_level(scene)
	probe.repaint(PackedInt32Array([probe.index_of[keys[0]]]), REPAINT)
	probe.repaint(probe.selected(), 0x1f)
	check(not probe.has_edits(), "a repaint back leaves no edit")

	# Added faces: a floor quad above the first face and a box beside it.
	var corners: Array[Vector3i] = LevelFaces.corners_of(PackedInt32Array(originals[0]), 0)
	var above: Vector3 = Vector3(corners[0] + corners[1] + corners[2]) / 3.0 / 64.0 + Vector3(0, 0, 3)
	var added: Array = []
	check(faces.add_triangles(LevelFaces.floor_quad(above, Vector2(2, 2)), ADDED) == 2, "a quad adds two faces")
	check(faces.selected().size() == 2, "the quad is selected")
	added.append_array(Array(faces.selected()).map(func(f: int) -> Array: return record_list(faces, f)))
	check(faces.add_triangles(LevelFaces.box_triangles(above + Vector3(4, 0, 0), Vector3(1, 1, 1)), ADDED) == 12,
		"a box adds twelve faces")
	added.append_array(Array(faces.selected()).map(func(f: int) -> Array: return record_list(faces, f)))

	# Stored and read back, as the scene keeps it.
	var stored: Dictionary = faces.stored_edits()
	check(stored.rc1_removed.size() == 30 and stored.rc1_added.size() == 160, "stored: 3 removed, 16 added (%d, %d)"
		% [stored.rc1_removed.size(), stored.rc1_added.size()])
	LevelFaces.write_edits(faces.layer, stored)
	var again := LevelFaces.new()
	again.load_level(scene)
	check(again.face_count() == faces.face_count() and again.stored_edits() == stored, "the edits read back")

	# Make collision from a mesh: a tie's, simplified within the cells' limits (not kept).
	var tie: Node = scene.get_node("Game/Ties").get_child(0)
	var probe_mesh := LevelFaces.new()
	probe_mesh.load_level(scene)
	var source: PackedVector3Array = probe_mesh.node_triangles(tie)
	var made: Dictionary = probe_mesh.simplify(source)
	check(source.size() > 0 and (made.triangles as PackedVector3Array).size() > 0, "a tie's mesh makes collision")
	check(int(made.most) <= LevelFaces.CELL_LIMIT, "its cells stay under the limit (%d)" % made.most)
	var before: int = probe_mesh.check_limits().size()
	var count: int = probe_mesh.add_triangles(made.triangles, 0x1f)
	check(count > 0 and probe_mesh.check_limits().size() <= before, "the level with it is within the limits it had")

	# What the packer must find, in 1/64 units.
	return {"removed": originals, "repainted": [repainted], "moved": [moved], "added": added}


## A face's record as a plain array, for the JSON.
func record_list(faces: LevelFaces, face: int) -> Array:
	return Array(faces.records.slice(face * 10, face * 10 + 10))


## A class's collision made from its model, in its class scene, saved: the box mode on the first
## class scene with a model. Returns {class: mode} for the packer's check.
func test_class(folder: String) -> Dictionary:
	for file in DirAccess.get_files_at(folder):
		# Class scenes only, and one with a model to make collision from.
		if not file.ends_with(".tscn"):
			continue

		var path: String = "%s/%s" % [folder, file]
		var scene: Node = (load(path) as PackedScene).instantiate()
		var layer: Node3D = ClassCollision.class_layer(scene)
		var triangles: PackedVector3Array = ClassCollision.model_triangles(scene) if layer != null else PackedVector3Array()

		if triangles.is_empty():
			scene.free()
			continue

		var made: Array[Node3D] = ClassCollision.make(triangles, "box", 0x1f)
		check(made.size() == 1 and made[0].get_meta("rc1_collision_kind") == "mesh", "box mode makes one mesh node")

		for child in layer.get_children():
			layer.remove_child(child)
			child.free()

		for node in made:
			layer.add_child(node)
			node.owner = scene

		var sphere: Array[Node3D] = ClassCollision.make(triangles, "sphere", 0x1f)
		check(sphere.size() == 1 and (sphere[0] as MeshInstance3D).mesh is SphereMesh, "sphere mode makes a sphere")
		var cylinder: Array[Node3D] = ClassCollision.make(triangles, "cylinder", 0x1f)
		check(cylinder.size() == 1 and (cylinder[0] as MeshInstance3D).mesh is CylinderMesh,
			"cylinder mode makes a cylinder")

		for node in sphere + cylinder:
			node.free()

		var packed := PackedScene.new()
		check(packed.pack(scene) == OK and ResourceSaver.save(packed, path) == OK, "the class scene saves")
		scene.free()
		return {file.get_basename().trim_prefix("moby_"): "box"}

	check(false, "no class scene with a model")
	return {}
