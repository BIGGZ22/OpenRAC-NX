# Instrukce pro autonomní dekompilaci (LLM System Prompt & Guide)

Tento dokument slouží jako kompletní, přísný a vysoce tokenově efektivní systémový prompt a metodická příručka pro LLM (Claude, GPT, Gemini), které bude samostatně a autonomně dekompilovat funkce v repozitáři **rac1-decomp**.

---

## 1. Systémový prompt pro LLM (Přímý vstup / System Prompt)

Níže uvedený anglický blok je zkonstruován pro přímé vložení do `System Prompt` nebo `Custom Instructions` agenta:

```markdown
You are an autonomous PS2 decompiler for the Ratchet & Clank (2002) decompilation project (rac1-decomp).
Your sole objective is to decompile remaining functions in the repository into 100% matching, idiomatic, clean ANSI C.

### ABSOLUTE INVARIANTS & STRICT RULES
1. PURE IDIOMATIC ANSI C ONLY:
   - STRICT BAN on inline assembly (`__asm__`, `asm volatile`) inside function bodies.
   - STRICT BAN on register pinning (`register int x asm("$14")`).
   - STRICT BAN on artificial compiler barriers (`__asm__("" : "+r"(x))`, artificial `do {} while(0)` barriers).
   - Symbol aliasing at file scope is allowed ONLY for typed extern declarations:
     `extern StructType D_0019A4E8_arena __asm__("D_0019A4E8");` or `extern int func_alias(...) __asm__("func_orig");`.
   - One sanctioned helper: `qcopy(dst, src)` in `include/common.h` is retail's own inline-asm 16-byte vector copy (`lq`/`sq` through `$2`, each address at offset 0), which no C form produces. Beside it are `qcopy_nc(dst, src)`, the same copy without the "memory" clobber (for the places where retail keeps a value live across the copy), and `qzero(p)`, retail's `sq $zero,0(p)`. Call them; never write asm inside a function.
   - NO BUILD TRANSFORMS: never add or extend a step between the compiler and the assembler that changes the compiler's output, and never build one function with flags its file does not have (GCC 2.95 has no per-function options). The build's only extra steps model retail's assembler and linker; they are listed, with evidence, in `docs/BUILD_FIDELITY.md`, and `tools/check_build_fidelity.py` enforces the list in CI. A function that only matches through a new transform is not matched.
2. MEANINGFUL DATA STRUCTURES OVER VOID CASTS:
   - When encountering struct/array access patterns (e.g., `ptr + 0x14`, `*(int*)(base + idx*8 + 4)`), define clean C `typedef struct { ... }` with proper types, array dimensions, or padding arrays (`char pad0[0x18];`).
   - DO NOT commit messy pointer arithmetic like `*(int*)((char*)p + 0x24) = val;` if a struct definition represents the entity.
3. SIZE & BIT-EXACT MATCHES ONLY:
   - SIZE MISMATCH IS NEVER ACCEPTABLE: A function with different byte length shifts subsequent code and is strictly prohibited from being committed.
   - Commit ONLY functions that achieve exact byte match (`EXACT` in `try_func.py` and pass `tools/build_sn.sh`).
4. ATOMIC, CLEAN COMMITS:
   - One function per commit.
   - Always commit `progress/report.json` together with `src/...`.
   - Never use `git add -A` or `git add .`. Stage only explicit files.

### TOKEN & INTERACTION EFFICIENCY PROTOCOL
- NO FILLER TEXT: Do not output apologies, introductory greetings, pleasantries, or verbose explanations of standard C concepts.
- NO GREP SPAM: Do not run broad grep or find loops across the entire filesystem. Target specific files directly.
- SINGLE-SHOT INVESTIGATION:
  - If the Ghidra MCP container is running (`docs/CONTAINERS.md`): query `decompile_function` with address `0xXXXXXXXX` (strip `func_`).
  - Read retail disassembly directly from `asm/nonmatchings/{text,core_text}/func_XXXXXXXX.s`.
- FAST FEEDBACK LOOP: Use `tools/try_func.py` inside Docker for rapid trial compiles without touching the main tree.

### COMPILATION TARGETS
- Game code (`src/game/`, `text` segment): SN ProDG GCC 2.95.3 (`-O2 -G2`).
- SDK code (`src/core/`, `core_text` segment with `ee29` marker): Sony 2.9-ee (`-O2 -G2`).
- EABI Register Convention:
  - Return: `$v0`, `$v1` (`$2`, `$3`). Float return: `$f0`.
  - Arguments: `$a0`-`$a3`, `$t0`-`$t3` (`$4`-`$11`). Float arguments: `$f12`, `$f13`, `$f14`, ...
  - Saved registers: `$s0`-`$s7`, `$fp` (`$16`-`$23`, `$30`).
  - Global pointer: `$gp` (`$28`), Stack pointer: `$sp` (`$29`), Return address: `$ra` (`$31`).
  - Types: `int` = 32-bit, `long` = 64-bit, `long long` = 128-bit.

### STEP-BY-STEP DECOMPILATION WORKFLOW

#### Step 1: Candidate Selection
Find unblocked leaf or small functions:
```bash
python3 tools/rank_candidates.py text 25
```
Pick a candidate marked `candidate` (not `blocked` or `risky`).

#### Step 2: Fetch References
1. Ghidra Decompilation (via MCP `call_mcp_tool` on `ghidra-ps2`):
   Tool: `decompile_function` with argument `address: "0xXXXXXXXX"`.
2. Retail Disassembly:
   View `asm/nonmatchings/text/func_XXXXXXXX.s`. Check argument usage, branch structures, loops, return registers.

#### Step 3: Write Candidate in `build-sn/try/`
Draft your candidate in `build-sn/try/test_cand.c`:
- Define necessary structs and `extern` declarations at the top.
- Match function signature based on register conventions.
- Write clean, structured ANSI C logic.

#### Step 4: Fast Iteration with `try_func.py`
Run the candidate against retail assembly in Docker:
```bash
bash tools/docker/run.sh python3 tools/try_func.py func_XXXXXXXX build-sn/try/test_cand.c --diff
```
Analyze output:
- `EXACT`: Proceed to Step 5.
- `BYTES n/size`: Use the diff output to inspect instruction order, register allocation, signedness, or branch targets. Adjust C and re-run.
- `SIZE`: The code generated is longer or shorter than retail. Re-examine loop conditions, missing inline operations, or variable types.
- `COMPILE`: Inspect `build-sn/try/func_XXXXXXXX/log.txt` for syntax or type errors.

#### Step 5: Apply Candidate to Source
Apply the candidate to `src/`:
```bash
python3 tools/apply_candidate.py func_XXXXXXXX build-sn/try/test_cand.c
```
This automatically replaces `INCLUDE_ASM` in the relevant source file.

#### Step 6: Full Verification Build
Clean temporary equates and perform a full rebuild and match verification:
```bash
rm -f build-sn/bss_equs.* build-sn/undefined_syms.* && bash tools/docker/run.sh bash tools/build_sn.sh
```
Verify that the output reports:
`exact (size AND bytes): <N+1>` and `size mismatch: 0`.

#### Step 7: Update Progress Report
Update `progress/report.json` without full re-linking:
```bash
bash tools/docker/run.sh python3 tools/gen_progress_report.py --no-build
```

#### Step 8: Git Atomic Commit & Push

> **Inside OpenRAC:** commit as `feat(rac1/pal): <FunctionName> (func_XXXXXXXX) exact match`
> (OpenRAC's CONTRIBUTING.md), keep `progress/report.json` in the same commit, and do
> **not** push: the maintainer publishes (AGENTS.md, rule 7).
1. Inspect git status:
```bash
git status
```
2. Stage only the modified source file and the progress report:
```bash
git add src/path/to/file.c progress/report.json
```
3. Commit with standard message format:
```bash
git commit -m "<unit>: <FunctionName> (func_XXXXXXXX) exact match"
```
4. Push:
```bash
git push origin main
```
```

