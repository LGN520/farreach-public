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

package site.ycsb;

import site.ycsb.measurements.Measurements;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import com.inswitchcache.InswitchCacheClient;
import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.MyUtils;
import com.inswitchcache.core.StatisticsHelper;

import site.ycsb.measurements.OneMeasurement;
import site.ycsb.measurements.OneMeasurementCustomHistogram;

/**
 * A thread to periodically show the status of the experiment to reassure you that progress is being made.
 */
public class StatusThread extends Thread {
  // Counts down each of the clients completing
  private final CountDownLatch completeLatch;

  // Stores the measurements for the run
  private final Measurements measurements;

  // Whether or not to track the JVM stats per run
  private final boolean trackJVMStats;

  // The clients that are running.
  private final List<ClientThread> clients;

  private final String label;
  private final boolean standardstatus;

  // The interval for reporting status.
  private long sleeptimeNs;

  // JVM max/mins
  private int maxThreads;
  private int minThreads = Integer.MAX_VALUE;
  private long maxUsedMem;
  private long minUsedMem = Long.MAX_VALUE;
  private double maxLoadAvg;
  private double minLoadAvg = Double.MAX_VALUE;
  private long lastGCCount = 0;
  private long lastGCTime = 0;

  // for inswitchcache databases
  private int cursec = 0;
  private int[] lastSwitchAndServerLoadList = null;
  private int[] lastPerserverCachehits = null;
  private long[] lastPerserverCachehitTotalLatency = null;
  private long[] lastPerserverCachehitTotalLatencynum = null;
  private long[][] lastPerserverCachehitTotalHistogram = null;
  private long[] lastPerserverCachemissTotalLatency = null;
  private long[] lastPerserverCachemissTotalLatencynum = null;
  private long[][] lastPerserverCachemissTotalHistogram = null;

  /**
   * Creates a new StatusThread without JVM stat tracking.
   *
   * @param completeLatch         The latch that each client thread will {@link CountDownLatch#countDown()}
   *                              as they complete.
   * @param clients               The clients to collect metrics from.
   * @param label                 The label for the status.
   * @param standardstatus        If true the status is printed to stdout in addition to stderr.
   * @param statusIntervalSeconds The number of seconds between status updates.
   */
  public StatusThread(CountDownLatch completeLatch, List<ClientThread> clients,
                      String label, boolean standardstatus, int statusIntervalSeconds) {
    this(completeLatch, clients, label, standardstatus, statusIntervalSeconds, false);
  }

  /**
   * Creates a new StatusThread.
   *
   * @param completeLatch         The latch that each client thread will {@link CountDownLatch#countDown()}
   *                              as they complete.
   * @param clients               The clients to collect metrics from.
   * @param label                 The label for the status.
   * @param standardstatus        If true the status is printed to stdout in addition to stderr.
   * @param statusIntervalSeconds The number of seconds between status updates.
   * @param trackJVMStats         Whether or not to track JVM stats.
   */
  public StatusThread(CountDownLatch completeLatch, List<ClientThread> clients,
                      String label, boolean standardstatus, int statusIntervalSeconds,
                      boolean trackJVMStats) {
    this.completeLatch = completeLatch;
    this.clients = clients;
    this.label = label;
    this.standardstatus = standardstatus;
    sleeptimeNs = TimeUnit.SECONDS.toNanos(statusIntervalSeconds);
    measurements = Measurements.getMeasurements();
    this.trackJVMStats = trackJVMStats;
  }

  /**
   * Run and periodically report status.
   */
  @Override
  public void run() {
    final long startTimeMs = System.currentTimeMillis();
    final long startTimeNanos = System.nanoTime();
    long deadline = startTimeNanos + sleeptimeNs;
    long startIntervalMs = startTimeMs;
    long lastTotalOps = 0;

    boolean alldone;

    do {
      long nowMs = System.currentTimeMillis();

      lastTotalOps = computeStats(startTimeMs, startIntervalMs, nowMs, lastTotalOps);

      if (trackJVMStats) {
        measureJVM();
      }

      alldone = waitForClientsUntil(deadline);

      startIntervalMs = nowMs;
      deadline += sleeptimeNs;
    }
    while (!alldone);

    if (trackJVMStats) {
      measureJVM();
    }
    // Print the final stats.
    computeStats(startTimeMs, startIntervalMs, System.currentTimeMillis(), lastTotalOps);

    // Dump per-second statistics by StatisticsHelper
    if (GlobalConfig.isInswitchCacheMethod() && GlobalConfig.isDynamicPattern()) {
      StatisticsHelper.flush();
    }
  }

