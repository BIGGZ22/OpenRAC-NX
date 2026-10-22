@tool
extends RefCounted
## The Collision dock's help page (its "How to edit collision" button). editor/LEVEL_MODDING.md,
## "Collision", says the same for the web.

# =============================================================================
# CONSTANTS
# =============================================================================

## The page, in BBCode.
const TEXT: String = """[b]How to edit collision[/b]

[b]Where it is.[/b] Collision is what Ratchet stands on and bumps into, and what shots and bolts hit. A level has two kinds: the [b]level's collision[/b] (one mesh for the whole level, terrain, ties and shrubs baked in, under Game/Collision; walls only the player meets under Game/HeroWalls) and each moby class's [b]object collision[/b] (crates, doors, enemies: spheres, cylinders, capsules and triangles in the class scene, mobys/moby_<class>.tscn).

[b]See it.[/b] Turn on [i]Show collision[/i]: every face is drawn over the level in its surface's colour, and every moby shows its class's collision. The legend lists the surfaces this level has. A face's type byte has three parts: the surface id (bits 0-4, the colour; 31 is nothing special, 0 water), the footstep sound class (bits 5-6) and a bit that keeps it out of some of the game's queries (bit 7).

[b]Select.[/b] Turn on [i]Pick faces[/i], then click a face in the 3D view. Shift-click adds a face or takes it out; drag a box to select every face whose middle is inside it; Alt-click takes the faces that touch it with the same type byte (Shift-Alt-click adds them; [i]Connected, same surface[/i] adds those of the whole selection). Selecting a tie or a shrub (Pick faces off) selects the collision baked inside its bounds; selecting a moby shows its class's collision.

[b]Repaint.[/b] Choose a surface id, sound class and the exclusion bit, then [i]Repaint the selected faces[/i].

[b]Delete.[/b] [i]Delete the selected faces[/i]: the game no longer meets them. Hero wall faces can be deleted, not repainted or moved.

[b]Move.[/b] Type an offset in game units (Z is up) and press [i]Move the selected faces by this[/i]. A tie you moved keeps its collision where it was: select it, then move its faces by the same offset.

[b]Add.[/b] Click where it goes, set a size, then [i]Add a floor quad[/i] or [i]Add a box[/i]. To give your own model collision, select its mesh (any MeshInstance3D, or a node with meshes under it) and press [i]Make collision from the selected mesh[/i]: its triangles are added, simplified to fit the game's cells. Faces are solid from the side their corners turn counter-clockwise from; a box faces out.

[b]Object collision.[/b] Select a moby and press [i]Open its class scene to edit[/i]. In the class scene, select a part under Collision to change its radius, height, position, queries or type byte, or move it with the gizmo; [i]Make class collision from its model[/i] replaces the class's collision with one made from the class's model (mesh: its triangles, merged to at most 256 corners; box, sphere, cylinder: one shape around it). [b]A class edit changes every moby of that class in this level[/b]; other levels keep their own copy.

[b]What packs.[/b] Every edit is undoable (Ctrl+Z) and is saved with the scene (Ctrl+S); the level's tiles are not changed. [i]Pack your edited levels[/i] (or extract.py pack) puts the edits into the level's collision; the faces you did not touch keep the disc's bytes, and a level nobody edited packs to nothing. changes.txt says what was packed and why something could not be.

[b]Limits.[/b] The game keeps the collision in 4-unit cells: at most 255 corners in a cell ([i]Check the limits[/i] estimates it), positions on a grid of 1/16 unit across and 1/64 up (a face the grid flattens is dropped), everything inside 0-1024 units on each axis. A large face is cut in pieces at packing. A class's collision keeps its corners within 32 model units of its origin and its triangles to 256 corners. Over a limit, the packer leaves that level's (or that class's) collision as the disc has it and says why.

[b]Advanced.[/b] The tiles are glTF files in the level's collision/ folder (a material rc1_surface_XX per type byte): edit one in Blender and export it to the same file, or add meshes under Game/Collision with an rc1_surface metadata entry. The dock's edits apply on top of the tiles."""