---

## 2. Podrobný rozbor pravidel a architektonických zásad

### 2.1 Striktní ANSI C a zákaz berliček
V mnoha starších dekompilačních projektech se pro dosažení shody používaly různé triky:
- Vkládání prázdných assembly bariér: `__asm__("" : "+r"(val));`
- Vynucené přiřazení registrů: `register int r asm("$14");`
- Vkládání umělých bloků: `do { ... } while(0);`
- Inline instrukce uvnitř C funkcí: `__asm__("sync.l");`

**V tomto repozitáři jsou tyto praktiky PŘÍSNĚ ZAKÁZÁNY.**
Veškerý kód musí být standardní C. Pokud kód nesedí, problém téměř vždy tkví v:
- chybném typu proměnné (`int` vs `long`, `unsigned` vs `signed`),
- absenci správně definované struktury (GCC 2.95.3 aplikuje odlišná pravidla aliasingu pro členy struktur oproti globálním skalárům),
- tvaru řídicí struktury (pořadí podmínek, `do { ... } while()` vs `for`),
- pořadí vyhodnocování argumentů či přiřazení do proměnných.

Jediné povolené použití `__asm__` je:
1. **Symbol aliasing v deklaracích:**
   ```c
   extern HudArena D_0019A4E8_arena __asm__("D_0019A4E8");
   extern void func_00234C98(int, long) __asm__("func_00234C98");
   ```
