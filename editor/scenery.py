"""Ties and shrubs edited in Godot, packed for the native port.

The port draws a level's ties and shrubs from the files the extractor wrote (port.py's
placements.json and one model per class), not from the game's memory, so an edit to them is
packed as those files are: `<out>/level_NN/placements.json` with the lists that changed, which
the port's `--mods` reads in place of the level's own (port/viewer/level.cpp).

Packed: ties and shrubs that were moved, turned, scaled (each axis its own), given another class
of the level or other values, added (a copy, or a class of the level dropped under Game/Ties or
Game/Shrubs) or deleted. Each is lit again where it stands, as the game lights it at level load
(lighting.py). Not packed: what the game itself knows of them. The gameplay file's tie and shrub
tables and the level's collision stay the disc's, so a tie that was moved is drawn in its new
place and is still walked on in its old one, and a new one is walked through.
"""

from formats import FormatError
from lighting import light_bank, light_shrub_instance, light_tie_instance
from pack import same, same_value, scene_nodes, scene_transform
from port import FORMAT, placement
from shrubs import shrub_class_normals, shrub_classes, shrub_instances
from ties import tie_classes, tie_instances, tie_slot_normals

# A family's list in placements.json, its nodes' parent in the scene, and what a node may set.
FAMILIES = (("ties", "Ties", ("draw_distance", "occlusion_index", "directional_lights", "uid")),
            ("shrubs", "Shrubs", ("draw_distance", "colour", "directional_lights")))


def matrix_of(t: tuple) -> list[float]:
    """A scene transform (basis rows, then the origin) as a column-major 4 x 4."""
    columns = [[t[row * 3 + column] for row in range(3)] + [0.0] for column in range(3)]
    return [v for column in columns for v in column] + [t[9], t[10], t[11], 1.0]


def pack_family(nodes: list[dict], originals: list[dict], classes, values: tuple) -> tuple[list[dict] | None, list[str], list[str]]:
    """The family's instances as the scene has them (None when it has them as the disc does),
    what changed, and what could not be packed."""
    changes, problems = [], []

    def index_of(node: dict) -> int | None:
        index = node["meta"].get("index")
        return index if isinstance(index, int) and 0 <= index < len(originals) else None

    owner: dict[int, dict] = {}
    for node in nodes:
        index = index_of(node)
        if index is None:
            continue
        named = node["name"].endswith(f"_{index:04}")
        if index not in owner or (named and not owner[index]["name"].endswith(f"_{index:04}")):
            owner[index] = node

    out = []
    for node in nodes:
        index = index_of(node)
        mine = index is not None and owner[index] is node
        model = index
        if model is None and node["class_id"] is not None:
            model = next((p["index"] for p in originals if p["class_id"] == node["class_id"]), None)
        if model is None:
            problems.append(f"{node['name']}: the level has nothing of its class to take after; not packed")
            continue
        p = dict(originals[model])
        edits = []
        if not mine or not same(node["transform"], scene_transform(p["matrix"])):
            p["matrix"] = matrix_of(node["transform"])
            edits.append("moved")
        if node["class_id"] is not None and node["class_id"] != p["class_id"]:
            if node["class_id"] not in classes:
                problems.append(f"{node['name']}: class {node['class_id']} is not one of the level's; its class is kept")
            else:
                edits.append(f"class {p['class_id']} -> {node['class_id']}")
                p["class_id"] = node["class_id"]
        for key in values:
            if key in node["meta"] and not same_value(node["meta"][key], p[key]):
                p[key] = node["meta"][key]
                edits.append(key)
        if not mine:
            changes.append(f"{node['name']}: added, after {model}")
        elif edits:
            changes.append(f"{node['name']}: {', '.join(edits)}")
        out.append(p)
    for index in sorted(set(range(len(originals))) - set(owner)):
        changes.append(f"{index:04}: deleted")
    return (out if changes else None), changes, problems


class _Any:
    def __contains__(self, item) -> bool:
        return True


def edited(gameplay: bytes, scene: str) -> bool:
    """Whether the scene's ties or shrubs differ from the gameplay file's: the quick look that
    spares reading the whole level when they do not."""
    sources = {"ties": tie_instances(gameplay, _Any()), "shrubs": shrub_instances(gameplay, _Any())}
    for key, parent, values in FAMILIES:
        try:
            _, changes, problems = pack_family(scene_nodes(scene, parent, key[:-1]), sources[key], _Any(), values)
        except (FormatError, ValueError):
            return True
        if changes or problems:
            return True
    return False


def pack_scenery(level, scene: str) -> tuple[dict | None, list[str], list[str]]:
    """What goes into the level's placements.json for the port (None: nothing changed), what
    changed, and what could not be packed."""
    bank = light_bank(level.gameplay)
    ties, shrubs = tie_classes(level), shrub_classes(level)
    tie_normals, shrub_normals = tie_slot_normals(level), shrub_class_normals(level)
    sources = {"ties": (tie_instances(level.gameplay, ties), ties),
               "shrubs": (shrub_instances(level.gameplay, shrubs), shrubs)}
    out, changes, problems = {"format": FORMAT}, [], []
    for key, parent, values in FAMILIES:
        originals, classes = sources[key]
        kind = key[:-1]
        try:
            edited, what, wrong = pack_family(scene_nodes(scene, parent, kind), originals, classes, values)
        except (FormatError, ValueError) as e:
            problems.append(f"{key}: {e}; not packed")
            continue
        changes += [f"{kind} {line}" for line in what]
        problems += [f"{kind} {line}" for line in wrong]
        if edited is None:
            continue
        for index, p in enumerate(edited):
            p["index"] = index
            if key == "ties":
                lit = light_tie_instance(tie_normals[p["class_id"]], p["matrix"], p["ambient"],
                                         p["directional_lights"], bank)
            else:
                lit = light_shrub_instance(shrub_normals[p["class_id"]], p["matrix"], p["colour"],
                                           p["directional_lights"], bank)
            p["colours"] = [r | g << 8 | b << 16 | a << 24 for r, g, b, a in lit]
        out[key] = [placement(p) for p in edited]
    return (out if len(out) > 1 else None), changes, problems
