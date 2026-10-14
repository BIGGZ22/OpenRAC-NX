class_name RcEngine
extends RefCounted
## The engine services a translated moby update function calls, as static functions.
##
## Names follow behaviour_api.tsv, which says what each is and which C function it stands
## for. The maths (vectors, angles, timers, random numbers) is real. Services that need the
## game around them (collision, sound, hit messages, spawning) are stubs here: they record
## that they were called and return a neutral value, so a script can be read and stepped in
## the editor's preview but does not play the game. The native port does not use this file;
## it runs the C.

# =============================================================================
# VARIABLES
# =============================================================================

## Region speed factor: 1.0 NTSC, 1.2 PAL (D_0015EE60).
static var speed: float = 1.2
## Speed squared (D_0015EE64).
static var speed2: float = 1.44
## 1 / speed (D_0015EE68).
static var inv_speed: float = 1.0 / 1.2
## Seconds per frame (D_0015EE6C): 1/60 NTSC, 1/50 PAL.
static var dt: float = 0.02
## dt squared scale for accelerations (D_0015EE70).
static var dt2: float = 0.0004
## dt cubed scale (D_0015EE74).
static var dt3: float = 0.000008
## The frame counter (D_0015F6B0).
static var frame: int = 0
## The level number (D_0015EE84).
static var level: int = 0
## Any other global a translated script reads, by its C name; unknown ones read as 0.
static var globals: RcStruct = RcStruct.new()
## Engine calls with no implementation here: name -> times called.
static var unmapped_calls: Dictionary = {}
## Stubbed services called: name -> times called.
static var stub_calls: Dictionary = {}
## Placeholder mobys handed out by moby_at.
static var _table: Dictionary = {}
## Where random numbers come from.
static var rng: RandomNumberGenerator = RandomNumberGenerator.new()


# =============================================================================
# METHODS
# =============================================================================

# --- Helpers for generated code ---

## A float array of N zeros.
static func zeros_f(n: int) -> PackedFloat32Array:
	var a := PackedFloat32Array()
	a.resize(n)
	return a


## An int array of N zeros.
static func zeros_i(n: int) -> PackedInt32Array:
	var a := PackedInt32Array()
	a.resize(n)
	return a


## An array of N values made by calling MAKE.
static func array_of(n: int, make: Callable) -> Array:
	var a := []
	for i in n:
		a.append(make.call())
	return a


## A call to an engine function this API does not implement. Records it and returns 0.
static func unmapped(name: StringName, args: Array = []) -> Variant:
	unmapped_calls[name] = unmapped_calls.get(name, 0) + 1
	return 0


static func _stub(name: String, result: Variant = 0) -> Variant:
	stub_calls[name] = stub_calls.get(name, 0) + 1
	return result


## Copy a vector or struct: qcopy(dst, src).
static func qcopy(dst: Variant, src: Variant) -> void:
	if dst is PackedFloat32Array or dst is Array or dst is PackedInt32Array:
		for i in mini(dst.size(), src.size() if src != null and not src is int else 0):
			dst[i] = src[i]
	elif dst is RcStruct and src is RcStruct:
		dst.fields = src.fields.duplicate(true)


## Zero a 64-bit value or a small struct.
static func clear_u64(v: Variant) -> void:
	if v is RcRef:
		v.v = 0
	elif v is RcStruct:
		for k in v.fields:
			v.fields[k] = 0


# --- Time ---

## Frames at 60 Hz scaled to this region's frame rate.
static func scale_ticks(n: int) -> int:
	return int(float(n) * speed + 0.5)


## Count a timer down by one: 1 if it was already 0, 2 when this step reached 0, else 0.
static func dec_timer(ref: RcRef) -> int:
	var t: int = ref.v
	if t == 0:
		return 1
	t = maxi(t, 1) - 1
	ref.v = t
	return 2 if t < 1 else 0


