// Apply reviewed diagnostic-carrier annotations; preview unless --apply is supplied.
// Usage: <program identifier> <TSV path> [--apply]
// The TSV must have exactly: program TAB address TAB name.
// Build fingerprints cover the entire retail level .text. No function is created.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;

public class ApplyAssertMessageNames extends GhidraScript {
    // program -> {virtual .text base, ELF .text offset, length, SHA-256}.
    private static final Map<String,String[]> BUILDS = new LinkedHashMap<>();
    static {
        BUILDS.put("0_aranos_tutorial",new String[]{"2a3b80","fcf00","1719680","4e867217e54df35b6519230a4f90843eadf62d0bc2f71a4646b99b8d68ce9b66"});
        BUILDS.put("10_hrugis_cloud",new String[]{"2be900","117c80","1729208","16682d9a2e081a98a37505a44f077b7a1284ec4cb60a48f3c08d01b73c247b2f"});
        BUILDS.put("11_joba",new String[]{"2bce00","116180","2016344","45e9f308bcf15461c6fb65b510ac1e1032b432e0c2089d14b34e574412eb8060"});
        BUILDS.put("12_todano",new String[]{"2a9700","102a80","1788320","7ab97805ceb33eb1070450bd2640ba9b7c9c6127212ae4c39a80a2c0f788c53e"});
        BUILDS.put("13_boldan",new String[]{"2aeb00","107e80","1756920","648f201a9096c9f7e645adbdcc9502af9b010dcea8d46667f3218b3f4dfbb7ed"});
        BUILDS.put("14_aranos_prison",new String[]{"2ab300","104680","1812720","7e0a9e5deb494d0825ad8d6d490e535dba7ce8aa17bda2236ad6f2dbf309eccc"});
        BUILDS.put("15_gorn",new String[]{"2bca00","115d80","1782392","46313365e2ab42895c040b251c1878743d3a658effd00d942015ea49c9454efb"});
        BUILDS.put("16_snivelak",new String[]{"2a5400","fe780","1761096","b4c7a465306d942bc81b1669cbfcefb003fc4c0db1a6c9ad13381d429e01dbe6"});
        BUILDS.put("17_smolg",new String[]{"2a5080","fe400","1766224","fe6da137c1f5e84146a9b63519b94f03bd484625d889cbe1532e548d906209b9"});
        BUILDS.put("18_damosel",new String[]{"2c1a80","11ae00","1783368","051aa00a3a34f82ba44b663245fec435a70f87802435e023e3ca517b5cdfd479"});
        BUILDS.put("19_grelbin",new String[]{"2a7600","100980","1775272","77ba948f558e8e02bfb3995e987e95dc81e4666bdf8899a9a19eed87108dc0bd"});
        BUILDS.put("20_yeedil",new String[]{"2b6780","10fb00","1868528","e46f7be49d646353bd3ea076874d1270ffcbef9590f37a2c916ce850f237a327"});
        BUILDS.put("22_dobbo_orbit",new String[]{"2a7a00","100d80","1706240","746ca26a0d9e0e2657f3100dae33deb7a3a8c3a8f60c7f9b47d6db953e7a3413"});
        BUILDS.put("23_damosel_orbit",new String[]{"2ac100","105480","1775672","6d4821842f1b67a15c8c6642c63149eb3553ea85c6ca74b77b64505799a2ff71"});
        BUILDS.put("24_ship_shack",new String[]{"2a2c80","fc000","1588072","ff804352a506901ba372670c54228f687f20a98bd25bf6cea504c528ba49956f"});
        BUILDS.put("25_wupash_nebula",new String[]{"2b6d80","110100","1692160","e9642f9afd885f53114308d6211a5c01ba1aa265bb628dcde287eacbce0a38b3"});
        BUILDS.put("26_jamming_array",new String[]{"2a4180","fd500","1662232","f9af052df457818f1da6091a253b16e28848f7ce4bc7ccef388b6b0b69665e91"});
        BUILDS.put("30_insomniac_museum",new String[]{"2aa980","103d00","1706840","e830e957fa59c867920f730f609267ed56955d5c83828b79253e0cba4c8ecacd"});
        BUILDS.put("3_endako",new String[]{"2aa300","103680","1762672","036f95aa40ceb5ee349b1b2d254cf2e44b179ca2c0cd5fe2c514f4b7d1d2219b"});
        BUILDS.put("4_barlow",new String[]{"2cb480","124800","1833808","5c9314cce4896cef462a218cab7cbc4aba0a85bd7c613edd4cb32a42e58ab430"});
        BUILDS.put("5_feltzin_system",new String[]{"2bc400","115780","1731576","6d83227a8d1235f1a1118de030db03e131fa6bb5a2d864db90e6679ac2c1806c"});
        BUILDS.put("6_notak",new String[]{"2e3180","13c500","1795040","04a05621a1f5f4a6d15b77a69f8ae8bf01d6abf381bb2d3bdc1b46c1799802ab"});
        BUILDS.put("7_siberius",new String[]{"2a4180","fd500","1700632","d85d013ab2fdc7421f97382eb227bc18d0089239f688fb4424d325bd7e05c2dc"});
        BUILDS.put("8_tabora",new String[]{"2b6480","10f800","1758064","31fc0fcb64b86bca288c6d3c22132fb7aab0199c7985ff9147fc396a9164b086"});
        BUILDS.put("9_dobbo",new String[]{"2a7500","100880","1765608","315a6bb5cc08da4d6af515364705a6892aa3ae2ee0b697d35d768a3b3be6663b"});
    }
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2 || args.length > 3 || (args.length == 3 && !args[2].equals("--apply"))) {
            println("usage: <program identifier> <TSV path> [--apply]"); return;
        }
        String id=args[0]; boolean apply=args.length==3;
        if (!currentProgram.getName().equals(id+".elf") || !BUILDS.containsKey(id)) {
            throw new IllegalArgumentException("Program identity is not a supported retail overlay");
        }
        String[] build=BUILDS.get(id);
        long virtualBase=Long.parseLong(build[0],16), fileBase=Long.parseLong(build[1],16);
        int size=Integer.parseInt(build[2]);
        List<Long> validBases=new ArrayList<>();
        for (long base : new long[]{virtualBase,fileBase}) {
            monitor.checkCancelled();
            try {
                byte[] data=new byte[size];
                int got=currentProgram.getMemory().getBytes(address(base),data);
                if (got==size && digest(data).equals(build[3]) && !validBases.contains(base)) validBases.add(base);
            } catch (ghidra.program.model.mem.MemoryAccessException ex) { /* unmapped candidate */ }
        }
        if (validBases.size()!=1) throw new IllegalArgumentException("Retail .text fingerprint/mapping is missing or ambiguous");
        long delta=validBases.get(0)-virtualBase;
        List<String> lines=Files.readAllLines(Paths.get(args[1]),StandardCharsets.UTF_8);
        if (lines.isEmpty() || !lines.get(0).equals("program\taddress\tname")) {
            throw new IllegalArgumentException("Unexpected TSV schema");
        }
        Map<Long,String> names=new LinkedHashMap<>();
        for (int i=1;i<lines.size();i++) {
            monitor.checkCancelled(); String line=lines.get(i);
            if (line.trim().isEmpty()) continue;
            String[] cols=line.split("\t",-1);
            if (cols.length!=3 || !cols[0].matches("[A-Za-z0-9_]+") ||
                !cols[1].matches("0x[0-9a-f]{8}") || !cols[2].matches("[A-Za-z_][A-Za-z_0-9]*")) {
                throw new IllegalArgumentException("Invalid row "+(i+1));
            }
            if (!cols[0].equals(id)) continue;
            long virtual=Long.parseLong(cols[1].substring(2),16);
            if (virtual<virtualBase || virtual>=virtualBase+size) throw new IllegalArgumentException("Entry outside .text");
            String previous=names.putIfAbsent(virtual,cols[2]);
            if (previous!=null && !previous.equals(cols[2])) throw new IllegalArgumentException("Conflicting entry labels");
        }
        int equal=0,protectedNames=0,missing=0,renamed=0;
        Map<Function,String> eligible=new LinkedHashMap<>();
        for (Map.Entry<Long,String> row:names.entrySet()) {
            Function function=currentProgram.getFunctionManager().getFunctionAt(address(row.getKey()+delta));
            if (function==null) { missing++; continue; }
            if (function.getName().equals(row.getValue())) { equal++; continue; }
            if (!function.getName().startsWith("FUN_") || function.getSymbol().getSource()!=SourceType.DEFAULT) {
                protectedNames++; continue;
            }
            eligible.put(function,row.getValue());
        }
        for (Map.Entry<Function,String> row:eligible.entrySet()) {
            monitor.checkCancelled();
            println((apply?"APPLY ":"PREVIEW ")+row.getKey().getEntryPoint()+" -> "+row.getValue());
            if (apply) { row.getKey().setName(row.getValue(),SourceType.USER_DEFINED); renamed++; }
        }
        println("rows="+names.size()+" already_equal="+equal+" protected="+protectedNames+
            " missing_function="+missing+" eligible="+eligible.size()+" renamed="+renamed);
        if (apply) println("Review the changes and save the program explicitly.");
    }
    private Address address(long value) {
        return currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(value);
    }
    private String digest(byte[] data) throws Exception {
        byte[] hash=MessageDigest.getInstance("SHA-256").digest(data);
        StringBuilder text=new StringBuilder();
        for (byte value:hash) text.append(String.format("%02x",value&255));
        return text.toString();
    }
}
