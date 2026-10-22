@tool
extends RefCounted
## A moby class's own collision in its class scene (mobys/moby_<class>.tscn): the hidden
## Collision child of the scene's root, in the class's model units, as editor/moby_collision_scene.py
## writes and reads it. Its children: spheres, joint spheres, upright cylinders, capsules and
## triangle parts (Mesh_XX). Collision made from the class's model here (the box, sphere and
## cylinder modes as nodes, the mesh mode as a glTF file) mirrors editor/moby_collision.py's
## generate(). No UI.
##
## The collision belongs to the class: an edit is every moby of that class in that level.

const Surfaces = preload("surfaces.gd")
const LevelFaces = preload("level_faces.gd")

# =============================================================================
# CONSTANTS
# =============================================================================

## Packed units per model unit: the blob's vertices are 16-bit in 1/1024.
const PACKED: int = 1024

## How far from the class's origin a vertex may be, in model units (16 bits at 1/1024).
const REACH: float = 32.0

## The vertices the faces' byte indices reach.
const MAX_VERTICES: int = 256

## Both query sets (mask bits 0 and 1): what new primitives answer.
const ALL_QUERIES: int = 3

## The translucency the collision is drawn with.
const ALPHA: float = 0.55

## Godot's Y up turned to the game's Z up: a cylinder's basis (columns).
const UPRIGHT: Basis = Basis(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, -1, 0))

## The modes of make() and mesh_gltf().
const MODES: Array[String] = ["mesh", "box", "sphere", "cylinder"]

# =============================================================================
# METHODS
# =============================================================================

## The class collision node of a class scene's root or of a placed moby (an instance of it).
static func class_layer(root: Node) -> Node3D:
	var layer := root.get_node_or_null("Collision") as Node3D if root != null else null

	# Only the export's node is the class's collision.
	if layer == null or not layer.has_meta("rc1_collision_format"):
		return null

	return layer


## The class id of a class scene's root or of a placed moby, or -1.
static func class_id(root: Node) -> int:
	var path: String = root.scene_file_path
	var name: String = path.get_file().get_basename()

	# A class scene is mobys/moby_<class>.tscn.
	if name.begins_with("moby_") and name.trim_prefix("moby_").is_valid_int():
		return name.trim_prefix("moby_").to_int()

	return int(root.get_meta("rc1_class", -1))


## The children of a class collision node, described: name, kind, radius, height, position,
## queries, surface, the node itself.
static func parts(layer: Node3D) -> Array[Dictionary]:
	var out: Array[Dictionary] = []

	for child in layer.get_children():
		var node := child as Node3D

		# Only 3D nodes are collision.
		if node == null:
			continue

		var entry: Dictionary = {"name": String(node.name), "node": node,
			"kind": String(node.get_meta("rc1_collision_kind", "mesh")),
			"surface": int(node.get_meta("rc1_surface", Surfaces.DEFAULT_SURFACE)),
			"queries": int(node.get_meta("rc1_queries", ALL_QUERIES)), "position": node.position,
			"radius": 0.0, "height": 0.0}
		var mesh: Mesh = (node as MeshInstance3D).mesh if node is MeshInstance3D else null

		# The size is the primitive mesh's, times the node's scale.
		if mesh is SphereMesh:
			entry.radius = (mesh as SphereMesh).radius * node.scale.x
		elif mesh is CylinderMesh:
			entry.radius = (mesh as CylinderMesh).top_radius * node.scale.x
			entry.height = (mesh as CylinderMesh).height * node.scale.y
		elif mesh is CapsuleMesh:
			entry.radius = (mesh as CapsuleMesh).radius * node.scale.x

		out.append(entry)

	return out