## Same contract as dec_timer for a byte timer (func_L00_001FEF78).
static func step_timer(ref: RcRef) -> int:
	return dec_timer(ref)


## x times speed squared.
static func frame_scale(x: float) -> float:
	return speed2 * x


# --- Vectors (xyz; w is left alone) ---

## Component I of V; a value that is not a vector (a global the stub does not hold) reads as 0.
static func _at(v: Variant, i: int) -> float:
	if v is PackedFloat32Array or v is Array or v is PackedInt32Array:
		return float(v[i]) if i < v.size() else 0.0
	return 0.0


static func _put(out: Variant, i: int, x: float) -> void:
	if out is PackedFloat32Array or out is Array:
		if i < out.size():
			out[i] = x


static func vec_add(out: Variant, a: Variant, b: Variant) -> void:
	var x: float = _at(a, 0) + _at(b, 0)
	var y: float = _at(a, 1) + _at(b, 1)
	var z: float = _at(a, 2) + _at(b, 2)
	_put(out, 0, x)
	_put(out, 1, y)
	_put(out, 2, z)


static func vec_sub(out: Variant, a: Variant, b: Variant) -> void:
	var x: float = _at(a, 0) - _at(b, 0)
	var y: float = _at(a, 1) - _at(b, 1)
	var z: float = _at(a, 2) - _at(b, 2)
	_put(out, 0, x)
	_put(out, 1, y)
	_put(out, 2, z)


static func vec_scale(out: Variant, v: Variant, s: float) -> void:
	var x: float = _at(v, 0) * s
	var y: float = _at(v, 1) * s
	var z: float = _at(v, 2) * s
	_put(out, 0, x)
	_put(out, 1, y)
	_put(out, 2, z)


static func vec_dot(a: Variant, b: Variant) -> float:
	return _at(a, 0) * _at(b, 0) + _at(a, 1) * _at(b, 1) + _at(a, 2) * _at(b, 2)


static func vec_length(v: Variant) -> float:
	return sqrt(vec_dot(v, v))


static func vec_dist(a: Variant, b: Variant) -> float:
	var d := PackedFloat32Array([0.0, 0.0, 0.0, 0.0])
	vec_sub(d, a, b)
	return vec_length(d)


static func vec_distance(a: Variant, b: Variant) -> float:
	return vec_dist(a, b)


## out = v scaled to length a; a zero vector stays zero.
static func vec_set_length(out: Variant, v: Variant, a: float) -> void:
	var len2: float = vec_dot(v, v)
	if len2 == 0.0:
		_put(out, 0, 0.0)
		_put(out, 1, 0.0)
		_put(out, 2, 0.0)
		return
	var s: float = a / sqrt(len2)
	var x: float = _at(v, 0) * s
	var y: float = _at(v, 1) * s
	var z: float = _at(v, 2) * s
	_put(out, 0, x)
	_put(out, 1, y)
	_put(out, 2, z)


# --- Angles and maths ---

static func _wrap(a: float) -> float:
	return fposmod(a + PI, TAU) - PI


## a + b as an angle, wrapped to -pi..pi.
static func add_rots(a: float, b: float) -> float:
	return _wrap(a + b)


static func sub_rots(a: float, b: float) -> float:
	return _wrap(a - b)


## Absolute difference of two angles, 0..pi.
static func diff_rots(a: float, b: float) -> float:
	return absf(_wrap(a - b))


static func fast_sin(x: float) -> float:
	return sin(x)


static func fast_cos(x: float) -> float:
	return cos(x)


static func abs_f(x: float) -> float:
	return absf(x)


## Angle of the vector (x, y).
static func atan2(x: float, y: float) -> float:
	return Vector2(x, y).angle()


static func truncate(x: float) -> int:
	return int(x)


static func int_to_float(x: int) -> float:
	return float(x)


