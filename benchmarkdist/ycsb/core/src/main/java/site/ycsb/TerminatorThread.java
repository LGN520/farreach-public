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

import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import com.inswitchcache.InswitchCacheClient;
import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.StatisticsHelper;
import com.inswitchcache.core.MyUtils;
import site.ycsb.measurements.Measurements;
import site.ycsb.measurements.OneMeasurement;
import site.ycsb.measurements.OneMeasurementCustomHistogram;

/**
 * A thread that waits for the maximum specified time and then interrupts all the client
 * threads passed at initialization of this thread.
 *
 * The maximum execution time passed is assumed to be in seconds.
 *
 */
public class TerminatorThread extends Thread {

  private final Collection<? extends Thread> threads;
  private long maxExecutionTime;
  private Workload workload;
  private long waitTimeOutInMS;

  private List<ClientThread> clients = null;

  public TerminatorThread(long maxExecutionTime, Collection<? extends Thread> threads,
                          Workload workload, List<ClientThread> clients) {
    this.maxExecutionTime = maxExecutionTime;
    this.threads = threads;
    this.workload = workload;
    this.clients = clients;
    waitTimeOutInMS = 2000;
    System.err.println("Maximum execution time specified as: " + maxExecutionTime + " secs");
  }

  public void run() {
    long st = System.currentTimeMillis();

    try {
      InswitchCacheClient.runClientRulemapClient(maxExecutionTime);
    } catch (InterruptedException e) {
      System.err.println("Could not wait until max specified time, TerminatorThread interrupted.");
      // return;
    }

    long en = System.currentTimeMillis();

    // NOTE: collect and dump static pattern w/ server rotation in TerminatorThread
    //     instead of the main thread of Client to avoid clients join overhead especially with udp receiving overhead
    if (GlobalConfig.isStaticPattern()) {
      // Get throughput statistics from YCSB
      int totalOpsdone = 0;
      for (int i = 0; i < clients.size(); i++) {
        totalOpsdone += clients.get(i).getOpsDone();
      }
      int[] switchAndServerLoadList = InswitchCacheClient.getSwitchAndServerLoadList();
      int[] perserverOpsdone = new int[switchAndServerLoadList.length - 1];
      for (int i = 1; i < switchAndServerLoadList.length; i++) {
        perserverOpsdone[i-1] = switchAndServerLoadList[i];
      }
      int[] perserverCachehits = InswitchCacheClient.getCachehitLoadList();
      // NOTE: we keep the time of each rotation the same for fair comparison among all methods
      long executionMillis = en - st; // in unit of ms
      System.out.println("[debug]executionMillis:"+ executionMillis);
      //long executionMillis = maxExecutionTime * 1000; // in unit of ms

      // Get latency histogram from YCSB
      ConcurrentHashMap<String, OneMeasurement> opToMesurementMap = Measurements.getMeasurements().getOpToMesurementMap();
      long[] perserverCachehitTotalLatency = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[] perserverCachehitTotalLatencynum = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[][] perserverCachehitTotalHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][OneMeasurementCustomHistogram.BUCKETS_DEFAULT];
      long[] perserverCachemissTotalLatency = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[] perserverCachemissTotalLatencynum = new long[GlobalConfig.getServerTotalLogicalNum()];
      long[][] perserverCachemissTotalHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][OneMeasurementCustomHistogram.BUCKETS_DEFAULT];
      System.out.print("serverOps: ["+perserverOpsdone[0]);
      for (int i = 1; i < perserverOpsdone.length; i++) {
        System.out.print(","+perserverOpsdone[i]);
      }
      System.out.println("]");
      System.out.print("Cachehits: ["+perserverCachehits[0]);
      for (int i = 1; i < perserverCachehits.length; i++) {
        System.out.print(","+perserverCachehits[i]);
      }
      System.out.println("]");
      // System.out.println(perserverOpsdone);
      // System.out.println(perserverCachehits);
      // Collect and dump statistics by StatisticsHelper
      String tmpstrid = GlobalConfig.getCurRotationStatisticsStrid();
      // System.out.println("[debug]"+tmpstrid + " " + totalOpsdone);
      StatisticsHelper.collect(tmpstrid, totalOpsdone, perserverOpsdone, perserverCachehits, executionMillis,
          perserverCachehitTotalLatency, perserverCachehitTotalLatencynum, perserverCachehitTotalHistogram,
          perserverCachemissTotalLatency, perserverCachemissTotalLatencynum, perserverCachemissTotalHistogram);
      StatisticsHelper.flush();
    }

    if (GlobalConfig.getCurmethodId() == GlobalConfig.FARREACH_ID) {
      InswitchCacheClient.dumpUpstreamBackups();
    }

    System.err.println("Maximum time elapsed. Requesting stop for the workload.");
    workload.requestStop();
    if (GlobalConfig.isInswitchCacheMethod()) {
      InswitchCacheClient.stop(); // stop timeout-and-retry
    }
    System.err.println("Stop requested for workload. Now Joining!");
    for (Thread t : threads) {
      while (t.isAlive()) {
        try {
          t.join(waitTimeOutInMS);
          if (t.isAlive()) {
            System.out.println("Still waiting for thread " + t.getName() + " to complete. " +
                "Workload status: " + workload.isStopRequested());
          }
        } catch (InterruptedException e) {
          // Do nothing. Don't know why I was interrupted.
        }
      }
    }
  }
}
