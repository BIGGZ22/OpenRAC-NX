@tool
extends EditorPlugin
## The OpenRAC collision plugin: the Collision dock (dock.gd), face picking in the 3D view, the
## overlays that show the collision, and the undoable edits. Editor tooling only: nothing here runs
## in the game. The level's edits are kept on Game/Collision's metadata and saved with the scene
## (level_faces.gd says how; editor/collision_scene.py packs them); a class's collision is edited
## as its class scene's Collision children (class_collision.gd).

const Dock = preload("dock.gd")
const LevelFaces = preload("level_faces.gd")
const ClassCollision = preload("class_collision.gd")
const Surfaces = preload("surfaces.gd")

# =============================================================================
# VARIABLES
# =============================================================================

## The dock.
var dock: Dock

## The edited scene's root.
var scene_root: Node

## The level's collision, or null when the scene has none.
var faces: LevelFaces

## The overlay's root (not saved: it has no owner), under the level's Game node or the class scene's root.
var overlay: Node3D

## The overlay's mesh of every face.
var face_view: MeshInstance3D

## The overlay's mesh of the selected faces.
var selection_view: MeshInstance3D

## Whether the collision is shown.
var is_showing: bool = false

## Whether clicks in the 3D view pick faces.
var is_picking: bool = false

## Whether the left button is held in the 3D view while picking.
var is_pressing: bool = false

## Whether a box is being dragged.
var is_dragging: bool = false

## Where the drag started and where it is, in the view.
var drag_from: Vector2 = Vector2.ZERO

## Where the drag is now.
var drag_to: Vector2 = Vector2.ZERO

## Where added faces go (a Vector3 in layer space), or null before a click.
var place: Variant = null

## The node the editor has selected (one), or null.
var selected_node: Node

## Whether the dock is changing the history itself (no reload needed).
var is_committing: bool = false

## Waits a moment after other edits before reading the collision again.
var reload_timer: Timer

# =============================================================================
# CONSTANTS
# =============================================================================

## How far the mouse moves before a click becomes a box.
const DRAG_THRESHOLD: float = 6.0

## How translucent the faces are drawn.
const ALPHA: float = 0.55

## The selection's colour.
const SELECTED: Color = Color(1.0, 0.95, 0.1, 0.75)

## The colour of the box being dragged.
const DRAG_COLOUR: Color = Color(1.0, 0.95, 0.1, 0.9)

## How far a tie's or shrub's bounds are grown to take its baked faces, in game units.
const BOUNDS_MARGIN: float = 0.25

## The overlay's node name.
const OVERLAY_NAME: String = "OpenRACCollisionOverlay"

## The most lines a report shows.
const REPORT_LINES: int = 12

# =============================================================================
# METHODS
# =============================================================================

func _enter_tree() -> void:
	dock = Dock.new()
	dock.plugin = self
	add_dock(dock)
	set_input_event_forwarding_always_enabled()
	set_force_draw_over_forwarding_enabled()
	reload_timer = Timer.new()
	reload_timer.one_shot = true
	reload_timer.wait_time = 0.4
	reload_timer.timeout.connect(handle_reload_timeout)
	add_child(reload_timer)
	scene_changed.connect(handle_scene_changed)
	EditorInterface.get_selection().selection_changed.connect(handle_selection_changed)
	get_undo_redo().version_changed.connect(handle_version_changed)
	handle_scene_changed(EditorInterface.get_edited_scene_root())


# --- The scene ---

## Reads the edited scene's collision again and redraws.
func open_scene(root: Node) -> void:
	remove_overlay()
	scene_root = root
	faces = LevelFaces.new()

	# Only a level scene of this layout has the level's collision.
	if root == null or not faces.load_level(root):
		faces = null

	place = null
	dock.show_level(faces)
	dock.show_place(place)
	refresh_selection()
	handle_selection_changed()
	redraw()


## Removes the overlay from the scene it was in.
func remove_overlay() -> void:
	# Freed with its scene, or never made.
	if is_instance_valid(overlay):
		overlay.get_parent().remove_child(overlay)
		overlay.queue_free()

	overlay = null
	face_view = null
	selection_view = null


## The overlay's root, made on first use under the level's Game node or the class scene's root.
func overlay_root() -> Node3D:
	# One overlay per scene, made when first needed.
	if is_instance_valid(overlay):
		return overlay

	var parent: Node = faces.game if faces != null else scene_root

	# No scene is open: nowhere to draw.
	if parent == null:
		return null

	overlay = Node3D.new()
	overlay.name = OVERLAY_NAME
	parent.add_child(overlay)
	return overlay


