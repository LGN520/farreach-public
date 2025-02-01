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

package site.ycsb.measurements;

import site.ycsb.measurements.exporter.MeasurementsExporter;

import java.io.IOException;
//import java.text.DecimalFormat;
//import java.util.ArrayList;
import java.util.Properties;

import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.MyUtils;
import com.inswitchcache.core.Key;

/**
 * Take measurements and maintain a histogram of a given metric, such as READ LATENCY.
 *
 */
public class OneMeasurementCustomHistogram extends OneMeasurement {
  public static final int BUCKETS_DEFAULT = 10000; // up to 10ms

  // per-server latency statistics of cache hits and misses
  private long[][] perserverCachehitHistogram = null;
  private long[][] perserverCachemissHistogram = null;

  // per-server # of operations outside the histogram's range of cache hits and misses
  private long[] perserverCachehitHistogramoverflow = null;
  private long[] perserverCachemissHistogramoverflow = null;

  // per-server # of operations of cache hits and misses
  private long[] perserverCachehitOperations = null;
  private long[] perserverCachemissOperations = null;

  // per-server sum of latency of all operations of cache hits and misses
  private long[] perserverCachehitTotallatency = null;
  private long[] perserverCachemissTotallatency = null;

  // per-server sum of each latency measurement squared over all operations (used to calculate variance of latency)
  //private double[] perserverTotalsquaredlatency;

  // per-server keep a windowed version of these stats for printing status (similar as OneMeasurementTimeSeries)
  //private long perserverWindowoperations;
  //private long perserverWindowtotallatency;

  //private int perserverMinlatency;
  //private int perserverMaxlatency;

