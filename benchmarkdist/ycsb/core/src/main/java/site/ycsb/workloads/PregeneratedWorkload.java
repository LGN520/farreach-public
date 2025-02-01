package site.ycsb.workloads;

//import java.io.FileInputStream;
import java.io.File;
import java.io.RandomAccessFile;
import java.nio.BufferUnderflowException;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Properties;
import java.util.Vector;

import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.IOHelper;
import com.inswitchcache.core.InmemoryReq;
import com.inswitchcache.core.MyUtils;
import com.inswitchcache.core.packets.PacketType;

import site.ycsb.ByteIterator;
import site.ycsb.DB;
import site.ycsb.DBWrapper;
import site.ycsb.RandomByteIterator;
import site.ycsb.RemoteDB;
import site.ycsb.Workload;
import site.ycsb.WorkloadException;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
/**
 * PregeneratedWorkload to load pregenerated workload for server rotation during transaction phase.
 */
public class PregeneratedWorkload extends Workload {
  private static ArrayList<InmemoryReq>[] perclientInmemoryReqList = null;
  private static int[] perclientCurreqIdx = null;

  private static void close(RandomAccessFile tmpfile, FileChannel fis) {
    try {
      fis.close();
      tmpfile.close();
    } catch (Exception e) {
      System.out.println(e.getMessage());
      e.printStackTrace();
      System.exit(-1);
    }
  }