## Draws the level's faces, the selection and the class collision that is to be seen.
func redraw() -> void:
	# A scene that was closed has nothing to draw on.
	if scene_root == null or not is_instance_valid(scene_root):
		return

	var root: Node3D = overlay_root()

	# Nothing to draw on.
	if root == null:
		return

	for child in root.get_children():
		# The class collision copies are made again each time; the two face meshes stay.
		if child != face_view and child != selection_view:
			root.remove_child(child)
			child.queue_free()

	# The level's faces: all of them when shown, and the selection always.
	if faces != null:
		root.transform = faces.layer.transform

		# The two face meshes are made once per overlay.
		if face_view == null:
			face_view = mesh_view(face_material())
			selection_view = mesh_view(selection_material())

		face_view.mesh = faces.overlay_mesh(faces.all_faces()) if is_showing else null
		selection_view.mesh = faces.overlay_mesh(faces.selected(), SELECTED)

	for layer in class_layers_to_show():
		var copy := layer.duplicate(0) as Node3D
		copy.visible = true
		root.add_child(copy)
		# The copy stands where the class's node is, seen from the overlay (under Game in a level).
		copy.transform = root.transform.affine_inverse() * LevelFaces.relative(layer, root.get_parent())


## The class collision nodes to draw: every moby's when shown, else the selected moby's, and the
## class scene's own.
func class_layers_to_show() -> Array[Node3D]:
	var out: Array[Node3D] = []

	# A class scene: its own collision node.
	if faces == null and scene_root != null:
		var own: Node3D = ClassCollision.class_layer(scene_root)

		# Shown with the toggle, or while one of its parts (or itself) is selected.
		if own != null and (is_showing or is_part(selected_node) or selected_node == own):
			out.append(own)

		return out

	var mobys: Node = faces.game.get_node_or_null("Mobys") if faces != null else null

	# A level without mobys has no class collision to show.
	if mobys == null:
		return out

	for moby in mobys.get_children():
		var layer: Node3D = ClassCollision.class_layer(moby)

		# Every moby's with the toggle, else the selected one's.
		if layer != null and (is_showing or moby == selected_node):
			out.append(layer)

	return out


## A mesh instance for the overlay.
func mesh_view(material: Material) -> MeshInstance3D:
	var view := MeshInstance3D.new()
	view.material_override = material
	view.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	overlay.add_child(view)
	return view


## The faces' material: unshaded, translucent, both sides, in the vertex colours.
func face_material() -> StandardMaterial3D:
	var material := StandardMaterial3D.new()
	material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	material.cull_mode = BaseMaterial3D.CULL_DISABLED
	material.vertex_color_use_as_albedo = true
	material.vertex_color_is_srgb = true
	material.albedo_color = Color(1, 1, 1, ALPHA)
	return material


## The selection's material: drawn over everything, so it is seen through the terrain.
func selection_material() -> StandardMaterial3D:
	var material := face_material()
	material.no_depth_test = true
	material.render_priority = 1
	material.albedo_color = Color.WHITE
	return material


## Whether a node is a part of a class scene's collision (a child of its Collision node).
func is_part(node: Node) -> bool:
	return (node != null and is_instance_valid(node) and node.get_parent() != null
		and ClassCollision.class_layer(node.get_parent().get_parent()) == node.get_parent())


# --- Selection ---

## Tells the dock about the selection and redraws it.
func refresh_selection() -> void:
	# Without the level's collision there are no faces to select.
	if faces == null:
		dock.show_selection(null, PackedInt32Array())
		return

	dock.show_selection(faces, faces.selected())
	redraw()


## Selects the faces connected to the selection.
func select_connected() -> void:
	faces.select(faces.connected(faces.selected()), true)
	refresh_selection()


## Clears the selection.
func clear_selection() -> void:
	faces.selection = {}
	refresh_selection()


## Shows or hides the collision.
func set_showing(on: bool) -> void:
	is_showing = on
	redraw()


## Starts or stops picking faces in the 3D view.
func set_picking(on: bool) -> void:
	is_picking = on
	is_pressing = false
	is_dragging = false
	refresh_selection()


