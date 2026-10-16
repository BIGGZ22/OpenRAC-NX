import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
import java.nio.file.*;
import java.util.*;

public class L18Decomp extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();   // names.tsv request.txt outdir
        Map<String, Long> byName = new HashMap<>();
        int made = 0;
        for (String l : Files.readAllLines(Paths.get(a[0]))) {
            String[] p = l.split("\t");
            long addr = Long.parseLong(p[0], 16);
            Address ad = toAddr(addr);
            if (!currentProgram.getMemory().contains(ad)) continue;
            try {
                disassemble(ad);
                Function f = getFunctionAt(ad);
                if (f == null) { f = createFunction(ad, p[1]); }
                else { f.setName(p[1], SourceType.USER_DEFINED); }
                if (f != null) { byName.put(p[1], addr); made++; }
            } catch (Exception e) { /* duplicate name or bad bytes: skip */ }
        }
        println("functions created/renamed: " + made);
        Files.createDirectories(Paths.get(a[2]));
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        for (String n : Files.readAllLines(Paths.get(a[1]))) {
            Long addr = byName.get(n);
            if (addr == null) { println("no function " + n); continue; }
            Function f = getFunctionAt(toAddr(addr));
            DecompileResults r = di.decompileFunction(f, 180, monitor);
            String c = r.decompileCompleted() ? r.getDecompiledFunction().getC()
                                              : "// decompile failed: " + r.getErrorMessage();
            Files.write(Paths.get(a[2], n + ".c"), c.getBytes());
        }
    }
}
