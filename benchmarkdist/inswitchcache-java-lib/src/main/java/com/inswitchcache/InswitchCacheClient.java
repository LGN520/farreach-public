/**
 * Copyright (c) 2010-2016 Yahoo! Inc., 2017 YCSB contributors All rights reserved.
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You
 * may obtain a copy of the License at
 * <p>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 */

package com.inswitchcache;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

import com.inswitchcache.core.DynamicArray;
import com.inswitchcache.core.DynamicRulemap;
import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.IOHelper;
import com.inswitchcache.core.Key;
import com.inswitchcache.core.MyUtils;
import com.inswitchcache.core.OutbandPacketFormat;
import com.inswitchcache.core.SocketHelper;
import com.inswitchcache.core.UdpAddr;
import com.inswitchcache.core.UpstreamBackup;
import com.inswitchcache.core.StatisticsHelper;
import com.inswitchcache.db.DbUdpNativeResult;

/**
 * Extra functions for the main class to execute YCSB under
 * InswitchCache-related databases.
 */
public final class InswitchCacheClient {
  private InswitchCacheClient() {
    // not used
  }

  /**
   * [CLI PARAMETERS]
   * PHYSICAL_CLIENT_INDEX: indicate the physical index of this client.
   * SINGLE_ROTATION: indicate whether the current run is for a single rotation.
   */
  public static final String PHYSICAL_CLIENT_INDEX = "physicalclientindex";
  public static final String SINGLE_ROTATION = "singlerotation";

  /**
   * [INIT PARAMETERS]
   * LOCAL_LOGICAL_CLIENT_INDEX: indicate the local logical index.
   */
  public static final String LOCAL_LOGICAL_CLIENT_INDEX = "locallogicaclientindex";

  /**
   * [DEPRECATED CLI PARAMETERS]
   * WORKLOAD_DUMPING_FILE: indicate the dumping file path of this client.
   */
  // public static final String WORKLOAD_DUMPING_FILE = "workloaddumpingfile";

  /**
   * [DEPRECATED VARIABLES]
   * running: indicate the running status of the clients.
   * initialized: indicate the initialized status of inswitch cache client.
   * finishedThreadCnt: indicate the job finish of all client instances.
   */
  // private static boolean running;
  // private static boolean initialized = false;
  // private static AtomicInteger finishedThreadCnt;

  /**
   * [STATISTICS]
   * switchAndServerLoadList: help the server load calculation of current client
   * (0: cache hit count; i+1: load of logical server i).
   * cachehitLoadList: help per-server throughput calculation of current client
   * (i: cache hits of server i)
   */
  private static AtomicInteger[] switchAndServerLoadList;
  private static AtomicInteger[] cachehitLoadList;
  private static void printArray(AtomicInteger[] array) {
    for (AtomicInteger element : array) {
        System.out.print(element.get() + " ");
    }
    System.out.println();
  }
  /**
   * [DEBUG INFO]
   * syncPeriodKeyKeyRecordsList: record the key mapping for different periods.
   * syncPeriodKeyKeyFreqList: record the key mapping frequence for different
   * periods.
   */
  //// private static ArrayList<HashMap<String, String>> periodKeyKeyRecordsList;
  // private static List<HashMap<String, String>> syncPeriodKeyKeyRecordsList;
  //// private static ArrayList<HashMap<String, Integer>> periodKeyKeyFreqList;
  // private static List<HashMap<String, Integer>> syncPeriodKeyKeyFreqList;

  // DEPRECATED: clt/svr sockets for statistics report under static workload with
  // server rotation
  // NOTE: now we use StatisticsHelper to dump per-physical-client statistics into
  // disk under server rotation,
  // and use scripts to aggregate the dumped statistics.

  /**
   * [KEYDUMP INFO]
   * keyFrequencyRecordsList: record key frequencies of each client thread.
   * totalFrequency: total frequency of entire workload.
   * perserverLoadList16/32/64/128: per-server load used to calculate bottleneck
   * server idx under 16/32/64/128 servers.
   * // perclientInmemoryReqList: per-logical-client workload (DEPRECATED due to
   * JVM memory limitation under 100M records).
   */
  private static ArrayList<HashMap<String, Integer>> keyFrequencyRecordsList;
  private static int totalFrequency = 0;
  private static AtomicInteger[] perserverLoadList16 = null;
  private static AtomicInteger[] perserverLoadList32 = null;
  private static AtomicInteger[] perserverLoadList64 = null;
  private static AtomicInteger[] perserverLoadList128 = null;
  // private static ArrayList<InmemoryReq>[] perclientInmemoryReqList = null;

  /**
   * [DYNAMIC WORKLOAD]
   * dynamicRulemap: rulemaps for dynamic workload.
   * currentDynamicPeriod: current dynamic period.
   */
  private static DynamicRulemap dynamicRulemap;
  private static int currentDynamicPeriod = 0;

  /**
   * [UPSTREAM BACKUP]
   * perclientUpstreamBackups: per-logical-client upstream backups (i.e., record
   * preservations).
   */
  private static ArrayList<ConcurrentHashMap<Key, UpstreamBackup>> perclientUpstreamBackups = null;

  /**
   * [Workflow control]
   * isStop: whether the test is stopped.
   */
  private static boolean isStop = false;

