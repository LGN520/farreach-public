package com.inswitchcache.core;

import java.awt.image.AreaAveragingScaleFilter;
import java.io.File;
import java.io.IOException;
import java.util.*;

import org.ini4j.Ini;

/**
 * Class for parsing configuration and dynamic rulemaps.
 * NOTE: shared by multiple threads
 */
public final class GlobalConfig {
  private static boolean isInit = false;

  // constants for database name
  /**
   * Indicate the InswitchCache-related database names.
   */
  public static final int INVALID_ID = 0;
  public static final int FARREACH_ID = 1;
  public static final int NOCACHE_ID = 2;
  public static final int NETCACHE_ID = 3;
  public static final int DISTFARREACH_ID = 4;
  public static final int DISTNOCACHE_ID = 5;
  public static final int DISTCACHE_ID = 6;
  // NOTE: the following methodids are NOT used by packet format and socket helper
  public static final int KEYDUMP_ID = 7;
  public static final int RECORDLOAD_ID = 8;
  public static final ArrayList<String> INSWITCHCACHE_DBNAMES = new ArrayList<String>(
      Arrays.asList("invalid", "farreach", "nocache", "netcache", "distfarreach", "distnocache", "distcache",
          "keydump", "recordload"));

  private static int curmethodId = 0;

  // constants for buffer
  // use MAX_BUFFER_SIZE and MAX_LARGE_BUFFER_SIZE for DynamicArray
  public static final int MAX_BUFFER_SIZE = 40960; // 40KB
  public static final int MAX_LARGE_BUFFER_SIZE = 8388608; // 8MB
  public static final int MAX_KERNEL_SIZE = 1024;

  // constants for socket
  public static final int DEFAULT_SOCKET_TIMEOUT_SECS = 30;
  public static final int CLIENT_SOCKET_TIMEOUT_SECS = 30;
  public static final int CLIENT_SCAN_SOCKET_TIMEOUT_SECS = 30;
  public static final int UDP_DEFAULT_RCVBUFSIZE = 212992; // 208KB used in linux by default
  public static final int UDP_LARGE_RCVBUFSIZE = 8388608; // 8MB (used by socket to receive large data if with limited client threads)

  // config file
  public static final String CONFIG_FILENAME = "config.ini";

  // from command line
  private static int clientPhysicalIdx = -1;

  private static boolean singleRotation = false;

  // global section
  private static String workloadName = "";
  private static int workloadMode = 0;
  private static int dynamicPeriodNum = 0;
  private static int dynamicPeriodInterval = 0;
  private static String dynamicRulePrefix = "";
  private static int clientPhysicalNum = 0;
  private static int clientTotalLogicalNum = 0;

  private static int serverPhysicalNum = 0;

  private static int serverTotalLogicalNum = 0; // # of truly-running servers
  private static int serverTotalLogicalNumForRotation = 0; // server scale under server rotation
  private static int serverPerServerLogicalNum = 0;
  private static int maxServerTotalLogicalNum = 0;
  private static int bottleneckServeridxForRotation = 0;

  private static int maxLoadBatchSize = 0;
  // client common information
  private static short clientRotationDataServerPort = 0;
  private static short clientSendPktServerPortStart = 0;
  private static short clientRulemapServerPortStart = 0;
  private static short clientWorkerPortStart = 0;
  private static short clientUpstreamBackupReleaserPort = 0;

  // for each physical client
  private static ArrayList<Integer> clientLogicalNums = new ArrayList<Integer>();
  private static ArrayList<String> clientIpsForClient0 = new ArrayList<String>();

  // server
  private static short serverWorkerPortStart = 0;

  // for each physical server
  private static ArrayList<String> serverIps = new ArrayList<>();
  private static ArrayList<Integer>[] serverLogicalIdxesList = null;

  // switch
  private static int switchPartitionCount = 0;

  private static boolean latencyTest = false;

  private GlobalConfig() {
    // not used
  }

