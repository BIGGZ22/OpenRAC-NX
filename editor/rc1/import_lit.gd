@tool
extends EditorScenePostImport
## Runs when Godot imports one of the level's glTF files (project.godot's scene import default).
## A surface that carries vertex colours shows them: the terrain's are the light the game gives
## it at level load, a moby's its own colours. Godot's glTF import keeps the colours but leaves
## its materials unaware of them. Display only: nothing here runs in the game.

# =============================================================================
# METHODS
# =============================================================================

## Lets every coloured surface under the imported scene use its vertex colours.
func _post_import(scene: Node) -> Object:
	for node in scene.find_children("*", "MeshInstance3D", true, false):
		show_vertex_colours((node as MeshInstance3D).mesh)

	return scene


## Turns vertex colours on in the material of each surface of mesh that has them.
func show_vertex_colours(mesh: Mesh) -> void:
	# A mesh instance without a mesh has nothing to show.
	if mesh == null:
		return

	for i in mesh.get_surface_count():
		var material := mesh.surface_get_material(i) as BaseMaterial3D
		var colours = mesh.surface_get_arrays(i)[Mesh.ARRAY_COLOR]

		# Only a standard material on a surface with colours; the collision's unlit
		# materials have none, and other materials are left as they are.
		if material != null and colours != null and colours.size() > 0:
			material.vertex_color_use_as_albedo = true