  /**
   * InSwitch Cache enter functions section.
   * - initBenchmark(): initialize general configs, properties and structs.
   * - preBenchmark(): coordinate client threads.
   * - postBenchmark(): collect workload results after client threads finish.
   */
  public static void initBenchmark(String dbname, Properties props) {
    // check required properties
    int tmpClientPhysicalIdx = Integer.parseInt(props.getProperty(PHYSICAL_CLIENT_INDEX, "-1"));
    boolean tmpSingleRotation = (Integer.parseInt(props.getProperty(SINGLE_ROTATION, "0")) == 1);
    // String tmpWorkloadDumpFileName = props.getProperty(WORKLOAD_DUMPING_FILE,
    // "");
    // if (!checkRequiredProperties(dbname, tmpClientPhysicalIdx,
    // tmpWorkloadDumpFileName)) {

    // check required parameters
    if (dbname.toLowerCase().contains("keydump") || dbname.toLowerCase().contains("recordload")) {
      // if (workloadDumpFileName == "") {
      // System.err.println("[ERROR][InSwitchCacheClient] Missing property: " +
      // WORKLOAD_DUMPING_FILE);
      // return false;
      // }
      tmpClientPhysicalIdx = 0;
    } else if (tmpClientPhysicalIdx == -1) {
      System.err.println("[ERROR][InSwitchCacheClient] Missing property: " + PHYSICAL_CLIENT_INDEX);
      usageMessage();
      System.exit(-1);
    }

    // initialize global config
    // GlobalConfig.init(dbname, tmpClientPhysicalIdx, tmpWorkloadDumpFileName);
    GlobalConfig.init(dbname, tmpClientPhysicalIdx, tmpSingleRotation);

    switch (GlobalConfig.getCurmethodId()) {
      case GlobalConfig.FARREACH_ID:
        System.out.println("[INFO][InSwitchCacheClient] Initializing perclientUpstreamBackups for FarReach.");
        perclientUpstreamBackups = new ArrayList<>();
        for (int i = 0; i < GlobalConfig.getCurrentClientLogicalNum(); i++) {
          perclientUpstreamBackups.add(new ConcurrentHashMap<>());
        }
      case GlobalConfig.NOCACHE_ID:
      case GlobalConfig.NETCACHE_ID:
        System.out.println(
            String.format("[INFO][InSwitchCacheClient] Initializing common parts for %s.", GlobalConfig.getDatabase()));
        // NOTE: 0 refers to cache hit count by switch; i+1 refers to load of logical
        // server i
        switchAndServerLoadList = new AtomicInteger[GlobalConfig.getMaxServerTotalLogicalNum() + 1];
        for (int i = 0; i < GlobalConfig.getMaxServerTotalLogicalNum() + 1; i++) {
          switchAndServerLoadList[i] = new AtomicInteger();
        }
        cachehitLoadList = new AtomicInteger[GlobalConfig.getMaxServerTotalLogicalNum()];
        for (int i = 0; i < GlobalConfig.getMaxServerTotalLogicalNum(); i++) {
          cachehitLoadList[i] = new AtomicInteger();
        }
        if (GlobalConfig.isNonStableDynamicPattern()) {
          dynamicRulemap = new DynamicRulemap(GlobalConfig.getDynamicPeriodNum(),
              IOHelper.getDynamicRulepath(GlobalConfig.getWorkloadName(), GlobalConfig.getDynamicRulePrefix()));
          dynamicRulemap.nextperiod();

          currentDynamicPeriod = 0;
        }
        break;
      case GlobalConfig.KEYDUMP_ID:
        System.out.println("[INFO][InSwitchCacheClient] Initializing keydump workload.");
        totalFrequency = 0;
        // finishedThreadCnt = new AtomicInteger();
        keyFrequencyRecordsList = new ArrayList<HashMap<String, Integer>>();
        for (int i = 0; i < GlobalConfig.getCurrentClientLogicalNum(); i++) {
          keyFrequencyRecordsList.add(new HashMap<String, Integer>());
        }
        perserverLoadList16 = new AtomicInteger[16];
        for (int i = 0; i < 16; i++) {
          perserverLoadList16[i] = new AtomicInteger();
        }
        perserverLoadList32 = new AtomicInteger[32];
        for (int i = 0; i < 32; i++) {
          perserverLoadList32[i] = new AtomicInteger();
        }
        perserverLoadList64 = new AtomicInteger[64];
        for (int i = 0; i < 64; i++) {
          perserverLoadList64[i] = new AtomicInteger();
        }
        perserverLoadList128 = new AtomicInteger[128];
        for (int i = 0; i < 128; i++) {
          perserverLoadList128[i] = new AtomicInteger();
        }
        // perclientInmemoryReqList = new
        // ArrayList[GlobalConfig.getCurrentClientLogicalNum()];
        // for (int i = 0; i < perclientInmemoryReqList.length; i++) {
        // perclientInmemoryReqList[i] = new ArrayList<InmemoryReq>();
        // }
        // check directory for pre-generated workload
        String pregenerateDirpath = IOHelper.getPregenerateWorkloadDirpath(GlobalConfig.getWorkloadName());
        File directoryFile = new File(pregenerateDirpath);
        if (!directoryFile.exists()) {
          directoryFile.mkdir();
        } else {
          System.out
              .println(String.format("[WARN][KeydumpClient] %s already exists, please delete it before running keydump",
                  pregenerateDirpath));
          System.exit(-1);
        }
        break;
      case GlobalConfig.RECORDLOAD_ID:
        break;
      default:
        System.out.println("[WARN][InSwitchCacheClient] Initializing functions not ready yet.");
        break;
    }
  }

  // Before any sub-thread starting packet sending
  public static void preBenchmark() {
    switch (GlobalConfig.getCurmethodId()) {
      case GlobalConfig.FARREACH_ID:
        System.out.println("[INFO][InSwitchCacheClient] Launch ClientUpstreamBackupReleaser for FarReach");
        Thread runClientUpstreamBackupReleaserThread = new Thread() {
          public void run() {
            runClientUpstreamBackupReleaser();
          }
        };
        runClientUpstreamBackupReleaserThread.start();
      case GlobalConfig.NOCACHE_ID:
      case GlobalConfig.NETCACHE_ID:
        System.out.println("[INFO][InSwitchCacheClient] Before method start");
        runSendPkt();
        if (GlobalConfig.isDynamicPattern() &&
            GlobalConfig.getClientPhysicalIdx() != 0 && GlobalConfig.getClientPhysicalNum() > 1) {
          Thread runClientRulemapServerThread = new Thread() {
            public void run() {
              runClientRulemapServer();
            }
          };
          runClientRulemapServerThread.start();
        }
        break;
      case GlobalConfig.KEYDUMP_ID:
        System.out.println("[INFO][InSwitchCacheClient] Before keydump start");
        // NOTE: we only use a single physical client for keydump
        break;
      case GlobalConfig.RECORDLOAD_ID:
        System.out.println("[INFO][InSwitchCacheClient] Before recordload start");
        runSendPkt();
        break;
      default:
        System.out
            .println(String.format("[ERROR][InSwitchCacheClient] invalid methodid: %d", GlobalConfig.getCurmethodId()));
        System.exit(-1);
        break;
    }
  }