## The triangles of the class's model (every mesh under the root but the collision and the box
## marker), in the collision node's space (model units, game axes), glTF's winding.
static func model_triangles(root: Node) -> PackedVector3Array:
	var layer: Node3D = class_layer(root)
	var out := PackedVector3Array()

	# Without the collision node there is no space to put them in.
	if layer == null:
		return out

	var to_layer: Transform3D = layer.transform.affine_inverse()

	for found in root.find_children("*", "MeshInstance3D", true, false):
		var mesh_node := found as MeshInstance3D

		# The collision (or the dock's copy of it) is not the model; a marker box stands for a class
		# without one.
		if is_collision(mesh_node, root) or mesh_node.name == &"Marker" or mesh_node.mesh == null:
			continue

		var transform: Transform3D = to_layer * LevelFaces.relative(mesh_node, root)

		for surface in mesh_node.mesh.get_surface_count():
			# Only triangle surfaces have faces.
			if LevelFaces.is_triangles(mesh_node.mesh, surface):
				out.append_array(transform * LevelFaces.mesh_triangles(mesh_node.mesh, surface))

	return out


## Whether a node is part of a class collision node (the class's, or a copy the dock draws).
static func is_collision(node: Node, root: Node) -> bool:
	var current: Node = node

	while current != null and current != root:
		# The export's node and the dock's copies carry the collision format.
		if current.has_meta("rc1_collision_format"):
			return true

		current = current.get_parent()

	return false


## A material in a surface's colour, as the export draws class collision.
static func material(kind: int) -> StandardMaterial3D:
	var out := StandardMaterial3D.new()
	var colour: Color = Surfaces.colour(kind)
	out.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	out.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	out.cull_mode = BaseMaterial3D.CULL_DISABLED
	out.albedo_color = Color(colour, ALPHA)
	return out


## Collision nodes made from triangles (model units, game axes): "box" a BoxMesh around them (a
## triangle part of type byte surface), "sphere" one sphere around them, "cylinder" one upright
## cylinder around them. Not yet in a scene; the mesh mode is mesh_gltf()'s.
static func make(triangles: PackedVector3Array, mode: String, surface: int) -> Array[Node3D]:
	var out: Array[Node3D] = []

	# Nothing to wrap.
	if triangles.is_empty():
		return out

	var box := AABB(triangles[0], Vector3.ZERO)

	for p in triangles:
		box = box.expand(p)

	var node := MeshInstance3D.new()
	var centre: Vector3 = box.get_center()

	# Each mode is one node: the packer reads a BoxMesh as triangles, the others as primitives.
	match mode:
		"box":
			var mesh := BoxMesh.new()
			mesh.size = box.size.max(Vector3(0.01, 0.01, 0.01))
			mesh.material = material(surface)
			node.mesh = mesh
			node.name = "Mesh_box"
			node.position = centre
			node.set_meta("rc1_collision_kind", "mesh")
			node.set_meta("rc1_surface", surface)
		"sphere":
			var radius: float = 0.0

			for p in triangles:
				radius = maxf(radius, centre.distance_to(p))

			var mesh := SphereMesh.new()
			mesh.radius = maxf(radius, 1.0 / PACKED)
			mesh.height = mesh.radius * 2.0
			mesh.material = material(Surfaces.DEFAULT_SURFACE)
			node.mesh = mesh
			node.name = "Sphere_new"
			node.position = centre
			primitive_metadata(node, "sphere")
		"cylinder":
			var base := Vector3(centre.x, centre.y, box.position.z)
			var radius: float = 0.0

			for p in triangles:
				radius = maxf(radius, Vector2(p.x - base.x, p.y - base.y).length())

			var mesh := CylinderMesh.new()
			mesh.top_radius = maxf(radius, 1.0 / PACKED)
			mesh.bottom_radius = mesh.top_radius
			mesh.height = maxf(box.size.z, 1.0 / PACKED)
			mesh.material = material(Surfaces.DEFAULT_SURFACE)
			node.mesh = mesh
			node.name = "Cylinder_new"
			node.transform = Transform3D(UPRIGHT, base + Vector3(0, 0, mesh.height / 2.0))
			primitive_metadata(node, "cylinder")
		_:
			node.free()
			return out

	out.append(node)
	return out