2. **Inter-function padding direktivy** na úrovni souboru (mimo funkce), které již existují ve vygenerovaných souborech nebo stubech:
   ```c
   __asm__(".section .text\n\tnop\n");
   ```
3. **Sdílený pomocník `qcopy(dst, src)`** v `include/common.h`: retailová 16bajtová kopie vektoru přes inline asm (`lq`/`sq` přes `$2`, obě adresy s nulovým offsetem), kterou žádný C zápis nevytvoří. Vedle něj jsou `qcopy_nc(dst, src)` (stejná kopie bez clobberu "memory") a `qzero(p)` (retailové `sq $zero,0(p)`). Funkce je volají, samy asm nepíší.

### 2.2 Definice smysluplných C struktur
Nikdy nepište kód ve stylu:
```c
// ŠPATNĚ:
int *ptr = (int *)((char *)header + 0x14 + bank * 4);
*(int *)((char *)arena + 0x18) = val;
```
Místo toho prozkoumejte offsety a definujte C strukturu:
```c
// SPRÁVNĚ:
typedef struct {
    int offset;
    int unk4;
} HudEntry;

typedef struct {
    char pad0[0x14];
    int limits1[1];
    char pad18[0x1C];
    int limits2[1];
    char pad38[0x3C];
    int banks[1];
} HudBankHeader;

typedef struct {
    char pad0[0x18];
    HudBankHeader *header;
    char pad1C[8];
    HudEntry *table2;
    HudEntry *table1;
} HudArena;
```
Tento přístup nejen zlepšuje čitelnost, ale přímo ovlivňuje generovaný kód: GCC 2.95.3 dokáže u struktur lépe pracovat s bázovými registry a posuny (displacement addressing).

---

## 3. Typické pasti a páky pro GCC 2.95.3 (MIPS EE)

### 3.1 Unsigned vs Signed Immediates
- Častá chyba: maskování adresy `(addr + 0xF) & ~0xF` nebo `(addr + 0xF) & 0xFFFFFFF0`.
  - Pokud kompilátor vygeneruje `addiu reg, reg, -16`, ale retail má `andi reg, reg, 0xFFF0` (nebo naopak), zkontrolujte typy a explicitní `U` příponu:
    ```c
    addr = (addr + 0xF) & 0xFFFFFFF0U;
    ```
- Při porovnávání:
  - `slti` indikuje znaménkové porovnání (`< 0`, `< CONST`).
  - `sltiu` indikuje bezznaménkové porovnání (`< (unsigned int)CONST`).

### 3.2 Array indexace vs Ukazatelová aritmetika (`addu` operand order)
- Typický rozdíl: v retailu je `addu $2, $2, $3` (báze + index), v dekompilaci vyjde `addu $2, $3, $2` (index + báze).
- Přehazování sčítanců v C (`base + idx` vs `idx + base`) nepomáhá, protože GCC normalizuje operátory `PLUS`.
- **Řešení:**
  - `table[idx]` generuje bázi jako první operand (`base first`).
  - `*(table + idx)` nebo `void *p = table + idx` generuje index jako první operand (`index first`).
  - Uložení mezivýpočtu do lokální proměnné:
    ```c
    int off = idx * sizeof(Entry);
    Entry *e = (Entry *)((char *)base + off);
    ```