  // After all sub-threads starting packet sending yet before finishing (between
  // preBenchmark and postBenchmark)
  public static void withinBenchmark() {
    switch (GlobalConfig.getCurmethodId()) {
      case GlobalConfig.FARREACH_ID:
        try {
          if (GlobalConfig.getClientPhysicalIdx() == 0) {
            // System.out.println("[INFO][InswitchCacheClient] enable snapshot for FarReach " +
            //     "(TODO: double-check tmp_controller.out and tmp_controller_bwcost.out).");

            // // notify server for controller-based snapshot
            // String cmd = "./preparefinish_client";
            // File dir = new File("../../farreach");
            // Process process = Runtime.getRuntime().exec(cmd, null, dir);
            // int status = process.waitFor();
            // if (status != 0) {
            //   System.err.println(
            //       "[ERROR][InswitchCacheCLient] Failed to execute preparefinish_client for FarReach: " + status);
            //   // System.exit(-1); // NOTE: NOT exit to avoid incorrect system behavior (e.g.,
            //   // server rotation are executed without dumping statistics)
            // }
            System.out.println("[INFO][InswitchCacheClient] no need snapshot");

            // TMPDEBUG: dump output information of preparefinish_client
            /*
             * System.out.
             * println("[INFO][InswitchCacheClient] dump output information of preparefinish_client"
             * );
             * BufferedReader inputbuf = new BufferedReader(new
             * InputStreamReader(process.getInputStream(), "UTF-8"));
             * String line = null;
             * while ((line = inputbuf.readLine()) != null) {
             * System.out.println(line);
             * }
             */
          }
        } catch (Exception e) {
          e.printStackTrace();
        }
        break;
      case GlobalConfig.NOCACHE_ID:
      case GlobalConfig.NETCACHE_ID:
      case GlobalConfig.KEYDUMP_ID:
      case GlobalConfig.RECORDLOAD_ID:
        break;
      default:
        System.out
            .println(String.format("[ERROR][InSwitchCacheClient] invalid methodid: %d", GlobalConfig.getCurmethodId()));
        System.exit(-1);
        break;
    }
  }

  // After all sub-threads finishing packet sending
  public static void postBenchmark() {
    switch (GlobalConfig.getCurmethodId()) {
      case GlobalConfig.FARREACH_ID:
      case GlobalConfig.NOCACHE_ID:
      case GlobalConfig.NETCACHE_ID:
        System.out.println("[INFO][InSwitchCacheClient] Collecting method workload results");
        System.out.println("[INFO][InSwitchCacheClient] cacheHitCount: " + getSwitchAndServerLoad(0).get());
        // System.out.println(Arrays.toString(SwitchAndServerLoadList))
        // System.out.println(Arrays.toString(CachehitLoadList))
        
        // int[] perserverOps = InswitchCacheClient.getCachehitLoadList();
        // int[] perserverCachehits = InswitchCacheClient.getCachehitLoadList();
        // System.out.print("serverOps: ["+perserverOps[0]);
        // for (int i = 1; i < perserverOps.length; i++) {
        //   System.out.print(","+perserverOps[i]);
        // }
        // System.out.println("]");
        // System.out.print("Cachehits: ["+perserverOps[0]);
        // for (int i = 1; i < perserverOps.length; i++) {
        //   System.out.print(","+perserverOps[i]);
        // }
        // System.out.println("]");
        // printArray(SwitchAndServerLoadList)
        // printArray(CachehitLoadList)
        // dumpStatistics();
        break;
      case GlobalConfig.KEYDUMP_ID:
        System.out.println("[INFO][InSwitchCacheClient] Collecting keydump workload results");
        dumpHotKeys();
        dumpBottleneckPartitions();
        // pregenerateWorkload();
        break;
      case GlobalConfig.RECORDLOAD_ID:
        break;
      default:
        System.out
            .println(String.format("[ERROR][InSwitchCacheClient] invalid methodid: %d", GlobalConfig.getCurmethodId()));
        System.exit(-1);
        break;
    }
  }

  /**
   * Initialization Preparation Section.
   * - usageMessage(): print InswitchCache argument flags information
   * - checkRequiredProperties(): check inswitch cache flags vadility
   */

  public static void usageMessage() {
    System.out.println("");
    System.out.println("Required properties for InswitchCache:");
    System.out.println("  -pi: the index of physical client");
    // System.out.println("");
    // System.out.println("Optional properties for InswitchCache:");
    // System.out.println(" -df: the absolute file path of dumped keys for keydump
    // database");
  }

  /**
   * Prebenchmark Section.
   * - runSendPkt(): sync physical clients.
   * - runClientRulemapServer(): launch RulemapServer under dynamic workload for
   * non-first physcial client.
   * - runClientRulemapClient(): launch RulemapClient under dynamic workload for
   * physcial client 0.
   */