  /**
   * Computes and prints the stats.
   *
   * @param startTimeMs     The start time of the test.
   * @param startIntervalMs The start time of this interval.
   * @param endIntervalMs   The end time (now) for the interval.
   * @param lastTotalOps    The last total operations count.
   * @return The current operation count.
   */
  private long computeStats(final long startTimeMs, long startIntervalMs, long endIntervalMs,
                            long lastTotalOps) {
    SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:SSS");

    long totalops = 0;
    long todoops = 0;

    // Calculate the total number of operations completed.
    // System.out.print("[DEBUG][StatusThread] ");
    for (ClientThread t : clients) {
      totalops += t.getOpsDone();
      todoops += t.getOpsTodo();
      // System.out.print(t.getOpsTodo() + " ");
    }
    // System.out.println();


    long interval = endIntervalMs - startTimeMs;
    double throughput = 1000.0 * (((double) totalops) / (double) interval);
    double curthroughput = 1000.0 * (((double) (totalops - lastTotalOps)) /
        ((double) (endIntervalMs - startIntervalMs)));
    long estremaining = (long) Math.ceil(todoops / throughput);
    // long estremaining = (long) todoops;


    DecimalFormat d = new DecimalFormat("#.####");
    String labelString = this.label + format.format(new Date());

    StringBuilder msg = new StringBuilder(labelString).append(" ").append(interval / 1000).append(" sec: ");
    msg.append(totalops).append(" operations; ");

    if (totalops != 0) {
      msg.append(d.format(curthroughput)).append(" current ops/sec; ");
    }
    if (todoops != 0) {
      msg.append("est completion in ").append(RemainingFormatter.format(estremaining));
      // msg.append("est completion in ").append(d.format(estremaining));
    }

    // Add for InswitchCacheDatabase under dynamic pattern
    //if (totalops - lastTotalOps != 0) {
    if (GlobalConfig.isInswitchCacheMethod() && GlobalConfig.isDynamicPattern()
        && startTimeMs != startIntervalMs) {
      // Get throughput statistics within the current second
      int curTotalOpsdone = (int) (totalops - lastTotalOps);
      int[] nowSwitchAndLoadList = InswitchCacheClient.getSwitchAndServerLoadList();
      int[] curPerserverOpsdone = new int[nowSwitchAndLoadList.length - 1];
      for (int i = 1; i < nowSwitchAndLoadList.length; i++) {
        curPerserverOpsdone[i-1] = nowSwitchAndLoadList[i];
      }
      if (lastSwitchAndServerLoadList != null) {
        for (int i = 1; i < nowSwitchAndLoadList.length; i++) {
          curPerserverOpsdone[i-1] -= lastSwitchAndServerLoadList[i];
        }
      }
      int[] nowPerserverCachehits = InswitchCacheClient.getCachehitLoadList();
      int[] curPerserverCachehits = new int[nowPerserverCachehits.length];
      for (int i = 0; i < curPerserverCachehits.length; i++) {
        curPerserverCachehits[i] = nowPerserverCachehits[i];
      }
      if (lastPerserverCachehits != null) {
        for (int i = 0; i < curPerserverCachehits.length; i++) {
          curPerserverCachehits[i] -= lastPerserverCachehits[i];
        }
      }
      long curExecutionMillis = endIntervalMs - startIntervalMs;

      // Get latency statistics within the current second
      ConcurrentHashMap<String, OneMeasurement> opToMesurementMap = Measurements.getMeasurements().getOpToMesurementMap();
      long[] nowPerserverCachehitTotalLatency = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[] nowPerserverCachehitTotalLatencynum = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[][] nowPerserverCachehitTotalHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][OneMeasurementCustomHistogram.BUCKETS_DEFAULT];
      long[] nowPerserverCachemissTotalLatency = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[] nowPerserverCachemissTotalLatencynum = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[][] nowPerserverCachemissTotalHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][OneMeasurementCustomHistogram.BUCKETS_DEFAULT];
      for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
        nowPerserverCachehitTotalLatency[i] = 0;
        nowPerserverCachehitTotalLatencynum[i] = 0;
        nowPerserverCachemissTotalLatency[i] = 0;
        nowPerserverCachemissTotalLatencynum[i] = 0;
        for (int j = 0; j < OneMeasurementCustomHistogram.BUCKETS_DEFAULT; j++) {
          nowPerserverCachehitTotalHistogram[i][j] = 0;
          nowPerserverCachemissTotalHistogram[i][j] = 0;
        }
      }

      for (Map.Entry<String, OneMeasurement> mEntry : opToMesurementMap.entrySet()) {
        OneMeasurement m = mEntry.getValue();
        MyUtils.myAssert(m instanceof OneMeasurementCustomHistogram);
        if (mEntry.getKey().contains("CLEANUP")) {
          continue; // NOT measure CLEANUP operations
        }

        long[] tmpPerserverCachehitTotalLatency = ((OneMeasurementCustomHistogram) m).getPerserverCachehitTotallatency();
        long[] tmpPerserverCachehitTotalLatencynum = ((OneMeasurementCustomHistogram) m).getPerserverCachehitOperations();
        long[][] tmpPerserverCachehitTotalHistogram = ((OneMeasurementCustomHistogram) m).getPerserverCachehitHistogram();
        long[] tmpPerserverCachemissTotalLatency = ((OneMeasurementCustomHistogram) m).getPerserverCachemissTotallatency();
        long[] tmpPerserverCachemissTotalLatencynum = ((OneMeasurementCustomHistogram) m).getPerserverCachemissOperations();
        long[][] tmpPerserverCachemissTotalHistogram = ((OneMeasurementCustomHistogram) m).getPerserverCachemissHistogram();
        MyUtils.myAssert(tmpPerserverCachehitTotalLatency.length == GlobalConfig.getServerTotalLogicalNum());
        MyUtils.myAssert(tmpPerserverCachehitTotalLatencynum.length == GlobalConfig.getServerTotalLogicalNum());
        MyUtils.myAssert(tmpPerserverCachehitTotalHistogram.length == GlobalConfig.getServerTotalLogicalNum());
        MyUtils.myAssert(tmpPerserverCachemissTotalLatency.length == GlobalConfig.getServerTotalLogicalNum());
        MyUtils.myAssert(tmpPerserverCachemissTotalLatencynum.length == GlobalConfig.getServerTotalLogicalNum());
        MyUtils.myAssert(tmpPerserverCachemissTotalHistogram.length == GlobalConfig.getServerTotalLogicalNum());
        for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
          nowPerserverCachehitTotalLatency[i] += tmpPerserverCachehitTotalLatency[i];
          nowPerserverCachehitTotalLatencynum[i] += tmpPerserverCachehitTotalLatencynum[i];
          nowPerserverCachemissTotalLatency[i] += tmpPerserverCachemissTotalLatency[i];
          nowPerserverCachemissTotalLatencynum[i] += tmpPerserverCachemissTotalLatencynum[i];
          for (int j = 0; j < OneMeasurementCustomHistogram.BUCKETS_DEFAULT; j++) {
            nowPerserverCachehitTotalHistogram[i][j] += tmpPerserverCachehitTotalHistogram[i][j];
            nowPerserverCachemissTotalHistogram[i][j] += tmpPerserverCachemissTotalHistogram[i][j];
          }
        }
      }

      long[] curPerserverCachehitTotalLatency = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[] curPerserverCachemissTotalLatency = new long[GlobalConfig.getServerTotalLogicalNum()];
      for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
        curPerserverCachehitTotalLatency[i] = nowPerserverCachehitTotalLatency[i];
        curPerserverCachemissTotalLatency[i] = nowPerserverCachemissTotalLatency[i];
      }
      if (lastPerserverCachehitTotalLatency != null) {
        for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
          curPerserverCachehitTotalLatency[i] -= lastPerserverCachehitTotalLatency[i];
        }
      }
      if (lastPerserverCachemissTotalLatency != null) {
        for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
          curPerserverCachemissTotalLatency[i] -= lastPerserverCachemissTotalLatency[i];
        }
      }

      long[] curPerserverCachehitTotalLatencynum = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[] curPerserverCachemissTotalLatencynum = new long[GlobalConfig.getServerTotalLogicalNum()];
      for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
        curPerserverCachehitTotalLatencynum[i] = nowPerserverCachehitTotalLatencynum[i];
        curPerserverCachemissTotalLatencynum[i] = nowPerserverCachemissTotalLatencynum[i];
      }
      if (lastPerserverCachehitTotalLatencynum != null) {
        for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
          curPerserverCachehitTotalLatencynum[i] -= lastPerserverCachehitTotalLatencynum[i];
        }
      }
      if (lastPerserverCachemissTotalLatencynum != null) {
        for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
          curPerserverCachemissTotalLatencynum[i] -= lastPerserverCachemissTotalLatencynum[i];
        }
      }

      long[][] curPerserverCachehitTotalHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][OneMeasurementCustomHistogram.BUCKETS_DEFAULT];
      long[][] curPerserverCachemissTotalHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][OneMeasurementCustomHistogram.BUCKETS_DEFAULT];
      for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
        for (int j = 0; j < OneMeasurementCustomHistogram.BUCKETS_DEFAULT; j++) {
          curPerserverCachehitTotalHistogram[i][j] = nowPerserverCachehitTotalHistogram[i][j];
          curPerserverCachemissTotalHistogram[i][j] = nowPerserverCachemissTotalHistogram[i][j];
        }
      }
      if (lastPerserverCachehitTotalHistogram != null) {
        for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
          for (int j = 0; j < OneMeasurementCustomHistogram.BUCKETS_DEFAULT; j++) {
            curPerserverCachehitTotalHistogram[i][j] -= lastPerserverCachehitTotalHistogram[i][j];
          }
        }
      }
      if (lastPerserverCachemissTotalHistogram != null) {
        for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
          for (int j = 0; j < OneMeasurementCustomHistogram.BUCKETS_DEFAULT; j++) {
            curPerserverCachemissTotalHistogram[i][j] -= lastPerserverCachemissTotalHistogram[i][j];
          }
        }
      }

      // Collect statistics within the current second
      String tmpstrid = String.format("sec-%d", cursec);
      StatisticsHelper.collect(tmpstrid, curTotalOpsdone, curPerserverOpsdone, curPerserverCachehits, curExecutionMillis,
          curPerserverCachehitTotalLatency, curPerserverCachehitTotalLatencynum, curPerserverCachehitTotalHistogram,
          curPerserverCachemissTotalLatency, curPerserverCachemissTotalLatencynum, curPerserverCachemissTotalHistogram);

      // TMPDEBUG
      int curCacheHitCnt = curPerserverOpsdone[0]; // cache hit rate
      double cacheHitRate = (((double)curCacheHitCnt) / ((double) (totalops - lastTotalOps)));
      msg.append("; hit count rate: ").append(d.format(cacheHitRate));
      int curMaxServerLoad = 0; // normalized throughput
      for (int i = 1; i < curPerserverOpsdone.length; i++) {
        if (curPerserverOpsdone[i] > curMaxServerLoad) {
          curMaxServerLoad = curPerserverOpsdone[i];
        }
      }
      double normalizedThpt = ((double)(totalops - lastTotalOps)) / ((double)curMaxServerLoad);
      msg.append("; normalized throughput: ").append(d.format(normalizedThpt));

      cursec += 1;
      lastSwitchAndServerLoadList = nowSwitchAndLoadList;
      lastPerserverCachehits = nowPerserverCachehits;
      lastPerserverCachehitTotalLatency = nowPerserverCachehitTotalLatency;
      lastPerserverCachehitTotalLatencynum = nowPerserverCachehitTotalLatencynum;
      lastPerserverCachehitTotalHistogram = nowPerserverCachehitTotalHistogram;
      lastPerserverCachemissTotalLatency = nowPerserverCachemissTotalLatency;
      lastPerserverCachemissTotalLatencynum = nowPerserverCachemissTotalLatencynum;
      lastPerserverCachemissTotalHistogram = nowPerserverCachemissTotalHistogram;
    }

    msg.append(Measurements.getMeasurements().getSummary());

    // System.err.println(msg);

    // NOTE: as we use OneMeasurementCustomHistogram instead of OneMeasurementTimeseries and hence standardstatus = false,
    //     you need to comment the condition judgement if you want to print msg.
    if (standardstatus) {
      System.out.println(msg);
    }

    return totalops;
  }

  /**
   * Waits for all of the client to finish or the deadline to expire.
   *
   * @param deadline The current deadline.
   * @return True if all of the clients completed.
   */
  private boolean waitForClientsUntil(long deadline) {
    boolean alldone = false;
    long now = System.nanoTime();

    while (!alldone && now < deadline) {
      try {
        alldone = completeLatch.await(deadline - now, TimeUnit.NANOSECONDS);
      } catch (InterruptedException ie) {
        // If we are interrupted the thread is being asked to shutdown.
        // Return true to indicate that and reset the interrupt state
        // of the thread.
        Thread.currentThread().interrupt();
        alldone = true;
      }
      now = System.nanoTime();
    }

    return alldone;
  }

  /**
   * Executes the JVM measurements.
   */
  private void measureJVM() {
    final int threads = Utils.getActiveThreadCount();
    if (threads < minThreads) {
      minThreads = threads;
    }
    if (threads > maxThreads) {
      maxThreads = threads;
    }
    measurements.measure("THREAD_COUNT", threads);

    // TODO - once measurements allow for other number types, switch to using
    // the raw bytes. Otherwise we can track in MB to avoid negative values
    // when faced with huge heaps.
    final int usedMem = Utils.getUsedMemoryMegaBytes();
    if (usedMem < minUsedMem) {
      minUsedMem = usedMem;
    }
    if (usedMem > maxUsedMem) {
      maxUsedMem = usedMem;
    }
    measurements.measure("USED_MEM_MB", usedMem);

    // Some JVMs may not implement this feature so if the value is less than
    // zero, just ommit it.
    final double systemLoad = Utils.getSystemLoadAverage();
    if (systemLoad >= 0) {
      // TODO - store the double if measurements allows for them
      measurements.measure("SYS_LOAD_AVG", (int) systemLoad);
      if (systemLoad > maxLoadAvg) {
        maxLoadAvg = systemLoad;
      }
      if (systemLoad < minLoadAvg) {
        minLoadAvg = systemLoad;
      }
    }

    final long gcs = Utils.getGCTotalCollectionCount();
    measurements.measure("GCS", (int) (gcs - lastGCCount));
    final long gcTime = Utils.getGCTotalTime();
    measurements.measure("GCS_TIME", (int) (gcTime - lastGCTime));
    lastGCCount = gcs;
    lastGCTime = gcTime;
  }

  /**
   * @return The maximum threads running during the test.
   */
  public int getMaxThreads() {
    return maxThreads;
  }

  /**
   * @return The minimum threads running during the test.
   */
  public int getMinThreads() {
    return minThreads;
  }

  /**
   * @return The maximum memory used during the test.
   */
  public long getMaxUsedMem() {
    return maxUsedMem;
  }

  /**
   * @return The minimum memory used during the test.
   */
  public long getMinUsedMem() {
    return minUsedMem;
  }

  /**
   * @return The maximum load average during the test.
   */
  public double getMaxLoadAvg() {
    return maxLoadAvg;
  }

  /**
   * @return The minimum load average during the test.
   */
  public double getMinLoadAvg() {
    return minLoadAvg;
  }

  /**
   * @return Whether or not the thread is tracking JVM stats.
   */
  public boolean trackJVMStats() {
    return trackJVMStats;
  }
}