## The metadata a new primitive carries: its kind, no surface, both query sets, no rc1_index.
static func primitive_metadata(node: Node, kind: String) -> void:
	node.set_meta("rc1_collision_kind", kind)
	node.set_meta("rc1_surface", Surfaces.DEFAULT_SURFACE)
	node.set_meta("rc1_queries", ALL_QUERIES)


## Whether triangles (model units) stay within the 32 units the blob's vertices reach.
static func fits(triangles: PackedVector3Array) -> bool:
	for p in triangles:
		# One vertex too far and the packer refuses the class.
		if absf(p.x) > REACH or absf(p.y) > REACH or absf(p.z) > REACH:
			return false

	return true


## Triangles merged on a grid until at most 256 vertices are left (moby_collision.cluster), in
## packed units: the coarsest step reached by doubling; collapsed triangles are dropped.
static func cluster(triangles: PackedVector3Array) -> PackedVector3Array:
	var step: int = 1

	while true:
		var out := PackedVector3Array()
		var seen: Dictionary = {}
		var points: Dictionary = {}

		for i in range(0, triangles.size() - 2, 3):
			var corners: Array[Vector3i] = []

			for k in 3:
				var p: Vector3 = triangles[i + k] * PACKED
				corners.append(Vector3i(snappedi(p.x, step), snappedi(p.y, step), snappedi(p.z, step)))

			var key: String = LevelFaces.key_of(LevelFaces.record_of(corners, 0), LevelFaces.NO_WALL)

			# Merged corners can flatten a triangle or make two the same.
			if LevelFaces.is_degenerate(corners) or seen.has(key):
				continue

			seen[key] = true

			for p in corners:
				points[p] = true
				out.append(Vector3(p) / PACKED)

		# Few enough vertices, or nothing left to merge.
		if points.size() <= MAX_VERTICES or out.is_empty():
			return out

		step *= 2

	return PackedVector3Array()


## A glTF file (text, the buffer inline) of triangles (model units, glTF's winding) with one
## material rc1_surface_XX: the mesh mode's triangle part, which the packer reads as a Mesh_XX.
static func mesh_gltf(triangles: PackedVector3Array, surface: int) -> String:
	var floats := PackedFloat32Array()
	var low := Vector3(INF, INF, INF)
	var high := -low

	for p in triangles:
		floats.append_array(PackedFloat32Array([p.x, p.y, p.z]))
		low = low.min(p)
		high = high.max(p)

	var data: PackedByteArray = floats.to_byte_array()
	var colour: Color = Surfaces.colour(surface)
	var doc: Dictionary = {
		"asset": {"version": "2.0", "generator": "OpenRAC collision dock"},
		"extensionsUsed": ["KHR_materials_unlit"],
		"scene": 0, "scenes": [{"nodes": [0]}],
		"nodes": [{"name": "Mesh_%02x" % surface, "mesh": 0}],
		"meshes": [{"name": "Mesh_%02x" % surface, "primitives": [{"attributes": {"POSITION": 0}, "material": 0,
			"mode": 4}]}],
		"materials": [{"name": "rc1_surface_%02x" % surface, "doubleSided": true, "alphaMode": "BLEND",
			"pbrMetallicRoughness": {"baseColorFactor": [pow(colour.r, 2.2), pow(colour.g, 2.2), pow(colour.b, 2.2),
				ALPHA], "metallicFactor": 0}, "extensions": {"KHR_materials_unlit": {}}}],
		"accessors": [{"bufferView": 0, "componentType": 5126, "count": triangles.size(), "type": "VEC3",
			"min": [low.x, low.y, low.z], "max": [high.x, high.y, high.z]}],
		"bufferViews": [{"buffer": 0, "byteLength": data.size()}],
		"buffers": [{"byteLength": data.size(),
			"uri": "data:application/octet-stream;base64," + Marshalls.raw_to_base64(data)}],
	}
	return JSON.stringify(doc)


## A child name not yet used under a node: base, else base_2, base_3...
static func free_name(parent: Node, base: String) -> String:
	var name: String = base
	var n: int = 2

	while parent.has_node(NodePath(name)):
		name = "%s_%d" % [base, n]
		n += 1

	return name
