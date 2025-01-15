import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ThreadLocalRandom;

public class LoadStatisticsTest {

    private static ArrayList<ConcurrentHashMap<Key, UpstreamBackup>> perclientUpstreamBackups;

    public static void main(String[] args) {
        // Initialize the data structure
        int clientCount = 1;
        perclientUpstreamBackups = new ArrayList<>();
        for (int i = 0; i < clientCount; i++) {
            ConcurrentHashMap<Key, UpstreamBackup> map = new ConcurrentHashMap<>();
            for (int j = 0; j < 10000; j++) {
                Key key = new Key(j, j + 1, j + 2, j + 3);
                Value value = new Value(new byte[128], 128);
                UpstreamBackup backup = new UpstreamBackup(value, j, true);
                map.put(key, backup);
            }
            perclientUpstreamBackups.add(map);
        }

        // Measure time for 1000 updates
        long totalTime = 0;
        int iterations = 10000000;
        for (int i = 0; i < iterations; i++) {
            Key randomKey = generateRandomKey();
            Value randomValue = generateRandomValue();
            DbUdpNativeResult result = new DbUdpNativeResult(
                    randomKey,
                    randomValue,
                    i,
                    true,
                    -1
            );

            long startTime = System.nanoTime();
            updateLoadStatistics(0, result);
            long endTime = System.nanoTime();
            totalTime += (endTime - startTime);
        }

        // Output the average time taken
        System.out.println("Average time taken to update: " + (totalTime / iterations) + " ns");
    }

    private static Key generateRandomKey() {
        int base = ThreadLocalRandom.current().nextInt(0, 10000);
        return new Key(base, base + 1, base + 2, base + 3);
    }

    private static Value generateRandomValue() {
        ThreadLocalRandom random = ThreadLocalRandom.current();
        byte[] data = new byte[128];
        random.nextBytes(data);
        return new Value(data, 128);
    }

    public static void updateLoadStatistics(int clientLogicalIdx, DbUdpNativeResult result) {

        UpstreamBackup existingBackup = perclientUpstreamBackups.get(clientLogicalIdx).get(result.getKey());
        if (existingBackup == null || existingBackup.getSeq() < result.getSeq()) {
            UpstreamBackup newBackup = new UpstreamBackup(result.getValue(), result.getSeq(), result.isStat());
            perclientUpstreamBackups.get(clientLogicalIdx).put(result.getKey(), newBackup);
        }
    }
}

class DbUdpNativeResult {
    private Key key;
    private Value value;
    private int seq;
    private boolean stat;
    private long nodeIdxForEval;

    public DbUdpNativeResult(Key key, Value value, int seq, boolean stat, long nodeIdxForEval) {
        this.key = key;
        this.value = value;
        this.seq = seq;
        this.stat = stat;
        this.nodeIdxForEval = nodeIdxForEval;
    }

    public Key getKey() {
        return key;
    }

    public Value getValue() {
        return value;
    }

    public int getSeq() {
        return seq;
    }

    public boolean isStat() {
        return stat;
    }

    public long getNodeIdxForEval() {
        return nodeIdxForEval;
    }
}

class GlobalConfig {
    public static final int FARREACH_ID = 1;

    public static int getCurmethodId() {
        return FARREACH_ID;
    }
}

class Key {
    private int keylolo, keylohi, keyhilo, keyhihi;

    public Key(int keyLoLo, int keyLoHi, int keyHiLo, int keyHiHi) {
        this.keylolo = keyLoLo;
        this.keylohi = keyLoHi;
        this.keyhilo = keyHiLo;
        this.keyhihi = keyHiHi;
    }

    public int getHashPartitionIdx(int partitionCount, int maxServerNum) {
        return (keylolo ^ keylohi ^ keyhilo ^ keyhihi) % partitionCount;
    }
}

class Value {
    private byte[] valData;
    private int valLength;

    public Value(byte[] valData, int valLength) {
        this.valData = valData;
        this.valLength = valLength;
    }
}

class UpstreamBackup {
    private Value value;
    private int seq;
    private boolean stat;

    public UpstreamBackup(Value value, int seq, boolean stat) {
        this.value = value;
        this.seq = seq;
        this.stat = stat;
    }

    public int getSeq() {
        return seq;
    }
}