## A click in the 3D view: the face under the mouse.
func pick_at(camera: Camera3D, at: Vector2, is_adding: bool, is_connected: bool) -> void:
	var to_layer: Transform3D = faces.layer.global_transform.affine_inverse()
	var from: Vector3 = to_layer * camera.project_ray_origin(at)
	var direction: Vector3 = (to_layer.basis * camera.project_ray_normal(at)).normalized()
	var hit: Array = []
	var face: int = faces.pick(from, direction, hit)

	# A click on nothing clears the selection (unless adding).
	if face < 0:
		if not is_adding:
			faces.selection = {}

		refresh_selection()
		return

	place = hit[0]
	dock.show_place(place)
	var picked := PackedInt32Array([face])
	faces.select(faces.connected(picked) if is_connected else picked, is_adding)
	refresh_selection()


## A box dragged in the 3D view: the faces whose middle is inside it.
func pick_in_box(camera: Camera3D, rect: Rect2, is_adding: bool) -> void:
	var to_world: Transform3D = faces.layer.global_transform
	var project := func(p: Vector3):
		var world: Vector3 = to_world * p
		return null if camera.is_position_behind(world) else camera.unproject_position(world)

	faces.select(faces.faces_in_rect(project, rect), is_adding)
	refresh_selection()


# --- Level edits ---

## Commits the change faces just made as one undoable action (before: the stored edits before it).
func commit(action: String, before: Dictionary) -> void:
	var after: Dictionary = faces.stored_edits()

	# Nothing changed: nothing to undo.
	if after == before:
		refresh_selection()
		return

	var history := get_undo_redo()
	is_committing = true
	history.create_action(action, UndoRedo.MERGE_DISABLE, scene_root)
	history.add_do_method(self, "apply_stored_edits", faces.layer, after)
	history.add_undo_method(self, "apply_stored_edits", faces.layer, before)
	history.commit_action()
	is_committing = false


## Puts stored edits on a collision layer and shows them (the undo history calls this).
func apply_stored_edits(layer: Node, edits: Dictionary) -> void:
	LevelFaces.write_edits(layer, edits)

	# The scene may have changed since; only the open level's faces are read again.
	if faces != null and faces.layer == layer:
		faces.read_edits()
		dock.show_level(faces)
		refresh_selection()


## Repaints the selection with a type byte.
func repaint(kind: int) -> void:
	var before: Dictionary = faces.stored_edits()
	var count: int = faces.repaint(faces.selected(), kind)
	dock.show_report("%d faces repainted to %s." % [count, Surfaces.spell(kind)])
	commit("Repaint collision faces", before)


## Deletes the selection.
func delete_selection() -> void:
	var before: Dictionary = faces.stored_edits()
	var count: int = faces.delete(faces.selected())
	dock.show_report("%d faces deleted." % count)
	commit("Delete collision faces", before)


## Moves the selection by an offset in game units.
func move_selection(offset: Vector3) -> void:
	var before: Dictionary = faces.stored_edits()
	var count: int = faces.move(faces.selected(), offset)
	dock.show_report("%d faces moved by %s (hero walls are not moved)." % [count, offset])
	commit("Move collision faces", before)


## Adds a floor quad where the last click was.
func add_quad(size: Vector2, kind: int) -> void:
	var before: Dictionary = faces.stored_edits()
	var count: int = faces.add_triangles(LevelFaces.floor_quad(place, size), kind)
	dock.show_report("A floor quad added (%d faces)." % count)
	commit("Add a collision quad", before)


## Adds a box where the last click was (resting on it).
func add_box(size: Vector3, kind: int) -> void:
	var before: Dictionary = faces.stored_edits()
	var count: int = faces.add_triangles(LevelFaces.box_triangles(place + Vector3(0, 0, size.z / 2.0), size), kind)
	dock.show_report("A box added (%d faces)." % count)
	commit("Add a collision box", before)


## Adds collision made from the selected mesh (custom level geometry), simplified to fit the cells.
func make_from_mesh(kind: int) -> void:
	var triangles: PackedVector3Array = faces.node_triangles(selected_node)

	# A node without triangle meshes has nothing to make collision from.
	if triangles.is_empty():
		dock.show_report("%s has no triangle mesh." % selected_node.name)
		return

	var made: Dictionary = faces.simplify(triangles)
	var before: Dictionary = faces.stored_edits()
	var count: int = faces.add_triangles(made.triangles, kind)
	var merged: String = (" Corners merged on a %.3f-unit grid to fit the cells." % (made.step / 64.0)
		if int(made.step) > 4 else "")
	var over: String = (" Some cells still look full (about %d corners, 255 at most): simplify the mesh "
		+ "there." % made.most) if int(made.most) > LevelFaces.CELL_LIMIT else ""
	dock.show_report("%d faces added from %s (%d triangles).%s%s" % [count, selected_node.name,
		int(triangles.size() / 3.0), merged, over])
	commit("Make collision from a mesh", before)


