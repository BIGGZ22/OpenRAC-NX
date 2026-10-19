import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;

public class PrepareFullModule extends GhidraScript {
    @Override
    public void run() throws Exception {
        String catalogue = getScriptArgs()[0];
        long codeEnd = 0;
        for (String line : Files.readAllLines(Paths.get(catalogue), StandardCharsets.UTF_8)) {
            String[] fields = line.split("\t");
            if (fields.length > 2 && fields[0].equals(currentProgram.getName())) {
                codeEnd = Long.decode(fields[2]);
                break;
            }
        }
        Memory memory = currentProgram.getMemory();
        Address start = currentProgram.getAddressFactory().getAddress("0x0");
        MemoryBlock header = memory.getBlock(start);
        if (header == null || codeEnd <= 0x80 ||
                start.add(codeEnd).compareTo(header.getEnd()) >= 0) {
            throw new IllegalStateException("full SNR2 module or pinned code interval missing");
        }
        memory.split(header, start.add(0x80));
        MemoryBlock code = memory.getBlock(start.add(0x80));
        memory.split(code, start.add(codeEnd));
        MemoryBlock data = memory.getBlock(start.add(codeEnd));
        header.setExecute(false);
        code.setExecute(true);
        data.setExecute(false);
        println(String.format("full module code interval 0x80..0x%X", codeEnd));
    }
}