  public static boolean runSendPkt() {
    System.out.println("[INFO][InSwitchCacheClient] runSendPkt");
    if (GlobalConfig.getClientPhysicalNum() > 1) {
      // all physical clients other than 0
      if (GlobalConfig.getClientPhysicalIdx() != 0) {
        short sendPktSvrUdpPort = GlobalConfig.getClientSendPktServerPortStart();
        sendPktSvrUdpPort += GlobalConfig.getClientPhysicalIdx() - 1;
        int sendPktSvrUdpSock = SocketHelper.prepareUdpserver(false, sendPktSvrUdpPort, "client.sendpktserver",
            0, 0, GlobalConfig.UDP_DEFAULT_RCVBUFSIZE);

        System.out.println("[INFO][InSwitchCacheClient] client.sendpktserver ready");
        byte[] sendPktSvrBuf = new byte[256];

        UdpAddr client0Addr = new UdpAddr();
        SocketHelper.udprecvfrom(sendPktSvrUdpSock, sendPktSvrBuf, 256, client0Addr, "client.sendpktserver");
        boolean checkCondition = (ByteBuffer.wrap(sendPktSvrBuf)
            .getInt() == GlobalConfig.getClientSendPktServerPortStart() + GlobalConfig.getClientPhysicalIdx() - 1);
        if (!checkCondition) {
          System.err.println("[ERROR][InSwitchCacheClient] Errors in receiving packet in client.sendPacketServer");
          System.err.println("[ERROR][InSwitchCacheClient] sendPktSvrBuf: " + sendPktSvrBuf.length);
          System.err.println("[ERROR][InSwitchCacheClient] packet content: " + ByteBuffer.wrap(sendPktSvrBuf).getInt());
          int tmpPort = GlobalConfig.getClientSendPktServerPortStart() + GlobalConfig.getClientPhysicalIdx() - 1;
          System.err.println("[ERROR][InSwitchCacheClient] port calculation: " + tmpPort);
          System.exit(0);
        }
        SocketHelper.udpsendto(sendPktSvrUdpSock, sendPktSvrBuf, 256, client0Addr.getIpAddr(), client0Addr.getUdpPort(),
            "client.sendpktserver");
        SocketHelper.close(sendPktSvrUdpSock);
      } else {
        // physical client 0
        int sendPktCltUdpSock = SocketHelper.createUdpsock(true, "client.sendpktclient");
        byte[] sendPktCltBuf = new byte[256];
        try {
          // Get a list of physical client address
          ArrayList<InetAddress> otherClientSendPktSvrAddr = new ArrayList<>();
          for (int otherPhysicalClientIdx = 1; otherPhysicalClientIdx < GlobalConfig
              .getClientPhysicalNum(); otherPhysicalClientIdx++) {
            InetAddress otherPktCltAddr = InetAddress
                .getByName(GlobalConfig.getCertianClientIpForClient0(otherPhysicalClientIdx));
            otherClientSendPktSvrAddr.add(otherPktCltAddr);
          }
          System.out.println("[INFO][InSwitchCacheClient] client.sendpktclient ready");
          while (true) {
            // Send to every other physical clients
            for (int otherPhysicalClientIdx = 1; otherPhysicalClientIdx < GlobalConfig
                .getClientPhysicalNum(); otherPhysicalClientIdx++) {
              short otherPhysicalClientPort = GlobalConfig.getClientSendPktServerPortStart();
              otherPhysicalClientPort += (short) (otherPhysicalClientIdx) - 1;
              
              String otherPhyscialClientAddr = GlobalConfig.getCertianClientIpForClient0(otherPhysicalClientIdx);

              int tmpPort = GlobalConfig.getClientSendPktServerPortStart() + otherPhysicalClientIdx - 1;
              ByteBuffer buffer = ByteBuffer.allocate(Integer.BYTES);
              buffer.putInt(tmpPort);
              System.arraycopy(buffer.array(), 0, sendPktCltBuf, 0, Integer.BYTES);
              System.out.println("[INFO][InSwitchCacheClient]tmpPort" + tmpPort);
              SocketHelper.udpsendto(sendPktCltUdpSock, sendPktCltBuf, Integer.BYTES,
                  otherPhyscialClientAddr, otherPhysicalClientPort, "client.sendpktclient");
            }
            // Recevie from every other physical clients
            int isTimeout = -1;
            for (int otherPhysicalClientIdx = 1; otherPhysicalClientIdx < GlobalConfig
                .getClientPhysicalNum(); otherPhysicalClientIdx++) {
              short otherPhysicalClientPort = GlobalConfig.getClientSendPktServerPortStart();
              otherPhysicalClientPort += (short) (otherPhysicalClientIdx) - 1;
              UdpAddr otherPhysicalClientUdpAddr = new UdpAddr();
              otherPhysicalClientUdpAddr
                  .setIpAddr(GlobalConfig.getCertianClientIpForClient0(otherPhysicalClientIdx));
              otherPhysicalClientUdpAddr.setUdpPort(otherPhysicalClientPort);

              isTimeout = SocketHelper.udprecvfrom(sendPktCltUdpSock, sendPktCltBuf, 256, otherPhysicalClientUdpAddr,
                  "client.sendpktclient");
              if (isTimeout == -1) {
                break;
              }
            }

            if (isTimeout == -1) {
              continue;
            } else {
              break;
            }
          }
          SocketHelper.close(sendPktCltUdpSock);
        } catch (Exception e) {
          e.printStackTrace();
          System.err.println("[ERROR][InSwitchCacheClient] client.sendpktclient exception");
          SocketHelper.close(sendPktCltUdpSock);
          System.exit(0);
        }
      }
    }
    System.out
        .println("[INFO][InSwitchCacheClient] Finished physical client synchronization, start sending packets ...");
    return true;
  }

