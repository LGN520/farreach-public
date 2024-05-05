package site.ycsb.db;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.Key;
import com.inswitchcache.core.Value;
import com.inswitchcache.core.MyUtils;
import com.inswitchcache.db.DbUdpNative;
import com.inswitchcache.db.DbUdpNativeResult;

import site.ycsb.ByteIterator;
import site.ycsb.DBException;
import site.ycsb.RemoteDB;
import site.ycsb.Status;
import site.ycsb.StringByteIterator;

//import com.inswitchcache.core.packets.GetResponse; // TMPDEBUG

/**
 * RecordloadClient inherits RemoteDB.
 * Pre-store 100M records into storage servers for loading phase.
 */
public class RecordloadClient extends RemoteDB {
  private DbUdpNative dbInterface;

  private int pseudoMethodid = GlobalConfig.INVALID_ID;

  @Override
  public void init() throws DBException {
    super.init();
    System.out.println("[INFO][RecordloadClient] Client logical index: " + localLogicalClientIndex); // TMPDBG

    MyUtils.myAssert(GlobalConfig.getCurmethodId() == GlobalConfig.RECORDLOAD_ID);
    pseudoMethodid = GlobalConfig.NOCACHE_ID;

    this.dbInterface = new DbUdpNative(GlobalConfig.MAX_BUFFER_SIZE, GlobalConfig.MAX_LARGE_BUFFER_SIZE,
        GlobalConfig.CLIENT_SOCKET_TIMEOUT_SECS, 0, GlobalConfig.UDP_DEFAULT_RCVBUFSIZE);
  }

  @Override
  public void cleanup() throws DBException {
    System.out.println("[INFO][RecordloadClient] Logical client " + localLogicalClientIndex + " cleanup"); // TMPDBG
  }

  @Override
  public Status read(final String table, final String key, final Set<String> fields,
      Map<String, ByteIterator> result) {
    System.out.println("[ERROR][RecordloadClient] read should NOT appear in loading phase!");
    System.exit(-1);
    return Status.OK;
  }

  @Override
  public Status scan(final String table, final String startkey, final int recordcount, final Set<String> fields,
      final Vector<HashMap<String, ByteIterator>> result) {
    System.out.println("[ERROR][RecordloadClient] scan should NOT appear in loading phase!");
    System.exit(-1);
    return Status.OK;
  }

  @Override
  public Status update(final String table, final String key, final Map<String, ByteIterator> values) {
    System.out.println("[ERROR][RecordloadClient] update should NOT appear in loading phase!");
    System.exit(-1);
    return Status.OK;
  }

  @Override
  public Status insert(final String table, final String key, final Map<String, ByteIterator> values) {
    // Log information
    // System.out.println("[INFO][RecordloadClient] client " + localLogicalClientIndex + ", update request for key = "
    //     + entry.getKey() + " value = " + entry.getValue());

    // Check value requirement
    Map<String, String> valueMap = StringByteIterator.getStringMap(values);
    //System.out.println(String.format("valuemap size: %d", valueMap.entrySet().size())); // TMPDEBUG
    if (valueMap.entrySet().size() > 1) {
      // System.err.println("[ERROR][RecordloadClient] client " + localLogicalClientIndex + ", key = " + key
      //     + ": farreach update only accept one update value");
      return Status.BAD_REQUEST;
    }

    // prepare request key and value
    Map.Entry<String, String> entry = valueMap.entrySet().iterator().next();
    Value tmpValue = new Value();
    tmpValue.fromString(entry.getValue());
    Key tmpKey = new Key();
    tmpKey.fromString(key);
    // NOTE: loading phase does NOT need to consider rulemap for dynamic workload
    /*if (isDynamicWorkload) {
      tmpKey = InswitchCacheClient.getDynamicRulemap().trymap(tmpKey);
    }*/

    // send and recv
    DbUdpNativeResult dbResult = this.dbInterface.loadNative(pseudoMethodid, tmpKey, tmpValue,
        (short) this.globalClientLogicalIndex, GlobalConfig.getServerIp(),  (short)(GlobalConfig.getServerWorkerPortStart() +
          (short) tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
                  GlobalConfig.getServerTotalLogicalNum())%GlobalConfig.getserverPerServerLogicalNum()));

    // TMPDEBUG
    //System.out.println(String.format("[LOAD] valuestr (size %d): %s", entry.getValue().length(), entry.getValue()));
    //System.out.println(String.format("[LOAD] value (size %d): %s", tmpValue.toString().length(), tmpValue.toString()));
    //DbUdpNativeResult dbResult2 = this.dbInterface.getNative(pseudoMethodid, tmpKey,
    //    GlobalConfig.getServerIp(), GlobalConfig.getServerWorkerPortStart() +
          // (short) tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
          //         GlobalConfig.getServerTotalLogicalNum())%GlobalConfig.getserverPerServerLogicalNum());
    //GetResponse rsp = new GetResponse(pseudoMethodid, dbResult2.getPktContent(), dbResult2.getPktContent().length);
    //System.out.println(String.format("[GET] value (size %d): %s", rsp.val().toString().length(), rsp.val().toString()));
    //System.exit(-1);

    // NOTE: loading phase does NOT need to count cache hit rate (dbResult.nodeIdxForEval must = 0 for LOADACK)
    /*if (dbResult.getNodeIdxForEval() == -1) {
      InswitchCacheClient.increCacheHitCount();
    }*/

    return Status.OK;
  }

  @Override
  public Status delete(final String table, final String key) {
    System.out.println("[ERROR][RecordloadClient] delete should NOT appear in loading phase!");
    System.exit(-1);
    return Status.OK;
  }
}
