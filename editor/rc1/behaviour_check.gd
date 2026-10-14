extends SceneTree
## Loads every generated class script under res://levels/*/mobys/behaviour, builds one
## node of each and runs its update function for a few frames against the RcEngine stub.
## A script that does not parse fails at load; a runtime error is printed by the engine.
## Run: godot --headless --path <project> --script res://rc1/behaviour_check.gd

const FRAMES: int = 5
## Values a class needs to get past its set-up, which the exported level would read from
## the moby's pvar record: script file -> field -> value.
const PRESETS: Dictionary = {
	"moby_749.gd": {"f1E8": -1},
}


func _initialize() -> void:
	var failures := 0
	var count := 0
	var levels := DirAccess.get_directories_at("res://levels")
	for level in levels:
		var dir := "res://levels/%s/mobys/behaviour" % level
		for file in DirAccess.get_files_at(dir):
			if not file.ends_with(".gd"):
				continue
			var script: GDScript = load("%s/%s" % [dir, file])
			count += 1
			if script == null or not script.can_instantiate():
				print("FAIL parse: ", dir, "/", file)
				failures += 1
				continue
			var node: RcMoby = script.new()
			node.data.pvars = node
			node.data.state = 0
			node.data.unk31 = 1
			for field in PRESETS.get(file, {}):
				node.set(field, PRESETS[file][field])
			for i in FRAMES:
				node.rc_update(node.data)
			print("ok ", file, " ran ", FRAMES, " frames, state ", node.data.state,
					", stubs called ", RcEngine.stub_calls.size(), ", unmapped ", RcEngine.unmapped_calls.size())
			node.free()
	print("scripts: ", count, " failures: ", failures)
	quit(1 if failures > 0 else 0)
