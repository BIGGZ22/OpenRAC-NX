class_name RcStruct
extends RefCounted
## A C struct held by value: named fields with defaults, readable and writable by name.
## Fields the translated code reaches that were not declared read as 0 and are created
## on first write, so a half-known struct still runs.

# =============================================================================
# VARIABLES
# =============================================================================

## The C struct's name, for messages.
var type_name: String = ""
## Field name -> value.
var fields: Dictionary = {}


# =============================================================================
# METHODS
# =============================================================================

## A struct of TYPE_NAME with FIELDS as its starting values.
static func of(type_name: String, fields: Dictionary) -> RcStruct:
	var s := RcStruct.new()
	s.type_name = type_name
	s.fields = fields
	return s


func _get(property: StringName) -> Variant:
	return fields.get(property, 0)


func _set(property: StringName, value: Variant) -> bool:
	fields[property] = value
	return true
