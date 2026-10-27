# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Where a game's functions and globals are in each level's program.

A game that loads a program of its own for each level has the same function in
each, at another address and naming other addresses: the globals it uses and
the functions it calls moved with it. The decompilation has the function once,
under the name of one place. To let that one function stand in for every copy,
the build has to know, for each level, where each name it uses is.

The retail code says so itself. Two copies of a function are the same
instructions, apart from the addresses in them: the target of a call, the two
halves of an address (`lui` and the instruction that adds the low half) and an
offset from the global pointer. Reading the same instruction in both copies
gives an address in one program and the address that stands for it in the
other. `Places` reads them from the user's own files and answers three
questions: where is a name in a level, may a function be used there, and which
places does a function have.

A frame is which program an address is in: a level's number, or None for the
program the disc boots.
"""

import bisect
import re
import struct

NAME = re.compile(r"^(?:func|D)_(?:L(\d\d)_)?([0-9A-Fa-f]{8})")

# The opcodes that form an address from a register and 16 bits: the loads and stores, and addiu.
ADDIU = 9
MEMORY = {26, 27, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 49, 53, 54, 55, 57, 61, 62, 63}
# Of those, the ones that write the register in the rt field (a load; a store leaves it).
LOADS = {26, 27, 32, 33, 34, 35, 36, 37, 38, 39, 55}
# The registers a call may change: the result, argument and temporary registers, and the return address.
CALLER_SAVED = tuple(range(1, 16)) + (24, 25, 31)
GLOBAL_POINTER = 28

# How far above a name's address another address may be and still be taken for the same object.
OBJECT_REACH = 0x10000
# How far above it the nearest known address may be to place a name that no instruction names exactly.
NEAR_REACH = 0x400


def frame_and_address(name):
    """Returns (frame, address) of a symbol named after its address, or None."""
    match = NAME.match(name)
    if not match:
        return None
    return (int(match.group(1)) if match.group(1) else None), int(match.group(2), 16)


def signed(low):
    """Returns 16 bits as the signed number an instruction adds."""
    return low - 0x10000 if low & 0x8000 else low


def forms(words, gp):
    """Returns the addresses a function's instructions hold, as (kind, index, index of the lui, address).

    Kind "j" is a call's target, "a" an address in two halves (the index is the instruction
    with the low half), "g" an offset from the global pointer. The scan is in address order and
    remembers which register holds an upper half; a call forgets the registers it may change.
    """
    found = []
    upper = {}
    forget_after = -1
    for index, word in enumerate(words):
        opcode = word >> 26
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        low = word & 0xFFFF
        if opcode == 3:
            found.append(("j", index, -1, (word & 0x03FFFFFF) << 2))
            forget_after = index + 1
        elif opcode == 15:
            upper[rt] = (index, low)
        elif opcode == ADDIU or opcode in MEMORY:
            if rs == GLOBAL_POINTER and gp is not None:
                found.append(("g", index, -1, (gp + signed(low)) & 0xFFFFFFFF))
            elif rs in upper:
                at, high = upper[rs]
                found.append(("a", index, at, ((high << 16) + signed(low)) & 0xFFFFFFFF))
            if opcode == ADDIU or opcode in LOADS:
                upper.pop(rt, None)
        elif opcode == 0 or opcode == 28:
            # Register arithmetic (and the 128-bit set): the result is in the rd field.
            upper.pop((word >> 11) & 31, None)
            if opcode == 0 and (word & 0x3F) == 9:
                forget_after = index + 1
        elif 8 <= opcode <= 14 or 16 <= opcode <= 18 or opcode in (24, 25):
            upper.pop(rt, None)
        if index == forget_after:
            for register in CALLER_SAVED:
                upper.pop(register, None)
    return found


def in_other_copy(found, words, other, gp):
    """Returns the addresses another copy of a function holds where `found` has them, or None.

    None when the two copies are not the same instructions apart from those addresses.
    """
    if len(words) != len(other):
        return None
    masks = {}
    values = []
    for kind, index, at, _ in found:
        if kind == "j":
            masks[index] = 0xFC000000
            values.append((other[index] & 0x03FFFFFF) << 2)
        elif kind == "g":
            masks[index] = 0xFFFF0000
            values.append((gp + signed(other[index] & 0xFFFF)) & 0xFFFFFFFF)
        else:
            masks[index] = 0xFFFF0000
            masks[at] = 0xFFFF0000
            values.append((((other[at] & 0xFFFF) << 16) + signed(other[index] & 0xFFFF)) & 0xFFFFFFFF)
    for index, (ours, theirs) in enumerate(zip(words, other)):
        if ours != theirs and (ours ^ theirs) & masks.get(index, 0xFFFFFFFF):
            return None
    return values


class Places:
    """Where names are in each level's program, read from the retail programs' own code."""

    def __init__(self, regions, catalogue, sizes, gp, resident_end):
        """`regions` maps a frame to [(address, bytes)]; `catalogue` a function's name to (size, {level: [addresses]})."""
        self.regions = regions
        self.catalogue = catalogue
        self.sizes = sizes
        self.gp = gp
        self.resident_end = resident_end
        self.levels = sorted(frame for frame in regions if frame is not None)
        self.references = {}
        self.pair_cache = {}
        self.maps = {}
        self.map_keys = {}
        self.by_frame = {}
        for name in catalogue:
            where = frame_and_address(name)
            if where:
                self.by_frame.setdefault(where[0], []).append(name)

    def size_of(self, name):
        """Returns a function's size in bytes, from the catalogue or the progress report, or 0."""
        return self.catalogue[name][0] if name in self.catalogue else self.sizes.get(name, 0)

    def words(self, frame, address, size):
        """Returns the instructions at an address of a frame's program, or None when they are not on disk."""
        for base, data in self.regions.get(frame, []):
            if size and base <= address and address + size <= base + len(data):
                return struct.unpack_from(f"<{size // 4}I", data, address - base)
        return None

    def bytes_at(self, frame, address, size):
        """Returns the bytes at an address of a frame's program, or None."""
        for base, data in self.regions.get(frame, []):
            if size and base <= address and address + size <= base + len(data):
                return data[address - base:address - base + size]
        return None

    def places(self, name, frame):
        """Returns every address a function has in a frame's program: its own, or the catalogue's."""
        where = frame_and_address(name)
        if where and where[0] == frame:
            return [where[1]]
        if frame is None or name not in self.catalogue:
            return []
        return self.catalogue[name][1].get(frame, [])

    def single(self, name, frame):
        """Returns a function's address in a frame's program when it has exactly one there, or None."""
        found = self.places(name, frame)
        return found[0] if len(found) == 1 else None

    def reference(self, name):
        """Returns (frame, words, forms) of the copy a function is named after, or None."""
        if name not in self.references:
            where = frame_and_address(name)
            words = self.words(where[0], where[1], self.size_of(name)) if where else None
            self.references[name] = (where[0], words, forms(words, self.gp)) if words else None
        return self.references[name]

    def pairs(self, name, frame, address=None):
        """Returns what a function's copy in another frame says, or None when it is no like copy.

        A pair of lists: (target here, target there) for each call, and (address here, address
        there) for each address its instructions hold. The copy is the one at `address`, or the
        function's one place in the frame when no address is given (None if it has several).
        """
        if address is None:
            address = self.single(name, frame)
        key = (name, frame, address)
        if key not in self.pair_cache:
            result = None
            reference = self.reference(name)
            if reference and address is not None:
                _, words, found = reference
                other = self.words(frame, address, self.size_of(name))
                values = in_other_copy(found, words, other, self.gp) if other else None
                if values is not None:
                    calls = [(form[3], value) for form, value in zip(found, values) if form[0] == "j"]
                    data = [(form[3], value) for form, value in zip(found, values) if form[0] != "j"]
                    result = (calls, data)
            self.pair_cache[key] = result
        return self.pair_cache[key]

    def address_map(self, source, frame):
        """Returns {address in `source`: address in `frame`} from every function the two programs share.

        An address that two functions place differently maps to None: nothing is known of it.
        """
        key = (source, frame)
        if key not in self.maps:
            found = {}
            for name in self.by_frame.get(source, []):
                pairs = self.pairs(name, frame)
                for here, there in (pairs[1] if pairs else []):
                    if found.setdefault(here, there) != there:
                        found[here] = None
            self.maps[key] = found
            self.map_keys[key] = sorted(found)
        return self.maps[key]

    def data_place(self, name, frame):
        """Returns (address, exact) of a global in a frame's program, or None when the code does not say.

        Exact when it is the name's own frame, a resident address (below every level's own
        memory, the same in all programs), or an address some shared function holds. Otherwise
        the nearest address above it that a shared function holds is taken to be in the same
        object, and the name is placed by the same distance below its counterpart.
        """
        where = frame_and_address(name)
        if not where:
            return None
        source, address = where
        if source == frame or (source is None and address < self.resident_end):
            return address, True
        known = self.address_map(source, frame)
        if known.get(address) is not None:
            return known[address], True
        keys = self.map_keys[(source, frame)]
        at = bisect.bisect_right(keys, address)
        if at < len(keys) and keys[at] - address < NEAR_REACH and known[keys[at]] is not None:
            return known[keys[at]] - (keys[at] - address), False
        return None

    def chosen(self, name, module):
        """Returns {level: address} for a function's name as one module uses it, where the catalogue places it.

        Where a level has the function more than once (several of the original objects each
        carried a copy, or two functions that differ only in the globals they use went under
        one name), the copy is the one the retail code of the module's own functions calls or
        takes the address of; if that does not say, it is the copy nearest to the module's
        functions, as the decompilation's own check takes it. `agrees` then keeps out the
        functions whose code means another copy.
        """
        choice = {}
        for level, candidates in (self.catalogue[name][1] if name in self.catalogue else {}).items():
            if len(candidates) > 1:
                named = [address for address in candidates if address in self.referenced(module, level)]
                if len(named) == 1:
                    candidates = named
                else:
                    near = [address for plain in module.own for address in self.places(plain, level)]
                    middle = sorted(near)[len(near) // 2] if near else candidates[0]
                    candidates = [min(candidates, key=lambda address: abs(address - middle))]
            choice[level] = candidates[0]
        return choice

    def referenced(self, module, level):
        """Returns every address the retail code of a module's functions holds in a level's program."""
        if level not in module.referenced:
            found = set()
            for plain in module.own:
                for address in self.places(plain, level):
                    words = self.words(level, address, self.size_of(plain))
                    found.update(form[3] for form in forms(words or (), self.gp))
            module.referenced[level] = found
        return module.referenced[level]

    def held(self, name, frame, address=None):
        """Returns the addresses a function's copy in a frame's program holds, or an empty set."""
        if address is None:
            address = self.single(name, frame)
        words = self.words(frame, address, self.size_of(name)) if address is not None else None
        return {form[3] for form in forms(words or (), self.gp)}

    def calls(self, name, frame):
        """Returns the targets of the calls in a function's one copy in a frame's program."""
        address = self.single(name, frame)
        words = self.words(frame, address, self.size_of(name)) if address is not None else None
        return {form[3] for form in forms(words or (), self.gp) if form[0] == "j"}

    def prepare(self, module):
        """Reads what the copies of a module's own functions say about where addresses went.

        For each pair of frames, {address in the one: address in the other}, from every function
        of the module that has a like copy in the other; an address that two of them place
        differently maps to None.
        """
        module.evidence = {}
        for plain in module.own:
            source = frame_and_address(plain)[0]
            for frame in self.levels:
                pairs = self.pairs(plain, frame) if frame != source else None
                for here, there in (pairs[0] + pairs[1] if pairs else []):
                    known = module.evidence.setdefault((source, frame), {})
                    if known.setdefault(here, there) != there:
                        known[here] = None

    def function_place(self, name, module, frame):
        """Returns a function's address in a frame's program as a module's code means it, or None."""
        where = frame_and_address(name)
        if not where:
            return None
        if where[0] == frame:
            return where[1]
        # Resident code, below every level's own memory, is the same in all programs.
        if where[0] is None and where[1] < self.resident_end:
            return where[1]
        if frame is None:
            return None
        if name not in module.choices:
            module.choices[name] = self.chosen(name, module)
        if frame in module.choices[name]:
            return module.choices[name][frame]
        # The catalogue does not have it there: the module's own functions may say where it went.
        for (source, target), known in module.evidence.items():
            if target == frame:
                here = where[1] if where[0] == source else module.choices[name].get(source)
                if here is not None and known.get(here) is not None:
                    return known[here]
        return None

    def usable(self, module, name, frame, address=None):
        """Returns whether a module's function may stand in for its copy at an address of a frame's program.

        Without an address, the copy is the function's one place in the frame. A name with
        several places there (functions that differ only in the globals they use went under one
        name) is asked about place by place: the function stands in where the copy uses what
        its own code uses.

        It may when the copy is the same instructions, every function it calls is where the
        module's names lead in that frame, and every address it holds is where the module's
        names for globals lead. A call to another function of the same module needs that one
        usable too: host code calls it directly.
        """
        if address is None:
            address = self.single(name, frame)
        if address is None:
            return False
        key = (name, frame, address)
        if key in module.usable:
            return module.usable[key]
        source, own = frame_and_address(name)
        same = source == frame and own == address
        module.usable[key] = True  # a function that calls itself does not stand in its own way
        module.usable[key] = self.agrees(module, name, frame, address) and (same or self.fits(module, name, source, frame, address))
        return module.usable[key]

    def agrees(self, module, name, frame, address=None):
        """Returns whether a function's copy means the same copies of duplicated functions as its module.

        A module has one address for a name in a level. A function whose retail code holds
        the address of another copy under that name would call the wrong one from host code.
        """
        if frame is None:
            return True
        held = self.held(name, frame, address)
        for callee in module.function_names:
            candidates = self.catalogue[callee][1].get(frame, []) if callee in self.catalogue else []
            if len(candidates) > 1:
                mine = self.function_place(callee, module, frame)
                if any(address in held and not self.alike(callee, frame, address, mine) for address in candidates):
                    return False
        return True

    def alike(self, name, frame, one, other):
        """Returns whether two places of a function in one program hold the very same bytes.

        Two such copies call the same functions and use the same globals: either does for the
        other. (Copies that differ are two functions under one name.)
        """
        if one == other:
            return True
        if one is None or other is None:
            return False
        size = self.size_of(name)
        code = self.bytes_at(frame, one, size)
        return code is not None and code == self.bytes_at(frame, other, size)

    def fits(self, module, name, source, frame, address):
        """Does the work of `usable` for one function and one copy of it."""
        pairs = self.pairs(name, frame, address)
        if pairs is None:
            return False
        calls, data = pairs
        own = {self.single(plain, source): plain for plain in module.own}
        for here, there in calls:
            if here in own:
                callee = own[here]
                if there not in self.places(callee, frame) or not self.usable(module, callee, frame, there):
                    return False
                continue
            if not self.leads(module, source, frame, here, there):
                return False
        if (source, "data") not in module.sorted:
            placed = [(self.data_place(plain, source), plain) for plain in module.data_names]
            module.sorted[(source, "data")] = sorted((place[0], plain) for place, plain in placed if place)
        globals_here = module.sorted[(source, "data")]
        starts = [address for address, _ in globals_here]
        for here, there in data:
            # The address of a function (a callback handed on) follows the module's names like a call.
            if not self.leads(module, source, frame, here, there):
                return False
            at = bisect.bisect_right(starts, here) - 1
            if at < 0 or here - starts[at] >= OBJECT_REACH:
                continue
            start, plain = globals_here[at]
            place = self.data_place(plain, frame)
            if place is None:
                return False
            if place[0] + (here - start) == there:
                continue
            # Another object above an exactly placed global is no concern of this name.
            if here == start or not place[1]:
                return False
        return True

    def leads(self, module, source, frame, here, there):
        """Returns whether the module's names for the function at `here` lead to `there` in another frame.

        True as well when no name of the module stands for `here`: it is not a function the
        module's code names.
        """
        if (source, "functions") not in module.sorted:
            found = {}
            for callee in module.function_names:
                found.setdefault(self.function_place(callee, module, source), []).append(callee)
            module.sorted[(source, "functions")] = found
        return all(self.alike(callee, frame, self.function_place(callee, module, frame), there)
                   for callee in module.sorted[(source, "functions")].get(here, []))

    def column(self, name, module):
        """Returns a name's address in the boot program and then in each level, as a module's code means it.

        A function with no place in a level keeps the address in its name there, and a global
        that cannot be placed has 0: no function that uses it is bound in that level.
        """
        where = frame_and_address(name)
        own = where[1] if where else 0
        if name.startswith("D_"):
            found = [self.data_place(name, frame) for frame in self.levels]
            return [own] + [place[0] if place else 0 for place in found]
        return [own] + [self.function_place(name, module, frame) or own for frame in self.levels]