## Estimates what the packer would refuse.
func check_limits() -> void:
	var lines: PackedStringArray = faces.check_limits()

	# Nothing over a limit, as far as the estimate goes.
	if lines.is_empty():
		dock.show_report("Within the limits, as far as the dock can tell (the packer checks exactly).")
		return

	var shown: PackedStringArray = lines.slice(0, REPORT_LINES)

	# A long report says how much more there is.
	if lines.size() > REPORT_LINES:
		shown.append("... and %d more" % (lines.size() - REPORT_LINES))

	dock.show_report("\n".join(shown))


# --- Object (class) collision ---

## Tells the dock about the selected node's collision: a moby's class, a tie's or shrub's baked
## faces, a class scene's part.
func show_object() -> void:
	var info: Dictionary = {}
	var node: Node = selected_node

	# A placed moby: its class's collision, edited in its class scene.
	if faces != null and node != null and node.get_parent() == faces.game.get_node_or_null("Mobys"):
		var layer: Node3D = ClassCollision.class_layer(node)
		var id: int = ClassCollision.class_id(node)
		info.text = class_text(layer, id, String(node.get_meta("rc1_class_name", "")))
		info.can_open = not node.scene_file_path.is_empty()
	# The class scene itself: its parts can be edited here.
	elif faces == null and scene_root != null and ClassCollision.class_layer(scene_root) != null:
		var layer: Node3D = ClassCollision.class_layer(scene_root)
		info.text = class_text(layer, ClassCollision.class_id(scene_root),
			String(scene_root.get_meta("rc1_class_name", "")))
		info.can_make = true

		# A selected part fills the part editor.
		if is_part(node):
			for part in ClassCollision.parts(layer):
				if part.node == node:
					info.part = part

	dock.show_object(info)


## The text describing a class's collision.
func class_text(layer: Node3D, id: int, label: String) -> String:
	# Spawn points, gadgets and classes without data in the core have none.
	if layer == null:
		return "Class %d %s has no collision of its own in this level." % [id, label]

	var lines: PackedStringArray = PackedStringArray([
		"Class %d %s: %d parts. A class edit changes every moby of class %d in this level." % [id, label,
		layer.get_child_count(), id]])

	for part in ClassCollision.parts(layer):
		var shape: String = "radius %.3f" % part.radius if part.radius > 0 else "type byte 0x%02x" % part.surface
		lines.append("  %s (%s): %s at %s, queries %d" % [part.name, part.kind, shape, part.position, part.queries])

	return "\n".join(lines)


## Opens the selected moby's class scene.
func open_class_scene() -> void:
	EditorInterface.open_scene_from_path(selected_node.scene_file_path)


## Applies the part editor's values (radius, height, x, y, z, queries, surface) to the selected part.
func apply_part(values: Dictionary) -> void:
	var node := selected_node as MeshInstance3D
	var history := get_undo_redo()
	history.create_action("Edit class collision part", UndoRedo.MERGE_DISABLE, scene_root)
	var position := Vector3(values.x, values.y, values.z)
	history.add_do_property(node, "position", position)
	history.add_undo_property(node, "position", node.position)
	var mesh: Mesh = node.mesh if node != null else null
	var radius: float = values.radius / maxf(node.scale.x, 1e-6) if node != null else values.radius

	# Each shape keeps its size in its own properties; a triangle part has a type byte instead.
	if mesh is SphereMesh:
		property(history, mesh, "radius", radius)
		property(history, mesh, "height", radius * 2.0)
	elif mesh is CylinderMesh:
		property(history, mesh, "top_radius", radius)
		property(history, mesh, "bottom_radius", radius)
		property(history, mesh, "height", values.height / maxf(node.scale.y, 1e-6))
	elif mesh is CapsuleMesh:
		var capsule := mesh as CapsuleMesh
		property(history, mesh, "radius", radius)
		property(history, mesh, "height", capsule.height - 2.0 * capsule.radius + 2.0 * radius)

	# Primitives answer queries by their mask; triangle parts have a type byte.
	if String(selected_node.get_meta("rc1_collision_kind", "mesh")) == "mesh":
		meta(history, selected_node, "rc1_surface", int(values.surface))
	else:
		meta(history, selected_node, "rc1_queries", int(values.queries))

	history.add_do_method(self, "redraw")
	history.add_undo_method(self, "redraw")
	history.commit_action()
	show_object()