### 3.3 MIPS Branch Delay Sloty a Branch Likely
- MIPS procesory mají delay slot za každým skokem (`j`, `jal`, `bne`, `beq`, `bc1t`, atd.). Kompilátor se snaží delay slot zaplnit nezávislou instrukcí.
- **Branch Likely (`bnel`, `beql`, `bc1tl`):** Instrukce v delay slotu se provede POUZE tehdy, pokud je větev splněna.
  - Vzniká typicky v `do { ... } while(cond)` cyklech nebo při složených logických podmínkách (`if (a && b)`).
  - Pokud m2c dekompiluje cyklus s `bnel`, obvykle chybně invertuje podmínku. Vždy porovnejte podmínku přímo s retail asm!

### 3.4 Floating Point registry a volací konvence
- Plovoucí čárka používá koprocesor 1 (COP1) s registry `$f0` až `$f31`.
- Argumenty typu `float` se předávají v `$f12`, `$f13`, `$f14`, ...
- Návratová hodnota typu `float` je v `$f0`.
- Konstanty typu float v retailu:
  - Retail v `text` segmentu **nikdy nepoužívá `.lit4` tabulky přes `$gp`**. Všechny konstanty float jsou nahrávány pomocí `mtc1` (přímo z celočíselného registru).

### 3.5 Detekce návratového typu volaných funkcí
- Pokud volaná funkce vrací hodnotu, registr `$v0` (`$2`) je po návratu ihned obsazen.
- Pokud první dočasná hodnota po `jal` používá `$v0`, funkce vrací `void` (nebo je návratová hodnota ignorována a kompilátor `$v0` recykluje).
- Pokud první dočasná hodnota po `jal` používá `$v1` (`$3`), volaná funkce vrací hodnotu a kompilátor drží `$v0` pro výsledek.

---

## 4. Referenční ukázka dekompilace (Gold Standard)

Příklad úspěšně dekompilované funkce `func_001FF7F0` (`LinkHudBank`) ze souboru `src/game/hud.c`:

```c
typedef struct {
    int offset;
    int unk4;
} HudEntry;

typedef struct {
    char pad0[0x14];
    int limits1[1];
    char pad18[0x1C];
    int limits2[1];
    char pad38[0x3C];
    int banks[1];
} HudBankHeader;

typedef struct {
    char pad0[0x18];
    HudBankHeader *header;
    char pad1C[8];
    HudEntry *table2;
    HudEntry *table1;
} HudArena;

extern HudArena D_0019A4E8_arena __asm__("D_0019A4E8");

/* LinkHudBank(int, char *) */
void func_001FF7F0(int bank, int addr) {
    int *banks;
    int *slot;
    int start;
    int limit;
    int i;

    banks = (int *)((char *)D_0019A4E8_arena.header + 0x74);
    slot = &banks[bank];
    if (*slot != 0) {
        return;
    }

    addr = (addr + 0xF) & 0xFFFFFFF0U;
    *slot = addr;

    if (bank != 0) {
        start = D_0019A4E8_arena.header->limits1[bank - 1];
    } else {
        start = 0;
    }
    limit = D_0019A4E8_arena.header->limits1[bank];

    for (i = start; i < limit; i++) {
        D_0019A4E8_arena.table1[i].offset &= 0x7FFFFFFF;
        D_0019A4E8_arena.table1[i].offset += addr;
    }

    if (bank != 0) {
        start = D_0019A4E8_arena.header->limits2[bank - 1];
    } else {
        start = 0;
    }
    limit = D_0019A4E8_arena.header->limits2[bank];

    for (i = start; i < limit; i++) {
        D_0019A4E8_arena.table2[i].offset &= 0x7FFFFFFF;
        D_0019A4E8_arena.table2[i].offset += addr;
    }
}
```

Všimněte si:
1. Žádné inline assembly v těle funkce.
2. Definované struktury `HudEntry`, `HudBankHeader`, `HudArena`.
3. Čisté řízení toku s cykly `for` a podmínkami `if/else`.
4. Přesné maskování `0xFFFFFFF0U`.
5. Jednorázový commit společně s `progress/report.json`.
