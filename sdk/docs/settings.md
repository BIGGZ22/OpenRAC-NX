# Settings

Settings are options a player changes in the launcher's Mods tab, under your
mod's row. You declare them in the manifest; the launcher draws the controls,
keeps the values, and the game hands them to your mod at start. A change counts
from the game's next start.

## Declaring them

```json
"settings": [
  { "key": "mode", "name": "Mode", "type": "choice", "default": "lock",
    "choices": [ { "value": "lock", "name": "Lock with weapons" },
                 { "value": "hold", "name": "Hold L2 or R2" } ],
    "description": "How the strafe starts." },
  { "key": "trace", "name": "Trace", "type": "bool", "default": false },
  { "key": "every", "name": "Log every", "type": "int", "default": 10, "min": 1, "max": 600 },
  { "key": "scale", "name": "Speed", "type": "float", "default": 1.0, "min": 0.5, "max": 3.0, "step": 0.1 },
  { "key": "greeting", "name": "Greeting", "type": "string", "default": "hi", "max_length": 40 }
]
```

| Field | |
|---|---|
| `key` | lower case, digits and `_`: what your code asks for |
| `name` | the label players see |
| `type` | `bool`, `int`, `float`, `choice` or `string` |
| `default` | the value until the player changes it |
| `description` | one line under the label (optional) |
| `min`, `max` | `int` and `float` |
| `step` | `float` |
| `choices` | `choice`: two or more `{"value", "name"}` |
| `max_length` | `string` |

The launcher shows a switch for `bool`, a segmented control or a list for
`choice`, and a field for the others.

## Reading them

| | |
|---|---|
| Luau | `openrac.setting("mode")`: a boolean, number or string; `nil` for a key the manifest does not declare |
| C++ | `mod.setting<std::string>("mode")`, `setting<bool>`, `setting<int>`, `setting<double>`, ... |
| C | `api->setting_string(mod, "mode")`, `setting_bool`, `setting_int`, `setting_float` |

A value that is missing, of the wrong type or out of range is the default:
your code always gets a valid value.

Where the values are kept: [where mods live](where-mods-live.md)
(`plugins.json`).
