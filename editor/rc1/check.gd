extends SceneTree
## Check an imported project against the extractor's level.json files:
##
##   godot --headless --path PROJECT --import
##   godot --headless --path PROJECT --script res://rc1/check.gd
##
## Every placed mesh must be textured; the counts, triangles and world
## bounds of each level scene, its sky panorama and its placed mobys must
## match what was extracted. Moby meshes are counted on their own, not as
## level geometry: every class scene's model must be textured, and the
## models, triangles, textures, skeletons, bones and animations of the
## class scenes, and the meshes the placed mobys show, must match
## level.json. The collision layer (Game/Collision, hidden) is not one of
## the textured meshes either; its triangles are compared with level.json
## separately.

# =============================================================================
# CONSTANTS
# =============================================================================

## The nodes under Game that hold the gameplay data (editor/gamedata.py's GROUPS).
const GAMEPLAY: Array[String] = ["LevelSettings", "MobyGroups", "Cameras", "Sounds", "Volumes", "Paths",
	"GrindPaths"]

# =============================================================================
# METHODS
# =============================================================================

func _initialize() -> void:
	var failed: int = 0

	for name in DirAccess.get_directories_at("res://levels"):
		var info: Dictionary = JSON.parse_string(
			FileAccess.get_file_as_string("res://levels/%s/level.json" % name))
		var scene := load("res://levels/%s/%s.tscn" % [name, name]) as PackedScene

		# Without an import there is nothing to count.
		if scene == null:
			printerr("%s: scene did not load (is the project imported?)" % name)
			failed += 1
			continue

		var root: Node = scene.instantiate()
		var totals := {"instances": 0, "triangles": 0, "meshes": {}, "untextured": 0, "bounds": AABB()}
		walk(root, Transform3D.IDENTITY, totals)
		var sky: Vector2i = sky_size(root)
		var mobys: int = moby_count(root)
		var collision: int = collision_triangles(root, "Game/Collision")
		var expected_collision: int = int(info.collision.triangles) if info.has("collision") else 0
		var heroes: int = collision_triangles(root, "Game/HeroWalls")
		var expected_heroes: int = int(info.collision.get("hero_triangles", 0)) if info.has("collision") else 0
		failed += 0 if check_moby_meshes(name, root, info) else 1
		root.free()

		var low := Vector3(info.bounds[0][0], info.bounds[0][1], info.bounds[0][2])
		var high := Vector3(info.bounds[1][0], info.bounds[1][1], info.bounds[1][2])
		var box: AABB = totals.bounds

		# A level without a sky expects a zero-size panorama, and one without
		# mobys expects none.
		var expected_sky: Array = info.sky.panorama if info.has("sky") else [0, 0]
		var expected_mobys: int = int(info.mobys.instances) if info.has("mobys") else 0
		var ok: bool = (totals.instances == int(info.mesh_instances)
			and totals.triangles == int(info.triangles)
			and totals.meshes.size() == int(info.meshes)
			and totals.untextured == 0
			and collision == expected_collision
			and heroes == expected_heroes
			and sky == Vector2i(int(expected_sky[0]), int(expected_sky[1]))
			and mobys == expected_mobys
			and box.position.distance_to(low) < 0.01 and box.end.distance_to(high) < 0.01)

		print("%s: %d instances, %d meshes, %d triangles, collision %d, sky %s, %d mobys, bounds %s: %s" % [
			name, totals.instances, totals.meshes.size(), totals.triangles, collision, sky, mobys, box,
			"ok" if ok else "MISMATCH"])
		failed += 0 if ok else 1

	quit(1 if failed else 0)


## Adds the level geometry under node to totals: instances, triangles,
## distinct meshes, untextured surfaces and the world bounds. parent is the
## world transform of node's parent. Mobys and the collision layer are
## skipped; they are counted elsewhere.
func walk(node: Node, parent: Transform3D, totals: Dictionary) -> void:
	# Placed mobys are not level geometry, nor are the gameplay data's markers and volumes.
	if node.name == &"Mobys" or (node.get_parent() != null and node.get_parent().name == &"Game"
			and String(node.name) in GAMEPLAY):
		return

	# Only 3D nodes carry a transform; others pass the parent's through.
	var here: Transform3D = parent * node.transform if node is Node3D else parent

	# Layers for the editor, counted by collision_triangles().
	if node.name in [&"Collision", &"HeroWalls"] and node.get_parent().name == &"Game":
		return

	if node is MeshInstance3D:
		var mesh: Mesh = node.mesh

		for i in mesh.get_surface_count():
			totals.triangles += surface_triangles(mesh, i)
			var material := mesh.surface_get_material(i) as StandardMaterial3D

			# Every surface must carry a texture.
			if material == null or material.albedo_texture == null:
				totals.untextured += 1

		var box: AABB = here * mesh.get_aabb()

		# The empty starting box would pull the bounds to the origin.
		totals.bounds = box if totals.instances == 0 else totals.bounds.merge(box)
		totals.instances += 1
		totals.meshes[mesh.get_instance_id()] = true

	for child in node.get_children():
		walk(child, here, totals)


## The triangles in one surface of a mesh.
func surface_triangles(mesh: Mesh, surface: int) -> int:
	var arrays: Array = mesh.surface_get_arrays(surface)
	var indices: PackedInt32Array = arrays[Mesh.ARRAY_INDEX]

	# Surfaces without an index buffer list three vertices per triangle.
	return (indices.size() if not indices.is_empty() else arrays[Mesh.ARRAY_VERTEX].size()) / 3