  // NOTE: ONLY invoke once in main class
  public static void init(String dbname, int tmpClientPhysicalIdx, boolean tmpSingleRotation) {
    if (isInit) {
      System.out.println("[WARN][GlobalConfig] invoke ini more than once!");
      return;
    }
    isInit = true;

    for (int i = INSWITCHCACHE_DBNAMES.size() - 1; i >= 0; i--) {
      if (dbname.toLowerCase().contains(INSWITCHCACHE_DBNAMES.get(i))) {
        curmethodId = i;
        break;
      }
    }
    if (curmethodId == INVALID_ID) {
      System.out.println(String.format("[WARN][GlobalConfig] %s is not an inswitchcache database!", dbname));
      return;
    }

    String database = getDatabase();
    System.out.println(String.format("[INFO][GlobalConfig] current database: %s", database));

    clientPhysicalIdx = tmpClientPhysicalIdx;
    singleRotation = tmpSingleRotation;

    //String configFilePath = "";
    //if (curmethodId == RECORDLOAD_ID) {
    //  configFilePath = "../../" + INSWITCHCACHE_DBNAMES.get(NOCACHE_ID) + "/" + GlobalConfig.CONFIG_FILENAME;
    //} else {
    //  configFilePath = "../../" + database + "/" + GlobalConfig.CONFIG_FILENAME;
    //}
    String configFilePath = "../../" + database + "/" + GlobalConfig.CONFIG_FILENAME;
    File configFile = new File(configFilePath);
    if (!configFile.exists()) {
      System.err.println("[ERROR][GlobalConfig] No such config file: " + configFilePath);
      System.exit(0);
    } else {
      System.out.println("[INFO][GlobalConfig] Load config from file: " + configFilePath);
    }

    Ini ini = new Ini();
    try {
      ini.load(configFile);
    } catch (IOException e) {
      System.err.println("[ERROR][GlobalConfig] Failed to load config file");
      e.printStackTrace();
      e.printStackTrace(System.out);
      System.exit(0);
    }

    // global section
    GlobalConfig.workloadName = ini.get("global", "workload_name");
    GlobalConfig.workloadMode = Integer.parseInt(ini.get("global", "workload_mode"));
    try{
    GlobalConfig.maxLoadBatchSize = Integer.parseInt(ini.get("global", "max_load_batch_size"));
    }catch(Exception e){
      GlobalConfig.maxLoadBatchSize = 10000;
    }

    GlobalConfig.dynamicPeriodNum = Integer.parseInt(ini.get("global", "dynamic_periodnum"));
    GlobalConfig.dynamicPeriodInterval = Integer.parseInt(ini.get("global", "dynamic_periodinterval"));
    GlobalConfig.dynamicRulePrefix = ini.get("global", "dynamic_ruleprefix");
    GlobalConfig.clientPhysicalNum = Integer.parseInt(ini.get("global", "client_physical_num"));
    GlobalConfig.clientTotalLogicalNum = Integer.parseInt(ini.get("global", "client_total_logical_num"));
    GlobalConfig.serverPhysicalNum = Integer.parseInt(ini.get("global", "server_physical_num"));
    GlobalConfig.serverTotalLogicalNum = Integer.parseInt(ini.get("global", "server_total_logical_num"));
    GlobalConfig.serverTotalLogicalNumForRotation = Integer.parseInt(ini.get("global", "server_total_logical_num_for_rotation"));
    GlobalConfig.serverPerServerLogicalNum = GlobalConfig.serverTotalLogicalNum / GlobalConfig.serverPhysicalNum;
    if (isStaticPattern()) {
      maxServerTotalLogicalNum = serverTotalLogicalNumForRotation;
    } else { // keydump and recordload must enter this branch
      maxServerTotalLogicalNum = serverTotalLogicalNum;
    }
    GlobalConfig.bottleneckServeridxForRotation = Integer.parseInt(ini.get("global", "bottleneck_serveridx_for_rotation"));
    MyUtils.myAssert(bottleneckServeridxForRotation >= 0 && bottleneckServeridxForRotation < serverTotalLogicalNumForRotation);

    // client common infomration
    GlobalConfig.clientRotationDataServerPort = Short.parseShort(ini.get("client", "client_rotationdataserver_port"));
    GlobalConfig.clientSendPktServerPortStart = Short.parseShort(ini.get("client", "client_sendpktserver_port_start"));
    GlobalConfig.clientRulemapServerPortStart = Short.parseShort(ini.get("client", "client_rulemapserver_port_start"));
    GlobalConfig.clientWorkerPortStart = Short.parseShort(ini.get("client", "client_worker_port_start"));
    // [TODO] Bug: here we should check whether curmethodId == FARREACH_ID to read client_upstreambackupreleaser_port. (sysheng)
    /*if (GlobalConfig.workloadName == "farreach") {
      GlobalConfig.clientUpstreamBackupReleaserPort = Short
          .parseShort(ini.get("client", "client_upstreambackupreleaser_port"));
    }*/

    // for each physical client
    for (int i = 0; i < GlobalConfig.clientPhysicalNum; i++) {
      int tmpClientLogicalNum = Integer.parseInt(ini.get(String.format("client%d", i), "client_logical_num"));
      GlobalConfig.clientLogicalNums.add(tmpClientLogicalNum);

      String tmpClientIpForClient0 = ini.get(String.format("client%d", i), "client_ip_for_client0");
      GlobalConfig.clientIpsForClient0.add(tmpClientIpForClient0);
    }

    // server
    GlobalConfig.serverWorkerPortStart = Short.parseShort(ini.get("server", "server_worker_port_start"));

    // for each physical server
    serverLogicalIdxesList = new ArrayList[GlobalConfig.serverPhysicalNum];
    for (int i = 0; i < GlobalConfig.serverPhysicalNum; i++) {
      serverIps.add(ini.get(String.format("server%d", i), "server_ip"));
      serverLogicalIdxesList[i] = GlobalConfig.parseServerLogicalIdxes(ini.get(String.format("server%d", i), "server_logical_idxes"));
    }
    if (isStaticPattern()) {
      // MyUtils.myAssert(serverLogicalIdxesList[0].size() == 1 && serverLogicalIdxesList[0].get(0) == bottleneckServeridxForRotation);
      if (serverPhysicalNum > 1) {
        for (int i = 1; i < serverPhysicalNum; i++) {
          // MyUtils.myAssert(serverLogicalIdxesList[i].size() == 1 && serverLogicalIdxesList[i].get(0) != bottleneckServeridxForRotation);
        }
      }
    }

    // switch
    GlobalConfig.switchPartitionCount = Integer.parseInt(ini.get("switch", "switch_partition_count"));

    // Dump config information
    System.out.println(String.format("[INFO][GlobalConfig] client physical num: %d, client total logical num: %d",
        clientPhysicalNum, clientTotalLogicalNum));
    for (int i = 0; i < clientPhysicalNum; i++) {
      System.out.println(String.format("[INFO][GlobalConfig] client[%d] logical num: %d", i, clientLogicalNums.get(i)));
    }
    System.out.println(String.format("[INFO][GlobalConfig] server physical num: %d, max server total logical num: %d",
        serverPhysicalNum, maxServerTotalLogicalNum));
    for (int i = 0; i < serverPhysicalNum; i++) {
      System.out.println(String.format("[INFO][GlobalConfig] server[%d] logical indexes: %s",
          i, parseServerLogicalIdxes(ini.get(String.format("server%d", i), "server_logical_idxes"))));
    }

    // Verify and overwrite
    if (curmethodId == KEYDUMP_ID || curmethodId == RECORDLOAD_ID) {
      // NOTE: we generate hot keys in a single physical client to avoid data aggregation
      // NOTE: we load record into servers in a single physical client to avoid repeat data
      if (clientPhysicalIdx != 0) {
        System.err.println("[ERROR][GlobalConfig] Invalid client physical index of keydump/recordload job!");
        System.exit(0);
      }
      clientPhysicalNum = 1;
      clientTotalLogicalNum = clientLogicalNums.get(0);

      //if (curmethodId == RECORDLOAD_ID && workloadMode == 0) {
      //  MyUtils.myAssert(serverTotalLogicalNum == serverTotalLogicalNumForRotation);
      //}
      if (curmethodId == RECORDLOAD_ID) {
        MyUtils.myAssert(serverTotalLogicalNum == 1);
      }
    }
    if (!isCurmethodForTransaction()) { // keydump and recordload
      workloadMode = 0;
    }
    if (singleRotation == true) { // must run single rotation under static pattern
      MyUtils.myAssert(workloadMode == 0);
    }
  }