## Move REF's value toward TARGET by at most STEP; returns the remaining distance.
static func approach(target: float, step: float, ref: RcRef) -> float:
	var d: float = target - ref.v
	if absf(d) <= step:
		ref.v = target
	else:
		ref.v += step * signf(d)
	return d


## Turn the angle in REF toward TARGET, limited by speed and acceleration (simplified).
static func steer_angle(ref: RcRef, target: float, vel: RcRef, a: float, b: float, c: float) -> float:
	var d := sub_rots(target, ref.v)
	var step := minf(absf(d), absf(c) * 1.0)
	ref.v = add_rots(ref.v, step * signf(d))
	return d


static func random_between(lo: float, hi: float) -> float:
	return rng.randf_range(lo, hi)


static func random_below(n: int) -> int:
	return rng.randi() % maxi(n, 1)


static func rand_range(lo: int, hi: int) -> int:
	return rng.randi_range(lo, hi)


static func random_angle() -> float:
	return rng.randf_range(-PI, PI)


# --- Services (stubs) ---

static func delete_moby(m: Variant) -> void:
	if m != null:
		m.state = 0xFE
	_stub("delete_moby")


static func create_moby(oclass: int) -> Variant:
	var m := RcMobyData.new()
	m.oclass = oclass
	_stub("create_moby")
	return m


## The moby with table index I: in the stub, a dead placeholder (state 0xFE).
static func moby_at(i: int) -> RcMobyData:
	if not _table.has(i):
		var m := RcMobyData.new()
		m.state = 0xFE
		_table[i] = m
	return _table[i]


static func moby_alive(m: Variant) -> int:
	return 1 if m != null and m.state < 0xFD else 0


static func set_animation(m: Variant, seq: int, frame_: int, blend: int) -> void:
	m.prev_seq = seq
	m.seq = seq
	m.frame = frame_
	_stub("set_animation")


static func update_animation_state(m: Variant) -> void:
	_stub("update_animation_state")


static func ground_height(pos: Variant, unused: int, offset: float) -> float:
	_stub("ground_height")
	return pos[2] - offset


static func play_class_sound(n: int, unused: int, m: Variant) -> int:
	return _stub("play_class_sound")


static func sound_alive(m: Variant, slot: Variant) -> int:
	return 0


static func sound_release(m: Variant, slot: Variant) -> void:
	_stub("sound_release")


static func get_hit(m: Variant, mask: int, keep: int) -> Variant:
	return _stub("get_hit", null)


static func add_draw_callback(callback: Variant, m: Variant) -> void:
	_stub("add_draw_callback")


static func coll_sphere(radius: float, pos: Variant, mask: int, flags: int) -> int:
	return _stub("coll_sphere")


static func coll_sphere_mobys(radius: float, pos: Variant, mask: int, owner_: Variant, burst: Variant) -> int:
	return _stub("coll_sphere_mobys")


static func coll_line(a: Variant, b: Variant, mask: int, c: int, d: int) -> int:
	return _stub("coll_line")


static func group_first(out: Variant, group: int, a: int, b: int) -> void:
	_stub("group_first")


static func group_next(out: Variant, current: Variant, a: int, b: int) -> void:
	_stub("group_next")


static func in_clip_volume(pos: Variant, volume: Variant) -> int:
	return _stub("in_clip_volume")


static func joint_position(m: Variant, joint: int, out: Variant) -> void:
	_stub("joint_position")


static func beam_explosion(m: Variant, a: Variant = null, b: Variant = null, c: Variant = null) -> void:
	_stub("beam_explosion")


static func break_fx(m: Variant, a: Variant = null, b: Variant = null, c: Variant = null) -> void:
	_stub("break_fx")


static func step_motion(m: Variant, state: Variant) -> int:
	return _stub("step_motion")


static func attach_manipulator(m: Variant, index: int, data: Variant) -> void:
	_stub("attach_manipulator")


## -1 when the sphere is on screen; the preview treats everything as visible.
static func bsphere_check(pos: Variant, radius: float) -> int:
	return -1