  public OneMeasurementCustomHistogram(String name, Properties props) {
    super(name);

    // Only used for transaction phase
    MyUtils.myAssert(GlobalConfig.isCurmethodForTransaction());
    // # of storage servers in transaction phase must be 1 or 2
    // MyUtils.myAssert(GlobalConfig.getServerTotalLogicalNum() == 1
    //     || GlobalConfig.getServerTotalLogicalNum() == 2);

    perserverCachehitHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][BUCKETS_DEFAULT];
    perserverCachemissHistogram = new long[GlobalConfig.getServerTotalLogicalNum()][BUCKETS_DEFAULT];
    perserverCachehitHistogramoverflow = new long[GlobalConfig.getServerTotalLogicalNum()];
    perserverCachemissHistogramoverflow = new long[GlobalConfig.getServerTotalLogicalNum()];
    perserverCachehitOperations = new long[GlobalConfig.getServerTotalLogicalNum()];
    perserverCachemissOperations = new long[GlobalConfig.getServerTotalLogicalNum()];
    perserverCachehitTotallatency = new long[GlobalConfig.getServerTotalLogicalNum()];
    perserverCachemissTotallatency = new long[GlobalConfig.getServerTotalLogicalNum()];
    for (int i = 0; i < GlobalConfig.getServerTotalLogicalNum(); i++) {
      for (int j = 0; j < BUCKETS_DEFAULT; j++) {
        perserverCachehitHistogram[i][j] = 0;
        perserverCachemissHistogram[i][j] = 0;
      }
      perserverCachehitHistogramoverflow[i] = 0;
      perserverCachemissHistogramoverflow[i] = 0;
      perserverCachehitOperations[i] = 0;
      perserverCachemissOperations[i] = 0;
      perserverCachehitTotallatency[i] = 0;
      perserverCachemissTotallatency[i] = 0;
    }
    //totalsquaredlatency = 0;
    //windowoperations = 0;
    //windowtotallatency = 0;
    //minlatency = -1;
    //maxlatency = -1;
  }

  @Override
  public void measure(int latency) {
    System.out.println("[ERROR][OneMeasurementCustomHistogram] you should provide key to measure latency!");
    System.exit(-1);
  }

  @Override
  public synchronized void measure(String key, boolean ishit, int latency) {
    if (key == null) {
      return; // MUST be CLEANUP
    }

    Key tmpKey = new Key();
    tmpKey.fromString(key);
    int tmpidx = 0;
    if (GlobalConfig.getWorkloadMode() == 1) { // dynamic pattern
      tmpidx = tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
          GlobalConfig.getServerTotalLogicalNum());
    } else { // static pattern
      int tmpServeridx = tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
          GlobalConfig.getServerTotalLogicalNumForRotation());
      if (tmpServeridx == GlobalConfig.getBottleneckServeridxForRotation()) {
        tmpidx = 0;
      } else if (tmpServeridx == GlobalConfig.getRotatedServeridxForRotation()) {
        tmpidx = 1;
      } else {
        System.out.println(String.format("[ERROR][OneMeasurementCustomHistogram] tmpServeridx %d !=" +
            "bottleneckidx %d and != rotatedidx %d", tmpServeridx,
            GlobalConfig.getBottleneckServeridxForRotation(), GlobalConfig.getRotatedServeridxForRotation()));
        System.exit(-1);
      }
    }
    if(tmpidx >= perserverCachehitHistogram.length){
      tmpidx %= perserverCachehitHistogram.length;
      System.out.println("" + tmpidx + " " + perserverCachehitHistogram.length);
    }
    MyUtils.myAssert(tmpidx < perserverCachehitHistogram.length);

    //latency reported in us and collected in bucket by us.
    if (latency >= BUCKETS_DEFAULT) {
      if (ishit) {
        perserverCachehitHistogramoverflow[tmpidx]++;
        perserverCachehitHistogram[tmpidx][BUCKETS_DEFAULT-1]++; // approximate to 9999us
      } else {
        perserverCachemissHistogramoverflow[tmpidx]++;
        perserverCachemissHistogram[tmpidx][BUCKETS_DEFAULT-1]++; // approximate to 9999us
      }
    } else {
      if (ishit) {
        perserverCachehitHistogram[tmpidx][latency]++; // in unit of us
      } else {
        perserverCachemissHistogram[tmpidx][latency]++; // in unit of us
      }
    }
    if (ishit) {
      perserverCachehitOperations[tmpidx]++;
      perserverCachehitTotallatency[tmpidx] += latency;
    } else {
      perserverCachemissOperations[tmpidx]++;
      perserverCachemissTotallatency[tmpidx] += latency;
    }
    //totalsquaredlatency += ((double) latency) * ((double) latency);
    //windowoperations++;
    //windowtotallatency += latency;

    //if ((minlatency < 0) || (latency < minlatency)) {
    //  minlatency = latency;
    //}

    //if ((maxlatency < 0) || (latency > maxlatency)) {
    //  maxlatency = latency;
    //}
  }

  @Override
  public void exportMeasurements(MeasurementsExporter exporter) throws IOException {
    /*long totallatency = 0;
    long operations = 0;
    long[] histogram = new long[BUCKETS_DEFAULT];
    for (int i = 0; i < perserverHistogram.length; i++) {
      totallatency += perserverTotallatency[i];
      operations += perserverOperations[i];
      for (int j = 0; j < BUCKETS_DEFAULT; j++) {
        histogram[j] += perserverHistogram[i][j];
      }
    }

    double mean = totallatency / ((double) operations);
    //double variance = totalsquaredlatency / ((double) operations) - (mean * mean);
    exporter.write(getName(), "Operations", operations);
    exporter.write(getName(), "AverageLatency(us)", mean);
    //exporter.write(getName(), "LatencyVariance(us)", variance);
    //exporter.write(getName(), "MinLatency(us)", minlatency);
    //exporter.write(getName(), "MaxLatency(us)", maxlatency);

    long opcounter=0;
    boolean doneMedium = false;
    boolean done95th = false;
    for (int i = 0; i < BUCKETS_DEFAULT; i++) {
      opcounter += histogram[i];
      if ((!doneMedium) && (((double) opcounter) / ((double) operations) >= 0.50)) {
        exporter.write(getName(), "MediumLatency(us)", i);
        doneMedium = true;
      }
      if ((!done95th) && (((double) opcounter) / ((double) operations) >= 0.95)) {
        exporter.write(getName(), "95thPercentileLatency(us)", i);
        done95th = true;
      }
      if (((double) opcounter) / ((double) operations) >= 0.99) {
        exporter.write(getName(), "99thPercentileLatency(us)", i);
        break;
      }
    }*/

    exportStatusCounts(exporter);
  }

  @Override
  public String getSummary() { // invoked by StatusThread per second
    //if (windowoperations == 0) {
    //  return "";
    //}
    //DecimalFormat d = new DecimalFormat("#.##");
    //double report = ((double) windowtotallatency) / ((double) windowoperations);
    //windowtotallatency = 0;
    //windowoperations = 0;
    //return "[" + getName() + " AverageLatency(us)=" + d.format(report) + "]";
    return "";
  }

  // Getter and setter

  public long[][] getPerserverCachehitHistogram() {
    return perserverCachehitHistogram;
  }

  public long[][] getPerserverCachemissHistogram() {
    return perserverCachemissHistogram;
  }

  public long[] getPerserverCachehitOperations() {
    return perserverCachehitOperations;
  }

  public long[] getPerserverCachemissOperations() {
    return perserverCachemissOperations;
  }

  public long[] getPerserverCachehitTotallatency() {
    return perserverCachehitTotallatency;
  }

  public long[] getPerserverCachemissTotallatency() {
    return perserverCachemissTotallatency;
  }

}