  private static ArrayList<Integer> parseServerLogicalIdxes(String datastring) {
    int fromIndex = 0;
    ArrayList<Integer> result = new ArrayList<Integer>();
    while (true) {
      int endIndex = datastring.indexOf(':', fromIndex);
      if (endIndex == -1) {
        endIndex = datastring.length();
      }

      if (endIndex <= fromIndex) {
        System.err.println("[ERROR][GlobalConfig] Errors in parsing server logical indexes");
        System.exit(-1);
      }
      String idxstr = datastring.substring(fromIndex, endIndex);
      int tmpidx = Integer.parseInt(idxstr);
      result.add(tmpidx);

      fromIndex = endIndex + 1;
      if (endIndex >= datastring.length()) {
        break;
      }
    }
    return result;
  }

  /**
   * Condition judgement.
   */

  // isInswitchCacheDatabase(String dbname): return whether dbname belongs to inswitchcache databases
  public static boolean isInswitchCacheDatabase(String dbname) {
    boolean isInswitchCache = false;
    for (int i = 0; i < GlobalConfig.INSWITCHCACHE_DBNAMES.size(); i++) {
      if (dbname.toLowerCase().contains(GlobalConfig.INSWITCHCACHE_DBNAMES.get(i))) {
        isInswitchCache = true;
        break;
      }
    }
    return isInswitchCache;
  }

