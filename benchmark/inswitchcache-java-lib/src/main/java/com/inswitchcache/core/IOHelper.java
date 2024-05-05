package com.inswitchcache.core;

public final class IOHelper {
  public static String getDynamicRulepath(String workloadName, String dynamicRulePrefix) {
    return String.format("../output/%s-%srules/", workloadName, dynamicRulePrefix);
  }

  public static String getWorkloadPatternFile(String workloadName) {
    return String.format("workloads/%s", workloadName);
  }

  public static String getHotkeysDumpFilepath(String workloadName) {
    return String.format("../output/%s-hotest.out", workloadName);
  }

  public static String getNearhotkeysDumpFilepath(String workloadName) {
    return String.format("../output/%s-nearhot.out", workloadName);
  }

  public static String getColdkeysDumpFilepath(String workloadName) {
    return String.format("../output/%s-coldest.out", workloadName);
  }

  public static String getPregenerateWorkloadDirpath(String workloadName) {
    return String.format("../output/%s-pregeneration", workloadName);
  }

  public static String getPregenerateWorkloadFilepath(String workloadName, int localLogicalClientidx) {
    String directoryPath = getPregenerateWorkloadDirpath(workloadName);
    return String.format("%s/logicalclient-%d.out", directoryPath, localLogicalClientidx);
  }

  public static String getStatisticsDirpath(String workloadName) {
    return String.format("../output/%s-statistics", workloadName);
  }

  public static String getDynamicStatisticsFilepath(String workloadName, String dbName, String dynamicRulePrefix,
      int clientPhysicalIdx) {
    String statisticsDirpath = getStatisticsDirpath(workloadName);
    return String.format("%s/%s-%s-client%d.out", statisticsDirpath,
        dbName, dynamicRulePrefix, clientPhysicalIdx);
  }

  public static String getStaticStatisticsFilepath(String workloadName, String dbName, int rotationScale,
      int clientPhysicalIdx) {
    String statisticsDirpath = getStatisticsDirpath(workloadName);
    return String.format("%s/%s-static%d-client%d.out", statisticsDirpath,
        dbName, rotationScale, clientPhysicalIdx);
  }

  public static String getTwitterCacheTraceFilepath(int clusterNum) {
    return String.format("../output/twitter-trace-%d", clusterNum);
  }

  public static String getUpstreamBackupDirpath() {
    return "../output/upstreambackups";
  }

  public static String getDynamicUpstreamBackupFilepath(int clientPhysicalIdx) {
    String dirpath = getUpstreamBackupDirpath();
    return String.format("%s/dynamic-client%d.out", dirpath, clientPhysicalIdx);
  }

  public static String getStaticUpstreamBackupFilepath(int rotateScale, int bottleneckIdx, int rotateIdx,
      int clientPhysicalIdx) {
    String dirpath = getUpstreamBackupDirpath();
    if (bottleneckIdx == rotateIdx) {
      return String.format("%s/static%d-%d-client%d.out", dirpath, rotateScale, bottleneckIdx, clientPhysicalIdx);
    } else {
      return String.format("%s/static%d-%d-%d-client%d.out", dirpath, rotateScale, bottleneckIdx, rotateIdx,
          clientPhysicalIdx);
    }
  }
}