  private static void runClientUpstreamBackupReleaser() {
    MyUtils.myAssert(GlobalConfig.getCurmethodId() == GlobalConfig.FARREACH_ID);
    System.out.println("[INFO][InSwitchCacheClient] run client upstream backup releaser");

    // [TODO] Bug: clientUpstreamBackupReleaserPort is incorrectly set as 0 in GlobalConfig.
	//   We leave the test of the following code block after fixing the bug in GlobalConfig in the future.
    //   But note that not releasing upstream backups does NOT affect our main design (sysheng)
    /*short clientUpstreamBackupReleaserPort = GlobalConfig.getClientUpstreamBackupReleaserPort();
    int clientUpstreamBackupReleaserUdpSock = SocketHelper.prepareUdpserver(true,
        clientUpstreamBackupReleaserPort, "client.upstreamBackupReleaser", GlobalConfig.DEFAULT_SOCKET_TIMEOUT_SECS, 0,
        GlobalConfig.UDP_DEFAULT_RCVBUFSIZE);
    System.out.println("[INFO][InSwitchCacheClient] client.upstreamBackupReleaser ready");

    DynamicArray dynamicBuf = new DynamicArray(GlobalConfig.MAX_BUFFER_SIZE, GlobalConfig.MAX_LARGE_BUFFER_SIZE);
    while (true) {
      dynamicBuf.clear();
      int recvSize = SocketHelper.udprecvlargeudpfrag(GlobalConfig.getCurmethodId(),
          clientUpstreamBackupReleaserUdpSock,
          dynamicBuf, "client.upstreamBackupReleaser");
      if (recvSize != -1) {
        HashMap<Key, Integer> keySeqMap = OutbandPacketFormat.deserializeUpstreamBackupNotification(dynamicBuf.array(),
            dynamicBuf.size());
        // Release per-client outdated upstream backups
        for (int i = 0; i < GlobalConfig.getCurrentClientLogicalNum(); i++) {
          ArrayList<Key> outdatedKeys = new ArrayList<>();
          for (Map.Entry<Key, UpstreamBackup> entry : perclientUpstreamBackups.get(i).entrySet()) {
            boolean isOutdated = false;
            if (!keySeqMap.containsKey(entry.getKey())) { // key is not in switch
              isOutdated = true;
            } else { // key is still in switch
              int existingSeq = entry.getValue().getSeq();
              int snapshotSeq = keySeqMap.get(entry.getKey());
              if (existingSeq <= snapshotSeq) { // value has been snapshotted
                isOutdated = true;
              }
            }

            if (isOutdated) {
              outdatedKeys.add(entry.getKey());
            }
          } // each upstream backup

          for (int j = 0; j < outdatedKeys.size(); j++) {
            perclientUpstreamBackups.get(i).remove(outdatedKeys.get(j));
          } // each outdated upstream backup
        } // each logical client
      } // not timeout
    }*/
  }

  public static void dumpUpstreamBackups() {
    MyUtils.myAssert(GlobalConfig.getCurmethodId() == GlobalConfig.FARREACH_ID);

    // Aggregate per-client upstream backups
    HashMap<Key, UpstreamBackup> aggKeyBackupMap = new HashMap<>();
    for (int i = 0; i < GlobalConfig.getCurrentClientLogicalNum(); i++) {
      for (Map.Entry<Key, UpstreamBackup> entry : perclientUpstreamBackups.get(i).entrySet()) {
        if (!aggKeyBackupMap.containsKey(entry.getKey())) { // key is not aggregated
          aggKeyBackupMap.put(entry.getKey(), entry.getValue());
        } else { // key is already aggreagated
          int curSeq = entry.getValue().getSeq();
          int aggSeq = aggKeyBackupMap.get(entry.getKey()).getSeq();
          if (curSeq > aggSeq) { // current backup is more latest
            aggKeyBackupMap.put(entry.getKey(), entry.getValue());
          }
        }
      } // each upstream backup
    } // each logical client
    System.out
        .println(String.format("[INFO][InSwitchCacheClient] # of aggregated backups: %d", aggKeyBackupMap.size()));

    DynamicArray dynamicBuf = new DynamicArray(GlobalConfig.MAX_BUFFER_SIZE, GlobalConfig.MAX_LARGE_BUFFER_SIZE);
    dynamicBuf.clear();
    OutbandPacketFormat.dynamicSerializeUpstreamBackups(dynamicBuf, aggKeyBackupMap);

    String dirpath = IOHelper.getUpstreamBackupDirpath();
    File dir = new File(dirpath);
    if (!dir.exists()) {
      dir.mkdir();
    }

    String filepath = "";
    int clientPhysicalIdx = GlobalConfig.getClientPhysicalIdx();
    if (GlobalConfig.isStaticPattern()) {
      int rotateScale = GlobalConfig.getMaxServerTotalLogicalNum();
      int bottleneckIdx = GlobalConfig.getBottleneckServeridxForRotation();
      if (GlobalConfig.isFirstRotation()) {
        filepath = IOHelper.getStaticUpstreamBackupFilepath(rotateScale, bottleneckIdx, bottleneckIdx,
            clientPhysicalIdx);
      } else {
        int rotateIdx = GlobalConfig.getRotatedServeridxForRotation();
        filepath = IOHelper.getStaticUpstreamBackupFilepath(rotateScale, bottleneckIdx, rotateIdx, clientPhysicalIdx);
      }
    } else if (GlobalConfig.isDynamicPattern()) {
      filepath = IOHelper.getDynamicUpstreamBackupFilepath(clientPhysicalIdx);
    } else {
      System.out.println("[ERROR][InSwitchCacheClient] invalid pattern");
      System.exit(-1);
    }

    try {
      File file = new File(filepath);
      if (file.exists()) {
        System.out.println(
            String.format("[WARN][InSwitchCacheClient] file %s exists -> delete and write a new one", filepath));
        file.delete();
      }

      System.out.println(String.format("[INFO][InSwitchCacheClient] dump upstream backups into %s", filepath));
      FileOutputStream fos = new FileOutputStream(filepath);
      fos.write(dynamicBuf.array());
      fos.close();
    } catch (Exception e) {
      System.out.println(e.getMessage());
      e.printStackTrace();
      System.exit(-1);
    }
  }