  // isInswitchCacheMethod(): return whether current method belongs to inswitchcache methods
  public static boolean isInswitchCacheMethod() {
    return curmethodId != INVALID_ID;
  }

  public static boolean checkServerLogicalIdx(int serveridx) {
    MyUtils.myAssert(isInswitchCacheMethod());
    for (int i = 0; i < serverLogicalIdxesList.length; i++) {
      for (int j = 0; j < serverLogicalIdxesList[i].size(); j++) {
        if (serveridx == serverLogicalIdxesList[i].get(j)) {
          return true;
        }
      }
    }
    return false;
  }

  public static boolean isCurmethodForTransaction() {
    // NOTE: workload mode only affects methods during transaction phase,
    //     while keydump is in preparation phase and recordload is in loading phase.
    MyUtils.myAssert(isInswitchCacheMethod());
    if (curmethodId == KEYDUMP_ID || curmethodId == RECORDLOAD_ID) {
      return false;
    } else {
      return true;
    }
  }

  public static boolean isFirstRotation() {
    // NOTE: we only launch the bottleneck server in the first physical server at the first rotation
    MyUtils.myAssert(isInswitchCacheMethod());
    if (serverPhysicalNum == 1) {
      // MyUtils.myAssert(isStaticPattern() && serverLogicalIdxesList[0].size() == 1);
      return true;
    }
    return false;
  }

  // static pattern = server rotation
  public static boolean isStaticPattern() {
    return isCurmethodForTransaction() && workloadMode == 0;
  }

  public static boolean isDynamicPattern() {
    return isCurmethodForTransaction() && workloadMode != 0;
  }

  public static boolean isNonStableDynamicPattern() {
    return isDynamicPattern() && !dynamicRulePrefix.equals("stable");
  }

  public static boolean isNotSkewed() {
    return getWorkloadName().contains("tracereplay") || getWorkloadName().equals("workload-load");
  }

  // the index of the current rotation in the statistics file (JSONArray)
  public static int getCurRotationStatisticsIdx() {
    MyUtils.myAssert(isStaticPattern());

    int curRotationIdx = -1;
    if (isFirstRotation()) {
      curRotationIdx = 0;
    } else {
      int rotatedServeridx = getRotatedServeridxForRotation();
      MyUtils.myAssert(rotatedServeridx != bottleneckServeridxForRotation);
      if (rotatedServeridx < bottleneckServeridxForRotation) {
        curRotationIdx = rotatedServeridx + 1;
      } else {
        curRotationIdx = rotatedServeridx;
      }
    }
    return curRotationIdx;
  }

