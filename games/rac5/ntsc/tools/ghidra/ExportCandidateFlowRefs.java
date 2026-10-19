import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.Reference;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;

public class ExportCandidateFlowRefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        File input = new File(args[0], currentProgram.getName() + ".flow_review.tsv");
        File output = new File(args[1], currentProgram.getName() + ".refs.tsv");
        if (!input.isFile() || !output.getParentFile().isDirectory()) {
            throw new IllegalStateException("private code-only flow input or output directory missing");
        }
        try (PrintWriter out = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(output), StandardCharsets.UTF_8))) {
            out.println("target\tfrom\ttype\tcomputed\tjump\tcall");
            for (String row : Files.readAllLines(input.toPath(), StandardCharsets.UTF_8)) {
                if (!row.contains("\tinside_other_body")) continue;
                String targetText = row.split("\t")[0];
                Address target = currentProgram.getAddressFactory().getAddress(targetText);
                if (target == null) throw new IllegalStateException("invalid candidate address");
                for (Reference ref : currentProgram.getReferenceManager().getReferencesTo(target)) {
                    out.printf("%s\t0x%X\t%s\t%b\t%b\t%b%n", targetText,
                            ref.getFromAddress().getOffset(), ref.getReferenceType().getName(),
                            ref.getReferenceType().isComputed(),
                            ref.getReferenceType().isJump(), ref.getReferenceType().isCall());
                }
            }
        }
    }
}