## The size of the environment's sky panorama, or zero without one.
func sky_size(root: Node) -> Vector2i:
	var world := root.get_node_or_null("WorldEnvironment") as WorldEnvironment

	# No environment, or no sky in it.
	if world == null or world.environment == null or world.environment.sky == null:
		return Vector2i.ZERO

	var material := world.environment.sky.sky_material as PanoramaSkyMaterial

	# The sky is not a panorama, or has no image.
	if material == null or material.panorama == null:
		return Vector2i.ZERO

	return Vector2i(material.panorama.get_width(), material.panorama.get_height())


## Placed mobys: the children of Game/Mobys that are instances of a class scene.
func moby_count(root: Node) -> int:
	var mobys := root.get_node_or_null("Game/Mobys")

	# A level without mobys has no such node.
	if mobys == null:
		return 0

	var count: int = 0

	for child in mobys.get_children():
		count += 1 if child.scene_file_path.begins_with("res://") and child.has_meta("rc1_index") else 0

	return count


## The triangles of a collision layer (Game/Collision, Game/HeroWalls), which must be hidden by default.
func collision_triangles(root: Node, path: String) -> int:
	var layer := root.get_node_or_null(path) as Node3D

	# A level without collision, or without hero walls, has no such node.
	if layer == null:
		return 0

	# The layer is for the editor; it must not cover the level when it opens.
	if layer.visible:
		printerr("%s should be hidden by default" % path)
		return -1

	var total: int = 0

	for node in layer.find_children("*", "MeshInstance3D", true, false):
		var mesh: Mesh = (node as MeshInstance3D).mesh

		for i in mesh.get_surface_count():
			total += surface_triangles(mesh, i)

	return total


## Moby class scenes (mobys/*.tscn) and the meshes placed mobys show,
## against level.json's "mobys" entry.
func check_moby_meshes(level: String, root: Node, info: Dictionary) -> bool:
	var classes := {"scenes": 0, "models": 0, "triangles": 0, "untextured": 0, "textures": {},
		"skeletons": 0, "bones": 0, "animations": 0}
	var dir: String = "res://levels/%s/mobys" % level

	for file in DirAccess.get_files_at(dir):
		# Only the class scenes; the folder holds other imported files.
		if not file.ends_with(".tscn"):
			continue

		var scene := load("%s/%s" % [dir, file]) as PackedScene

		if scene == null:
			printerr("%s: %s did not load" % [level, file])
			return false

		var instance: Node = scene.instantiate()
		var totals: Dictionary = mesh_totals(instance)
		instance.free()

		classes.scenes += 1
		classes.models += 1 if totals.meshes > 0 else 0
		classes.triangles += totals.triangles
		classes.untextured += totals.untextured
		classes.textures.merge(totals.textures)
		classes.skeletons += totals.skeletons
		classes.bones += totals.bones
		classes.animations += totals.animations

	var placed := {"meshes": 0, "triangles": 0}
	var mobys := root.get_node_or_null("Game/Mobys")

	# A level without mobys has no such node.
	if mobys != null:
		for child in mobys.get_children():
			var totals: Dictionary = mesh_totals(child)
			placed.meshes += totals.meshes
			placed.triangles += totals.triangles

	var expected: Dictionary = info.get("mobys", {})
	var ok: bool = (classes.scenes == int(expected.get("classes", 0))
		and classes.models == int(expected.get("model_classes", 0))
		and classes.triangles == int(expected.get("class_triangles", 0))
		and classes.textures.size() == int(expected.get("textures", 0))
		and classes.untextured == 0
		and classes.skeletons == int(expected.get("animated_classes", 0))
		and classes.bones == int(expected.get("bones", 0))
		and classes.animations == int(expected.get("animations", 0))
		and placed.meshes == int(expected.get("mesh_instances", 0))
		and placed.triangles == int(expected.get("triangles", 0)))

	print("%s: %d moby classes, %d with models (%d triangles, %d textures), %d animated (%d bones, %d animations); %d placed meshes, %d triangles: %s" % [
		level, classes.scenes, classes.models, classes.triangles, classes.textures.size(), classes.skeletons,
		classes.bones, classes.animations, placed.meshes, placed.triangles, "ok" if ok else "MISMATCH"])
	return ok


## Meshes, triangles, untextured surfaces and albedo textures under node,
## and its skeletons, their bones and its animations; box markers are not
## meshes.
func mesh_totals(node: Node) -> Dictionary:
	var totals := {"meshes": 0, "triangles": 0, "untextured": 0, "textures": {}, "skeletons": 0, "bones": 0,
		"animations": 0}
	var pending: Array[Node] = [node]

	while not pending.is_empty():
		var current: Node = pending.pop_back()
		pending.append_array(current.get_children())

		if current is Skeleton3D:
			totals.skeletons += 1
			totals.bones += (current as Skeleton3D).get_bone_count()

		if current is AnimationPlayer:
			totals.animations += (current as AnimationPlayer).get_animation_list().size()

		# Only meshes count, and the box markers are editor aids.
		if not current is MeshInstance3D or current.name == &"Marker":
			continue

		var mesh: Mesh = current.mesh
		totals.meshes += 1

		for i in mesh.get_surface_count():
			totals.triangles += surface_triangles(mesh, i)
			var material := mesh.surface_get_material(i) as StandardMaterial3D

			# Untextured surfaces are failures; textured ones are tallied by file.
			if material == null or material.albedo_texture == null:
				totals.untextured += 1
			else:
				totals.textures[material.albedo_texture.resource_path] = true

	return totals