  // the strid of the current rotation in the statistics file (JSONArray)
  public static String getCurRotationStatisticsStrid() {
    MyUtils.myAssert(isStaticPattern());

    String tmpstrid = "";
    if (isFirstRotation()) {
      tmpstrid = String.format("server-%d", bottleneckServeridxForRotation);
    } else {
      tmpstrid = String.format("server-%d-%d", bottleneckServeridxForRotation, getRotatedServeridxForRotation());
    }
    return tmpstrid;
  }

  public static int getRotatedServeridxForRotation() {
    MyUtils.myAssert(isStaticPattern() && serverPhysicalNum > 1);
    return serverLogicalIdxesList[1].get(0);
  }

  /**
   * Getter and setter.
   */

  public static int getClientPhysicalIdx() {
    return clientPhysicalIdx;
  }

  public static int getWorkloadMode() {
    return workloadMode;
  }

  public static String getWorkloadName() {
    return workloadName;
  }

  public static int getDynamicPeriodNum() {
    return dynamicPeriodNum;
  }

  public static int getDynamicPeriodInterval() {
    return dynamicPeriodInterval;
  }

  public static String getDynamicRulePrefix() {
    return dynamicRulePrefix;
  }

  public static int getClientPhysicalNum() {
    return clientPhysicalNum;
  }
  public static int getmaxLoadBatchSize() {
    return maxLoadBatchSize;
  }
  //public static int getClientTotalLogicalNum() {
  //  return clientTotalLogicalNum;
  //}

  public static int getMaxServerTotalLogicalNum() {
    return maxServerTotalLogicalNum;
  }

  public static short getClientRotationDataServerPort() {
    return clientRotationDataServerPort;
  }

  public static short getClientSendPktServerPortStart() {
    return clientSendPktServerPortStart;
  }

  public static short getClientRulemapServerPortStart() {
    return clientRulemapServerPortStart;
  }

  public static short getClientWorkerPortStart() {
    return clientWorkerPortStart;
  }

  public static short getServerWorkerPortStart() {
    return serverWorkerPortStart;
  }

  public static String getServerIp() {
    return serverIps.get(0);
  }

  public static int getCurrentClientLogicalNum() {
    return clientLogicalNums.get(GlobalConfig.clientPhysicalIdx);
  }

  public static String getCurrentClientIpForClient0() {
    return clientIpsForClient0.get(GlobalConfig.clientPhysicalIdx);
  }

  public static String getCertianClientIpForClient0(int certainClient) {
    return clientIpsForClient0.get(certainClient);
  }

  public static int getGlobalClientLogicalIndex(int tmpClientLogicalIndex) {
    int tmpGlobalClientLogicalIndex = tmpClientLogicalIndex;
    for (int i = 0; i < clientPhysicalIdx; i++) {
      tmpGlobalClientLogicalIndex += clientLogicalNums.get(i);
    }
    return tmpGlobalClientLogicalIndex;
  }

  public static String getDatabase() {
    return INSWITCHCACHE_DBNAMES.get(curmethodId);
  }


  public static int getCurmethodId() {
    return curmethodId;
  }

  public static void setCurmethodId(int curmethodId) {
    GlobalConfig.curmethodId = curmethodId;
  }

  public static int getSwitchPartitionCount() {
    return switchPartitionCount;
  }

  public static int getServerTotalLogicalNumForRotation() {
    return serverTotalLogicalNumForRotation;
  }

  public static int getBottleneckServeridxForRotation() {
    return bottleneckServeridxForRotation;
  }

  public static int getServerPhysicalNum() {
    return serverPhysicalNum;
  }
  public static int getopcount() {
    return 1000000;
  }
  public static boolean isSingleRotation() {
    return singleRotation;
  }

  public static int getServerTotalLogicalNum() {
    return serverTotalLogicalNum;
  }

  public static int getserverPerServerLogicalNum() {
    return serverPerServerLogicalNum;
  }

  public static boolean isTestingLatency() {
    return latencyTest;
  }

  public static void setTestingLatency() {
    latencyTest = true;
  }

  public static short getClientUpstreamBackupReleaserPort() {
    return clientUpstreamBackupReleaserPort;
  }
  public static short getFakeCachepopClientRecvport() {
    return 10080;
  }
}