## One property change, both ways.
func property(history: EditorUndoRedoManager, target: Object, key: String, value) -> void:
	history.add_do_property(target, key, value)
	history.add_undo_property(target, key, target.get(key))


## One metadata change, both ways.
func meta(history: EditorUndoRedoManager, node: Node, key: String, value) -> void:
	history.add_do_method(node, "set_meta", key, value)

	# A value that was not there goes again on undo.
	if node.has_meta(key):
		history.add_undo_method(node, "set_meta", key, node.get_meta(key))
	else:
		history.add_undo_method(node, "remove_meta", key)


## Replaces the open class scene's collision with one made from its model (MODE: mesh, box,
## sphere or cylinder; KIND: the type byte of triangles).
func make_class_collision(mode: String, kind: int) -> void:
	var layer: Node3D = ClassCollision.class_layer(scene_root)
	var triangles: PackedVector3Array = ClassCollision.model_triangles(scene_root)

	# A class without a model has nothing to make collision from.
	if triangles.is_empty():
		dock.show_report("This class has no model to make collision from.")
		return

	var made: Array[Node3D] = []

	# The mesh mode is a glTF file beside the export's, the others nodes of their own.
	if mode == "mesh":
		var node: Node3D = await mesh_part(ClassCollision.cluster(triangles), kind)

		# The import did not finish: the report already says so.
		if node == null:
			return

		made.append(node)
	else:
		made = ClassCollision.make(triangles, mode, kind)

	var warning: String = ("" if ClassCollision.fits(triangles) else
		" The model reaches past 32 model units from the class's origin: the packer will refuse it.")
	replace_parts(layer, made)
	dock.show_report("Class collision made from the model (%s). Every moby of this class in this level gets it.%s"
		% [mode, warning])


## The mesh mode's triangle part: a glTF file in mobys/collision/, imported, as an instance.
func mesh_part(triangles: PackedVector3Array, kind: int) -> Node3D:
	var id: int = ClassCollision.class_id(scene_root)
	var folder: String = scene_root.scene_file_path.get_base_dir() + "/collision"
	var path: String = "%s/moby_%d_made.gltf" % [folder, id]
	var n: int = 2

	while FileAccess.file_exists(path):
		path = "%s/moby_%d_made_%d.gltf" % [folder, id, n]
		n += 1

	DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(folder))
	var file := FileAccess.open(path, FileAccess.WRITE)
	file.store_string(ClassCollision.mesh_gltf(triangles, kind))
	file.close()
	var filesystem := EditorInterface.get_resource_filesystem()
	filesystem.scan()
	await filesystem.filesystem_changed
	var scene := load(path) as PackedScene

	# The import may not be done yet in a busy editor: the file stays, and a second press uses a new one.
	if scene == null:
		dock.show_report("%s is written but not imported yet; press again in a moment." % path)
		return null

	var node := scene.instantiate() as Node3D
	node.name = "Mesh_%02x" % kind
	node.set_meta("rc1_collision_kind", "mesh")
	node.set_meta("rc1_surface", kind)
	return node


## Replaces a class collision node's children by new ones, undoably.
func replace_parts(layer: Node3D, made: Array[Node3D]) -> void:
	var history := get_undo_redo()
	history.create_action("Make class collision from its model", UndoRedo.MERGE_DISABLE, scene_root)

	for child in layer.get_children():
		history.add_do_method(layer, "remove_child", child)
		history.add_undo_method(layer, "add_child", child)
		history.add_undo_method(child, "set_owner", scene_root)
		history.add_undo_reference(child)

	for node in made:
		node.name = ClassCollision.free_name(layer, String(node.name))
		history.add_do_method(layer, "add_child", node, true)
		history.add_do_method(node, "set_owner", scene_root)
		history.add_do_reference(node)
		history.add_undo_method(layer, "remove_child", node)

	history.add_do_method(self, "redraw")
	history.add_undo_method(self, "redraw")
	history.commit_action()


# --- Handlers ---