  private static void runClientRulemapServer() {
    // NOTE: we have judged the condition in preBenchmark() before
    // if (GlobalConfig.getWorkloadMode() == 0 ||
    // GlobalConfig.getClientPhysicalIdx() == 0) {
    // // static workload or first physical client
    // return;
    // }
    System.out.println("[INFO][InSwitchCacheClient] run client rulemap server");

    short clientRulemapSvrUdpPort = GlobalConfig.getClientRulemapServerPortStart();
    clientRulemapSvrUdpPort += (short) (GlobalConfig.getClientPhysicalIdx()) - 1;
    int clientRuleMapSvrUdpSock = SocketHelper.prepareUdpserver(false,
        clientRulemapSvrUdpPort, "client.rulemapserver", GlobalConfig.DEFAULT_SOCKET_TIMEOUT_SECS, 0,
        GlobalConfig.UDP_DEFAULT_RCVBUFSIZE);
    // Note: Only use it in Bmv2 dynamic pattern !!!!!!!!
    int fakeCachepopClientRecvSock = SocketHelper.createUdpsock(true, "client.fakeCachepopClient");
    
    System.out.println("[INFO][InSwitchCacheClient] client.rulemapserver ready");

    int dynamicPeriodIdx = 0;
    byte[] rulemapSvrBuf = new byte[256];
    UdpAddr client0Addr = new UdpAddr();
    boolean recvFailed = false;
    while (true) {
      int recvSize = SocketHelper.udprecvfrom(clientRuleMapSvrUdpSock, rulemapSvrBuf, 256,
          client0Addr, "client.rulemapserver");
      if (recvSize != -1) {
        byte[] rulemapSvrTmpBuf = new byte[Integer.BYTES];
        System.arraycopy(rulemapSvrBuf, 0, rulemapSvrTmpBuf, 0, Integer.BYTES);
        if (ByteBuffer.wrap(rulemapSvrTmpBuf).getInt() == dynamicPeriodIdx) {
          dynamicRulemap.nextperiod();
          currentDynamicPeriod++;
          System.out.println("[INFO][InSwitchCacheClient]" + GlobalConfig.getDynamicPeriodNum() + " Switch dynamic rulemap from period " + dynamicPeriodIdx
              + ", to " + (dynamicPeriodIdx + 1));
          if (dynamicPeriodIdx == (GlobalConfig.getDynamicPeriodNum() - 2)) {
            break;
          }
          dynamicPeriodIdx++;
          System.out.println("send fake cachepop");
          SocketHelper.udpsendto(fakeCachepopClientRecvSock, new byte[] {(byte)(dynamicPeriodIdx >>> 24),
              (byte)(dynamicPeriodIdx >>> 16),
              (byte)(dynamicPeriodIdx >>> 8),
              (byte)dynamicPeriodIdx
              }, 
              Integer.BYTES, 
              "192.168.122.229",
              GlobalConfig.getFakeCachepopClientRecvport(),
              "client.sendpktserver");
        } else {
          System.out.println("[DEBUG][InSwitchCacheClient] rulemap failed because of wrong buffer number: " +
              ByteBuffer.wrap(rulemapSvrTmpBuf).getInt());
          recvFailed = true;
        }
      } else {
        System.out.println("[DEBUG][InSwitchCacheClient] recvfrom failed");
        recvFailed = true;
      }

      if (recvFailed) {
        System.err.println("[ERROR][InSwitchCacheClient] Failed to receive from rulemap clients.");
        System.exit(-1);
      }
    }
    System.err.println("[INFO][InSwitchCacheClient] Finish rulemap server.");
    SocketHelper.close(clientRuleMapSvrUdpSock);
  }

  public static void runClientRulemapClient(long maxExecutionTime) throws InterruptedException {
    if (!GlobalConfig.isNonStableDynamicPattern() || GlobalConfig.getClientPhysicalIdx() != 0) {
      Thread.sleep(maxExecutionTime * 1000);
    } else {
      // set max execution time
      long startTime = System.currentTimeMillis();
      long elapsedTime = 0L;
      int fakeCachepopClientRecvSock = SocketHelper.createUdpsock(true, "client.fakeCachepopClient");
      System.out.println("[INFO][InSwitchCacheClient] run client rulemap client");
      int clientRulemapCltUdpSock = SocketHelper.createUdpsock(true, "client.rulemapclient");
      byte[] rulemapClientBuf, periodIdxBytes;

      for (int periodIdx = 0; elapsedTime < maxExecutionTime * 1000
          && periodIdx < GlobalConfig.getDynamicPeriodNum(); periodIdx++) {
        long remainingTime = (maxExecutionTime * 1000 - elapsedTime);
        if (remainingTime < GlobalConfig.getDynamicPeriodInterval() * 1000) {
          Thread.sleep(remainingTime);
          elapsedTime = maxExecutionTime * 1000;
          break;
        }

        Thread.sleep(GlobalConfig.getDynamicPeriodInterval() * 1000);
        if (periodIdx == GlobalConfig.getDynamicPeriodNum() - 1) {
          elapsedTime = (new Date()).getTime() - startTime;
          break;
        }
        if (GlobalConfig.getClientPhysicalNum() > 1) {
          rulemapClientBuf = new byte[256];
          for (int otherPhysicalClientIdx = 1; otherPhysicalClientIdx < GlobalConfig
              .getClientPhysicalNum(); otherPhysicalClientIdx++) {
            periodIdxBytes = ByteBuffer.allocate(Integer.BYTES).putInt(
                periodIdx).array();
            System.arraycopy(periodIdxBytes, 0, rulemapClientBuf, 0, Integer.BYTES);

            String destAddr = GlobalConfig.getCertianClientIpForClient0(otherPhysicalClientIdx);
            short destPort = GlobalConfig.getClientRulemapServerPortStart();
            destPort += (short) (otherPhysicalClientIdx) - 1;
            SocketHelper.udpsendto(clientRulemapCltUdpSock, rulemapClientBuf, Integer.BYTES, destAddr, destPort,
                "client.rulemapclient");
          } // end of other physical client loop
        }
        System.out.println("[INFO][InSwitchCacheClient] Switch dynamic rulemap from period " + periodIdx + " to "
            + (periodIdx + 1));
        currentDynamicPeriod++;
        int periodIdxplus = periodIdx + 1;
        System.out.println("send fake cachepop");
        SocketHelper.udpsendto(fakeCachepopClientRecvSock, new byte[] {(byte)(periodIdxplus >>> 24),
            (byte)(periodIdxplus >>> 16),
            (byte)(periodIdxplus >>> 8),
            (byte)periodIdxplus
            }, 
            Integer.BYTES, 
            "192.168.122.229",
            GlobalConfig.getFakeCachepopClientRecvport(),
            "client.sendpktserver");
        dynamicRulemap.nextperiod();
        elapsedTime = (new Date()).getTime() - startTime;
      } // end of periodIdx loop

      long remainingTime = (maxExecutionTime * 1000 - elapsedTime);
      if (remainingTime > 0) {
        Thread.sleep(remainingTime);
      }
      SocketHelper.close(clientRulemapCltUdpSock);
    }
  }