  public void init(Properties p) throws WorkloadException {
    MyUtils.myAssert(GlobalConfig.getWorkloadMode() == 0);

    perclientInmemoryReqList = new ArrayList[GlobalConfig.getCurrentClientLogicalNum()];
    perclientCurreqIdx = new int[GlobalConfig.getCurrentClientLogicalNum()];
    // Create a fixed-size thread pool with the number of available processors
    int numThreads = Runtime.getRuntime().availableProcessors();
    ExecutorService executor = Executors.newFixedThreadPool(numThreads);
    String pregenerateDirpath = IOHelper.getPregenerateWorkloadDirpath(GlobalConfig.getWorkloadName());
    File tmpdir = new File(pregenerateDirpath);
    if (!tmpdir.exists()) {
      System.out.println(String.format("[ERROR] No such directory %s, please run keydump before static workload!", pregenerateDirpath));
      System.exit(-1);
    }
    for (int localClientidx = 0; localClientidx < perclientInmemoryReqList.length; localClientidx++) {
      perclientInmemoryReqList[localClientidx] = new ArrayList<InmemoryReq>();
      perclientCurreqIdx[localClientidx] = 0;

      final int finalLocalClientidx = localClientidx;
      executor.submit(() -> {
          loadWorkload(finalLocalClientidx);
      });
    }
    executor.shutdown();
    try {
      // Wait for all tasks to complete
      executor.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
    } catch (InterruptedException e) {
      System.out.println(String.format("Handle interruption"));
        // Handle interruption
    }
  }
  private void loadWorkload(int localClientidx) {
    // Load per-logical-client pre-generated workload
    String pregenerateFilepath = IOHelper.getPregenerateWorkloadFilepath(
        GlobalConfig.getWorkloadName(), localClientidx);
    File tmpfile = new File(pregenerateFilepath);
    if (!tmpfile.exists()) {
      System.out.println(String.format("[ERROR] No such file %s, please run keydump before static workload!", pregenerateFilepath));
      System.exit(-1);
    } else {
      System.out.println(String.format("Load pre-generate static workload for server rotation from %s", pregenerateFilepath));
    }

    // ArrayList<InmemoryReq> tmparray = perclientInmemoryReqList[localClientidx];
    RandomAccessFile pregenerateFile = null;
    FileChannel fis = null;
    try {
      //FileInputStream fis = new FileInputStream(pregenerateFilepath);
      pregenerateFile = new RandomAccessFile(pregenerateFilepath, "rw");
      fis = pregenerateFile.getChannel();
      // map all content into memory
      MappedByteBuffer buffer = fis.map(FileChannel.MapMode.READ_WRITE, 0, fis.size());
      System.out.println(String.format("file size: %d",fis.size()));
      System.out.println("mmap finish");
      // int load_idx = 0;
      while (true) {
        InmemoryReq tmpreq = new InmemoryReq();
        int tmpreqSize = tmpreq.bufferDeserialize(buffer);
        // load_idx ++;
        // if (load_idx % 2000000 == 0){
          
        //   System.out.println(String.format("load InmemoryReq %d",load_idx));
        //   System.gc();
        // }
        // Filter for server rotation to avoid invalid CPU cycles
        int tmpServeridx = tmpreq.getKey().getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
            GlobalConfig.getServerTotalLogicalNumForRotation());
        if (GlobalConfig.checkServerLogicalIdx(tmpServeridx)) {
          perclientInmemoryReqList[localClientidx].add(tmpreq);
        }
        tmpreq = null;
      }
      // for(int i = 0; i < 5; i++) {
      //   InmemoryReq tmpreq = new InmemoryReq();
      //   int tmpreqSize = tmpreq.bufferDeserialize(buffer);
      //   // load_idx ++;
      //   // if (load_idx % 2000000 == 0){
          
      //   //   System.out.println(String.format("load InmemoryReq %d",load_idx));
      //   //   System.gc();
      //   // }
      //   // Filter for server rotation to avoid invalid CPU cycles
      //   int tmpServeridx = tmpreq.getKey().getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
      //       GlobalConfig.getServerTotalLogicalNumForRotation());
      //   if (GlobalConfig.checkServerLogicalIdx(tmpServeridx)) {
      //     perclientInmemoryReqList[localClientidx].add(tmpreq);
      //   }
      //   tmpreq = null;
      // }
    } catch (Exception e) {
      if (e instanceof BufferUnderflowException) {
        // EOF
      } else {
        System.out.println(e.getMessage());
        e.printStackTrace();
        System.exit(-1);
      }
    } finally {
      System.out.println(String.format("inmemory req array of client %d size: %d",
          localClientidx, perclientInmemoryReqList[localClientidx].size()));
      close(pregenerateFile, fis);
      // System.gc();
    }
    
  }

  // NOTE: initThread is counted into execution time
  public Object initThread(Properties p, int mythreadid, int threadcount) throws WorkloadException {
    MyUtils.myAssert(mythreadid >= 0 && mythreadid < perclientInmemoryReqList.length);

    return null;
  }

  @Override
  public boolean doInsert(DB db, Object threadstate) {
    MyUtils.myAssert(db instanceof DBWrapper);
    DB tmpdb = ((DBWrapper) db).getDb();
    MyUtils.myAssert(tmpdb instanceof RemoteDB);
    int localLogicalClientIndex = ((RemoteDB) tmpdb).getLocalLogicalClientIndex();

    InmemoryReq tmpreq = null;
    int curidx = perclientCurreqIdx[localLogicalClientIndex];
    if (curidx < perclientInmemoryReqList[localLogicalClientIndex].size()) {
      tmpreq = perclientInmemoryReqList[localLogicalClientIndex].get(curidx);
      perclientCurreqIdx[localLogicalClientIndex] = curidx + 1;
    } else {
      tmpreq = perclientInmemoryReqList[localLogicalClientIndex].get(0);
      perclientCurreqIdx[localLogicalClientIndex] = 1;
    }

    String table = "usertable";
    HashSet<String> fields = null;
    String tmpkey = tmpreq.getKey().toString();

    String tmpfield = "";
    ByteIterator tmpvaldata = new RandomByteIterator(128);
    HashMap<String, ByteIterator> values = new HashMap<>();
    values.put(tmpfield, tmpvaldata);
    db.insert(table, tmpkey, values);

    return true;
  }

  @Override
  public boolean doTransaction(DB db, Object threadstate) {
    MyUtils.myAssert(db instanceof DBWrapper);
    DB tmpdb = ((DBWrapper) db).getDb();
    MyUtils.myAssert(tmpdb instanceof RemoteDB);
    int localLogicalClientIndex = ((RemoteDB) tmpdb).getLocalLogicalClientIndex();

    InmemoryReq tmpreq = null;
    int curidx = perclientCurreqIdx[localLogicalClientIndex];
    if (curidx < perclientInmemoryReqList[localLogicalClientIndex].size()) {
      tmpreq = perclientInmemoryReqList[localLogicalClientIndex].get(curidx);
      perclientCurreqIdx[localLogicalClientIndex] = curidx + 1;
    } else {
      // (1) Loop until 10 seconds
      tmpreq = perclientInmemoryReqList[localLogicalClientIndex].get(0);
      perclientCurreqIdx[localLogicalClientIndex] = 1;

      // DEPRECATED: (2) return false to stop client thread, which will count down completeLatch to stop status thread
      // NOTE: if we return false, client threads will still be alive yet without increasing opsdone, then the thpt is wrong
      //return false;
    }

    String table = "usertable";
    HashSet<String> fields = null;
    HashMap<String, ByteIterator> cells = new HashMap<String, ByteIterator>();
    String tmpkey = tmpreq.getKey().toString();

    // TMPDEBUG
    /*int totalsize = 0;
    for (int i = 0; i < perclientInmemoryReqList.length; i++) {
      totalsize += perclientInmemoryReqList[i].size();
    }
    System.out.println(String.format("client: %d, reqarray size: %d, total size: %d\n", localLogicalClientIndex,
        perclientInmemoryReqList[localLogicalClientIndex].size(), totalsize));
    int serveridx = tmpreq.getKey().getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
        GlobalConfig.getServerTotalLogicalNumForRotation());
    System.out.println(String.format("key: %s, serveridx: %d", tmpkey, serveridx));
    return false;*/

    short tmptype = tmpreq.getType();
    if (tmptype == PacketType.GETREQ.getType()) {
      db.read(table, tmpkey, fields, cells);
    } else if (tmptype == PacketType.PUTREQ.getType()) {
      String tmpfield = "";
      ByteIterator tmpvaldata = new RandomByteIterator(tmpreq.getVallen());
      HashMap<String, ByteIterator> values = new HashMap<>();
      values.put(tmpfield, tmpvaldata);
      db.update(table, tmpkey, values);
    } else if (tmptype == PacketType.DELREQ.getType()) {
      db.delete(table, tmpkey);
    } else if (tmptype == PacketType.SCANREQ.getType()) {
      db.scan(table, tmpkey, tmpreq.getRecordCnt(), fields, new Vector<HashMap<String, ByteIterator>>());
    } else {
      System.out.println(String.format("[ERROR][PregeneratedWorkload] invalid type: %d", tmpreq.getType()));
      System.exit(-1);
    }

    return true;
  }
}