## Another scene is edited: read its collision.
func handle_scene_changed(root: Node) -> void:
	open_scene(root)


## The editor's selection changed: a tie's or shrub's baked faces, a moby's class collision, a part.
func handle_selection_changed() -> void:
	var nodes: Array[Node] = EditorInterface.get_selection().get_selected_nodes()
	selected_node = nodes[0] if nodes.size() == 1 else null
	var mesh_name: String = ""

	# A node outside the collision with meshes can become collision; a tie or shrub shows its own.
	if faces != null and selected_node is Node3D and not faces.layer.is_ancestor_of(selected_node):
		var parent: Node = selected_node.get_parent()
		mesh_name = String(selected_node.name) if (selected_node is MeshInstance3D or
			selected_node.find_children("*", "MeshInstance3D", true, false).size() > 0) else ""

		if parent != null and parent.get_parent() == faces.game and parent.name in [&"Ties", &"Shrubs"]:
			select_baked(selected_node as Node3D)

	dock.show_mesh(mesh_name)
	show_object()
	redraw()


## Selects the level's faces inside a tie's or shrub's bounds: its baked collision.
func select_baked(node: Node3D) -> void:
	var box := AABB()
	var first: bool = true

	for found in node.find_children("*", "MeshInstance3D", true, false) + ([node] if node is MeshInstance3D else []):
		var mesh_node := found as MeshInstance3D

		# A mesh instance without a mesh has no bounds.
		if mesh_node.mesh == null:
			continue

		var bounds: AABB = LevelFaces.relative(mesh_node, faces.game) * mesh_node.mesh.get_aabb()
		box = bounds if first else box.merge(bounds)
		first = false

	# Nothing drawn, nothing baked.
	if first:
		return

	faces.select(faces.faces_within(box.grow(BOUNDS_MARGIN)), false)
	dock.show_report("%d faces of the level's collision lie inside %s's bounds: selected. A moved tie or shrub "
		% [faces.selection.size(), node.name] + "keeps its collision where it was: move these faces by the same offset.")
	refresh_selection()


## The undo history moved: someone else's edit (a tile deleted, a part moved) is read soon.
func handle_version_changed() -> void:
	# The dock's own actions already show themselves.
	if not is_committing:
		reload_timer.start()


## Reads the collision again after other edits.
func handle_reload_timeout() -> void:
	# The scene may have closed meanwhile.
	if faces != null and is_instance_valid(faces.layer):
		faces.reload()
		dock.show_level(faces)
		refresh_selection()

	show_object()
	redraw()


# --- Engine Callbacks ---

func _forward_3d_gui_input(camera: Camera3D, event: InputEvent) -> int:
	# Without picking (or a level) the editor handles the view as usual.
	if not is_picking or faces == null:
		return AFTER_GUI_INPUT_PASS

	# The left button picks: a click selects a face, a drag a box.
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
		var button := event as InputEventMouseButton

		if button.pressed:
			is_pressing = true
			drag_from = button.position
			drag_to = button.position
		elif is_dragging:
			is_pressing = false
			is_dragging = false
			pick_in_box(camera, Rect2(drag_from, drag_to - drag_from).abs(), button.shift_pressed)
			update_overlays()
		else:
			is_pressing = false
			pick_at(camera, button.position, button.shift_pressed, button.alt_pressed)

		return AFTER_GUI_INPUT_STOP

	# Moving with the button held drags a box once it is far enough.
	if event is InputEventMouseMotion and is_pressing:
		drag_to = (event as InputEventMouseMotion).position
		is_dragging = is_dragging or drag_from.distance_to(drag_to) > DRAG_THRESHOLD
		update_overlays()
		return AFTER_GUI_INPUT_STOP

	return AFTER_GUI_INPUT_PASS


func _forward_3d_force_draw_over_viewport(viewport_control: Control) -> void:
	# Only a box being dragged is drawn over the view.
	if is_dragging:
		var rect := Rect2(drag_from, drag_to - drag_from).abs()
		viewport_control.draw_rect(rect, Color(DRAG_COLOUR, 0.15), true)
		viewport_control.draw_rect(rect, DRAG_COLOUR, false, 1.0)


func _exit_tree() -> void:
	EditorInterface.get_selection().selection_changed.disconnect(handle_selection_changed)
	get_undo_redo().version_changed.disconnect(handle_version_changed)
	remove_overlay()
	remove_dock(dock)
	dock.queue_free()
