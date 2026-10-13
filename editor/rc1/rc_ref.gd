class_name RcRef
extends RefCounted
## A reference to a value: what C passes as `&x`. Holds either a box of its own
## (`RcRef.new()`, for a local whose address is taken) or a field of an object or
## an element of an array (`RcRef.of(owner, field_or_index)`).

# =============================================================================
# VARIABLES
# =============================================================================

## The value, when the reference is a box of its own.
var _box: Variant = 0
## The object or array the value lives in, or null for a box.
var _owner: Variant = null
## Field name (StringName) or element index (int).
var _key: Variant = null

## The value the reference points at.
var v: Variant:
	get:
		if _owner == null:
			return _box
		if _key is int:
			return _owner[_key]
		return _owner.get(_key)
	set(value):
		if _owner == null:
			_box = value
		elif _key is int:
			_owner[_key] = value
		else:
			_owner.set(_key, value)


# =============================================================================
# METHODS
# =============================================================================

## A reference to OWNER's field KEY (a String) or element KEY (an int).
static func of(owner: Variant, key: Variant) -> RcRef:
	var ref := RcRef.new()
	ref._owner = owner
	ref._key = StringName(key) if key is String else key
	return ref