  /**
   * Postbenchmark Section.
   * - dumpHotKeys(): dump hot keys for keydump
   * - dumpBottleneckPartitions(): dump bottleneck serveridx for keydump
   * - // dumpStatistics(): dump debug information for each method
   * - // pregenerateWorkload(): dump pre-generated workload into disk (dumped by
   * KeydumpClient now)
   */

  public static void dumpHotKeys() {
    System.out.println("[INFO][InswitchCacheClient] Collecting dump statistics from all clients");

    // Aggregate per-logicalclient keyFrequencyRecords
    HashMap<String, Integer> totalKeyFrequencyRecords = new HashMap<String, Integer>();
    for (int i = 0; i < keyFrequencyRecordsList.size(); i++) {
      for (Map.Entry<String, Integer> entry : keyFrequencyRecordsList.get(0).entrySet()) {
        totalKeyFrequencyRecords.put(entry.getKey(),
            totalKeyFrequencyRecords.getOrDefault(entry.getKey(), 0) + entry.getValue());
        totalFrequency += entry.getValue();
      }
      keyFrequencyRecordsList.remove(0);
    }

    // Find hot keys by sorting
    ArrayList<Map.Entry<String, Integer>> sortedKeyFrequencyRecords = new ArrayList<Map.Entry<String, Integer>>(
        totalKeyFrequencyRecords.entrySet());
    Collections.sort(sortedKeyFrequencyRecords, new Comparator<Map.Entry<String, Integer>>() {
      public int compare(Map.Entry<String, Integer> a, Map.Entry<String, Integer> b) {
        return b.getValue().compareTo(a.getValue()); // sort by descending order
      }
    });
    int maxnum = (sortedKeyFrequencyRecords.size() > GlobalConfig.getmaxLoadBatchSize()) ?  GlobalConfig.getmaxLoadBatchSize() : sortedKeyFrequencyRecords.size();

    // Dump hotest keys
    int topFrequency = 0;
    String hotestFilename = IOHelper.getHotkeysDumpFilepath(GlobalConfig.getWorkloadName());
    System.out.println("[INFO][InSwitchCacheClient] Workload dump file of hotest keys: " + hotestFilename);
    File dumpKeyFileHotest = new File(hotestFilename);
    try {
      FileWriter fw = new FileWriter(dumpKeyFileHotest.getAbsolutePath());
      BufferedWriter dumpTopKeysWriter = new BufferedWriter(fw);

      for (int recordIdx = 0; recordIdx < maxnum; recordIdx++) {
        Map.Entry<String, Integer> toprecord = sortedKeyFrequencyRecords.get(recordIdx);
        dumpTopKeysWriter.append(String.format("WARMUP %s %d\n", toprecord.getKey(), toprecord.getValue()));
        topFrequency += toprecord.getValue();
      }
      dumpTopKeysWriter.close();
      float hotratio = (float) topFrequency / totalFrequency;
      System.out.println(String.format(
          "[INFO][InSwitchCacheClient] total frequency: %d, hot frequency: %d, hot ratio: %f", totalFrequency,
          topFrequency, hotratio));
      fw.close();
    } catch (IOException e) {
      e.printStackTrace();
    }

    // Dump nearhot keys (the hot keys after the hotest keys)
    int nearhotFrequency = 0;
    String nearhotFilename = IOHelper.getNearhotkeysDumpFilepath(GlobalConfig.getWorkloadName());
    System.out.println("[INFO][InSwitchCacheClient] Workload dump file of nearhot keys: " + nearhotFilename);
    File dumpKeyFilenearhot = new File(nearhotFilename);
    try {
      FileWriter fw = new FileWriter(dumpKeyFilenearhot.getAbsolutePath());
      BufferedWriter dumpNearhotKeysWriter = new BufferedWriter(fw);

      int nearhotCnt = 0;
      for (int recordIdx = maxnum; recordIdx < sortedKeyFrequencyRecords.size(); recordIdx++) {
        Map.Entry<String, Integer> neartoprecord = sortedKeyFrequencyRecords.get(recordIdx);
        dumpNearhotKeysWriter.append(String.format("%s %d\n", neartoprecord.getKey(), neartoprecord.getValue()));
        nearhotFrequency += neartoprecord.getValue();
        nearhotCnt += 1;
        if (nearhotCnt >= maxnum) {
          break;
        }
      }
      float nearhotratio = (float) nearhotFrequency / totalFrequency;
      System.out.println(String.format(
          "[INFO][InSwitchCacheClient] total frequency: %d, near hot frequency: %d, hot ratio: %f", totalFrequency,
          nearhotFrequency, nearhotratio));
      dumpNearhotKeysWriter.close();
      fw.close();
    } catch (IOException e) {
      e.printStackTrace();
    }

    // Dump coldest keys
    String coldestFilename = IOHelper.getColdkeysDumpFilepath(GlobalConfig.getWorkloadName());
    System.out.println("[INFO][InSwitchCacheClient] Workload dump file of coldest keys: " + coldestFilename);
    File dumpKeyFileColdest = new File(coldestFilename);
    try {
      FileWriter fw = new FileWriter(dumpKeyFileColdest.getAbsolutePath());
      BufferedWriter dumpColdKeysWriter = new BufferedWriter(fw);

      for (int recordIdx = sortedKeyFrequencyRecords.size() - 1; recordIdx >= (sortedKeyFrequencyRecords.size()
          - maxnum); recordIdx--) {
        Map.Entry<String, Integer> coldrecord = sortedKeyFrequencyRecords.get(recordIdx);
        dumpColdKeysWriter.append(String.format("%s %d\n", coldrecord.getKey(), coldrecord.getValue()));
      }
      dumpColdKeysWriter.close();
      fw.close();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public static void dumpBottleneckPartitions() {
    int totalLoad16 = 0;
    int maxServerLoad16 = 0;
    int bottleneckServerIdx16 = 0;
    for (int i = 0; i < 16; i++) {
      int tmpServerLoad = perserverLoadList16[i].get();
      totalLoad16 += tmpServerLoad;
      if (tmpServerLoad > maxServerLoad16) {
        maxServerLoad16 = tmpServerLoad;
        bottleneckServerIdx16 = i;
      }
    }
    System.out.println(String.format("[INFO][KEYDUMP] under 16 servers, bottleneck idx %d with load %d/%d",
        bottleneckServerIdx16, maxServerLoad16, totalLoad16));

    int totalLoad32 = 0;
    int maxServerLoad32 = 0;
    int bottleneckServerIdx32 = 0;
    for (int i = 0; i < 32; i++) {
      int tmpServerLoad = perserverLoadList32[i].get();
      totalLoad32 += tmpServerLoad;
      if (tmpServerLoad > maxServerLoad32) {
        maxServerLoad32 = tmpServerLoad;
        bottleneckServerIdx32 = i;
      }
    }
    System.out.println(String.format("[INFO][KEYDUMP] under 32 servers, bottleneck idx %d with load %d/%d",
        bottleneckServerIdx32, maxServerLoad32, totalLoad32));

    int totalLoad64 = 0;
    int maxServerLoad64 = 0;
    int bottleneckServerIdx64 = 0;
    for (int i = 0; i < 64; i++) {
      int tmpServerLoad = perserverLoadList64[i].get();
      totalLoad64 += tmpServerLoad;
      if (tmpServerLoad > maxServerLoad64) {
        maxServerLoad64 = tmpServerLoad;
        bottleneckServerIdx64 = i;
      }
    }
    System.out.println(String.format("[INFO][KEYDUMP] under 64 servers, bottleneck idx %d with load %d/%d",
        bottleneckServerIdx64, maxServerLoad64, totalLoad64));

    int totalLoad128 = 0;
    int maxServerLoad128 = 0;
    int bottleneckServerIdx128 = 0;
    for (int i = 0; i < 128; i++) {
      int tmpServerLoad = perserverLoadList128[i].get();
      totalLoad128 += tmpServerLoad;
      if (tmpServerLoad > maxServerLoad128) {
        maxServerLoad128 = tmpServerLoad;
        bottleneckServerIdx128 = i;
      }
    }
    System.out.println(String.format("[INFO][KEYDUMP] under 128 servers, bottleneck idx %d with load %d/%d",
        bottleneckServerIdx128, maxServerLoad128, totalLoad128));
  }

  /**
   * Utils.
   */

  public static void updateLoadStatistics(int clientLogicalIdx, DbUdpNativeResult result) {
    int nodeIdxForEval = (int) result.getNodeIdxForEval();
    int serveridx = result.getKey().getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
        GlobalConfig.getMaxServerTotalLogicalNum());
    if (nodeIdxForEval == -1) {
      // per-server load of cache hits
      cachehitLoadList[serveridx].getAndIncrement();
    }
    // per-switch load of cahce hits and per-server load of cache misses
    switchAndServerLoadList[nodeIdxForEval + 1].getAndIncrement();

    // update upstream backups for FarReach
    if (GlobalConfig.getCurmethodId() == GlobalConfig.FARREACH_ID && nodeIdxForEval == -1) {
      UpstreamBackup existingBackup = perclientUpstreamBackups.get(clientLogicalIdx).get(result.getKey());
      if (existingBackup == null || existingBackup.getSeq() < result.getSeq()) {
        UpstreamBackup newBackup = new UpstreamBackup(result.getValue(), result.getSeq(), result.isStat());
        perclientUpstreamBackups.get(clientLogicalIdx).put(result.getKey(), newBackup);
      }
    }
  }

  public static int[] getSwitchAndServerLoadList() {
    int[] tmpSwitchAndServerLoadList = new int[switchAndServerLoadList.length];
    for (int i = 0; i < switchAndServerLoadList.length; i++) {
      tmpSwitchAndServerLoadList[i] = switchAndServerLoadList[i].get();
    }
    return tmpSwitchAndServerLoadList;
  }

  public static int[] getCachehitLoadList() {
    int[] tmpCachehitLoadList = new int[cachehitLoadList.length];
    for (int i = 0; i < cachehitLoadList.length; i++) {
      tmpCachehitLoadList[i] = cachehitLoadList[i].get();
    }
    return tmpCachehitLoadList;
  }

  /**
   * Getter and setter.
   */

  public static HashMap<String, Integer> getKeyFrequencyRecords(int logicalClientIdx) {
    return keyFrequencyRecordsList.get(logicalClientIdx);
  }

  // public static HashMap<String, String> getPeriodKeyKeyRecordsList(int
  // periodIdx) {
  // return syncPeriodKeyKeyRecordsList.get(periodIdx);
  // }

  // public static HashMap<String, Integer> getPeriodKeyKeyFreqList(int periodIdx)
  // {
  // return syncPeriodKeyKeyFreqList.get(periodIdx);
  // }

  public static int getTotalFrequency() {
    return totalFrequency;
  }

  // public static AtomicInteger getFinishedThreadCnt() {
  // return finishedThreadCnt;
  // }

  // public static void incrFinishedThreadCnt() {
  // finishedThreadCnt.getAndIncrement();
  // }

  public static AtomicInteger getSwitchAndServerLoad(int index) {
    return switchAndServerLoadList[index];
  }

  public static void increPerserverLoad16(int index) {
    perserverLoadList16[index].getAndIncrement();
  }

  public static void increPerserverLoad32(int index) {
    perserverLoadList32[index].getAndIncrement();
  }

  public static void increPerserverLoad64(int index) {
    perserverLoadList64[index].getAndIncrement();
  }

  public static void increPerserverLoad128(int index) {
    perserverLoadList128[index].getAndIncrement();
  }

  // public static void increSwitchAndServerLoad(int index) {
  // switchAndServerLoadList[index].getAndIncrement();
  // }

  // public static boolean getInitializedStatus() {
  // return initialized;
  // }

  public static DynamicRulemap getDynamicRulemap() {
    return dynamicRulemap;
  }

  public static int getCurrentDynamicPeriod() {
    return currentDynamicPeriod;
  }

  // public static ArrayList<InmemoryReq> getInmemoryReqList(int
  // localLogicalClientidx) {
  // return perclientInmemoryReqList[localLogicalClientidx];
  // }

  public static boolean isStop() {
    return isStop;
  }

  public static void stop() {
    isStop = true;
  }
}
