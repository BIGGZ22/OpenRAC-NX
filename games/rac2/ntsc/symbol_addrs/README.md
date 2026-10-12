# symbol_addrs

Symbol address definitions for the decompilation project.

## Structure

```
symbol_addrs/
├── usa/
│   ├── symbol_addrs.txt      # Main symbol definitions
│   └── alias_provides.txt    # Old name → new name aliases
└── eu/
    ├── symbol_addrs.txt
    └── alias_provides.txt
```

## symbol_addrs.txt

One definition per line:

```
<symbol_name> = 0x<address>;
```

Example:
```
g_bProgressiveScan = 0x002B58A0;
func_001163A0 = 0x001163A0;
```

## alias_provides.txt

Maps old address-named symbols to their new names:

```
<old_name> <new_name>
```

Example:
```
func_001163A0 FUN_001163A0
```

This allows the linker to resolve references to old names (used in hand-written
.s files) by providing them as aliases to the new names.

## Usage

The build system automatically reads symbol_addrs.txt and emits PROVIDE lines
for the linker. The alias_provides.txt is used by tools/ee/src_alias_provides.sh
to provide backward compatibility.

## Adding Symbols

1. Add new symbols to symbol_addrs/REGION/symbol_addrs.txt
2. Run the build to verify byte-exact match
3. Commit the changes

## Notes

- USA region uses SCUS_972.68
- EU region uses SCES_516.07
- Symbols must match the actual binary layout
- Use `mips-linux-gnu-nm` to inspect the current build
