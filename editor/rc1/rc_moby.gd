class_name RcMoby
extends Node3D
## Base of a moby class script: holds the moby's header record and, when the preview is
## switched on, calls the class's update function once per game frame (60 Hz).
##
## The generated class scripts (rc1/behaviour/moby_<class>.gd) extend this and supply
## rc_update(moby). The preview runs the translated code against the RcEngine stub: it
## shows what the function does with the values, not the game.

# =============================================================================
# VARIABLES
# =============================================================================

## Run rc_update every physics frame. Off by default: the exported level is a viewer.
@export var rc_preview: bool = false
## The header record the update function reads and writes.
var data: RcMobyData = RcMobyData.new()
var _accumulator: float = 0.0


# =============================================================================
# METHODS
# =============================================================================

func _ready() -> void:
	data.pvars = self
	data.oclass = int(get_meta("rc1_class", 0))
	var p := global_position
	data.pos = PackedFloat32Array([p.x, p.y, p.z, 1.0])
	set_physics_process(rc_preview)


## One game frame of the class's behaviour. Generated scripts override it.
func rc_update(moby: RcMobyData) -> void:
	pass


func _physics_process(delta: float) -> void:
	_accumulator += delta
	while _accumulator >= 1.0 / 60.0:
		_accumulator -= 1.0 / 60.0
		RcEngine.frame += 1
		rc_update(data)
