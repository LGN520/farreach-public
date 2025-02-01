package com.inswitchcache.core;

import java.io.*;
//import java.util.ArrayList;

import org.json.simple.JSONObject;
import org.json.simple.JSONArray;
import org.json.simple.parser.JSONParser;

/**
 * StatisticsHelper helps to collect and dump statistics in a fixed format for aggregation later.
 * Dump format of each collect:
 *
 */
public final class StatisticsHelper {
  private static JSONArray statisticsJsonArray = new JSONArray();

  private static final String STRID = "strid";
  private static final String TOTAL_OPSDONE = "totalOpsdone";
  private static final String PERSERVER_OPSDONE = "perserverOpsdone";
  private static final String PERSERVER_CACHEHITS = "perserverCachehits";
  private static final String EXECUTION_MILLIS = "executionMillis";
  //private static final String TOTAL_LATENCY = "totalLatency";
  //private static final String TOTAL_LATENCYNUM = "totalLatencynum";
  //private static final String TOTAL_HISTOGRAM = "totalHistogram";
  //private static final String PERSERVER_TOTAL_LATENCY = "perservertotalLatency";
  //private static final String PERSERVER_TOTAL_LATENCYNUM = "perservertotalLatencynum";
  //private static final String PERSERVER_TOTAL_HISTOGRAM = "perservertotalHistogram";
  private static final String PERSERVER_CACHEHIT_TOTAL_LATENCY = "perserverCachehitTotalLatency";
  private static final String PERSERVER_CACHEHIT_TOTAL_LATENCYNUM = "perserverCachehitTotalLatencynum";
  private static final String PERSERVER_CACHEHIT_TOTAL_HISTOGRAM = "perserverCachehitTotalHistogram";
  private static final String PERSERVER_CACHEMISS_TOTAL_LATENCY = "perserverCachemissTotalLatency";
  private static final String PERSERVER_CACHEMISS_TOTAL_LATENCYNUM = "perserverCachemissTotalLatencynum";
  private static final String PERSERVER_CACHEMISS_TOTAL_HISTOGRAM = "perserverCachemissTotalHistogram";

  public static void collect(String strid, int totalOpsdone, int[] perserverOpsdone, int[] perserverCachehits, long executionMillis,
                             long[] perserverCachehitTotalLatency, long[] perserverCachehitTotalLatencynum, long[][] perserverCachehitTotalHistogram,
                             long[] perserverCachemissTotalLatency, long[] perserverCachemissTotalLatencynum, long[][] perserverCachemissTotalHistogram) {
    // Collect statistics into statisticsJsonArray
    JSONObject tmpJsonObject = new JSONObject();
    tmpJsonObject.put(STRID, strid);
    tmpJsonObject.put(TOTAL_OPSDONE, totalOpsdone);
    tmpJsonObject.put(PERSERVER_OPSDONE, array2jsonarray(perserverOpsdone));
    tmpJsonObject.put(PERSERVER_CACHEHITS, array2jsonarray(perserverCachehits));
    tmpJsonObject.put(EXECUTION_MILLIS, executionMillis);
    tmpJsonObject.put(PERSERVER_CACHEHIT_TOTAL_LATENCY, array2jsonarray(perserverCachehitTotalLatency));
    tmpJsonObject.put(PERSERVER_CACHEHIT_TOTAL_LATENCYNUM, array2jsonarray(perserverCachehitTotalLatencynum));
    tmpJsonObject.put(PERSERVER_CACHEHIT_TOTAL_HISTOGRAM, array2jsonarray(perserverCachehitTotalHistogram));
    tmpJsonObject.put(PERSERVER_CACHEMISS_TOTAL_LATENCY, array2jsonarray(perserverCachemissTotalLatency));
    tmpJsonObject.put(PERSERVER_CACHEMISS_TOTAL_LATENCYNUM, array2jsonarray(perserverCachemissTotalLatencynum));
    tmpJsonObject.put(PERSERVER_CACHEMISS_TOTAL_HISTOGRAM, array2jsonarray(perserverCachemissTotalHistogram));
    statisticsJsonArray.add(tmpJsonObject);
  }

  public static void flush() {
    // flush statistics into disk
    try {
      // check statistics directory
      String statisticsDirpath = IOHelper.getStatisticsDirpath(GlobalConfig.getWorkloadName());
      File statisticsDir = new File(statisticsDirpath);
      if (GlobalConfig.isSingleRotation()) {
        // statistics file must exist for single rotation mode
        if (!statisticsDir.exists()) {
          System.out.println(String.format("[ERROR][StatisticsHelper] no such file %s for single rotation; " +
              "please re-run the entire server rotation!", statisticsDirpath));
          System.exit(-1);
        }
      } else if (!statisticsDir.exists()) {
        statisticsDir.mkdir();
      }

      // flush static/dynaimc statistics
      if (GlobalConfig.isStaticPattern()) {
        // at most one-rotation statistics under static pattern
        MyUtils.myAssert(statisticsJsonArray.size() == 1);

        String statisticsFilepath;
        if (GlobalConfig.isTestingLatency()) {
          statisticsFilepath = IOHelper.getStaticStatisticsFilepath(
              GlobalConfig.getWorkloadName(), GlobalConfig.getDatabase()+"-latency",
              GlobalConfig.getServerTotalLogicalNumForRotation(), GlobalConfig.getClientPhysicalIdx());
        } else {
          statisticsFilepath = IOHelper.getStaticStatisticsFilepath(
              GlobalConfig.getWorkloadName(), GlobalConfig.getDatabase(),
              GlobalConfig.getServerTotalLogicalNumForRotation(), GlobalConfig.getClientPhysicalIdx());
        }

        File statisticsFile = new File(statisticsFilepath);
        JSONArray existingJsonArray = new JSONArray();
        if (GlobalConfig.isFirstRotation()) {
          if (GlobalConfig.isSingleRotation()) {
            // statistics file must exist for single rotation mode
            if (!statisticsFile.exists()) {
              System.out.println(String.format("[ERROR][StatisticsHelper] no such file %s for single rotation; " +
                  "please re-run the entire server rotation!", statisticsFilepath));
              System.exit(-1);
            }

            // load JSONArray from existing statistics file for the first rotation under single rotation mode
            Reader reader = new FileReader(statisticsFile);
            JSONParser parser = new JSONParser();
            existingJsonArray = (JSONArray) parser.parse(reader);
            reader.close();
          } else if (statisticsFile.exists()) {
            // delete existing statistics file for the first rotation
            System.out.println(String.format("[WARN][StatisticsHelper] %s already exists -> delete...", statisticsFilepath));
            statisticsFile.delete();
          }
        } else {
          MyUtils.myAssert(statisticsFile.exists()); // statistics file must exist for each subsequent rotation

          // load JSONArray from existing statistics file for the subsequent rotation
          Reader reader = new FileReader(statisticsFile);
          JSONParser parser = new JSONParser();
          existingJsonArray = (JSONArray) parser.parse(reader);
          reader.close();
        }

        if (GlobalConfig.isSingleRotation()) {
          for (int i = 0; i < statisticsJsonArray.size(); i++) { // NOTE: size must be 1
            JSONObject tmpCurJsonobj = (JSONObject) statisticsJsonArray.get(i);

            int tmpExistingIdx = -1; // -1 means tmpCurStrid does not exist in existingJsonArray
            String tmpCurStrid = (String) tmpCurJsonobj.get(STRID);
            for (int j = 0; j < existingJsonArray.size(); j++) {
              System.out.println(String.format("[DEBUG][StatisticsHelper] existingJsonArray[%d][%s] %s vs. tmpCurStrid %s -> %b",
                  j, STRID, ((String) ((JSONObject) existingJsonArray.get(j)).get(STRID)), tmpCurStrid,
                  ((String) ((JSONObject) existingJsonArray.get(j)).get(STRID)).equals(tmpCurStrid)));
              if (((String) ((JSONObject) existingJsonArray.get(j)).get(STRID)).equals(tmpCurStrid)) {
                tmpExistingIdx = j;
                break;
              }
            }

            if (tmpExistingIdx != -1) {
              // overwrite current statistics if existing
              System.out.println(String.format("[INFO][StatisticsHelper] STRID %s exists, " +
                  "which will be overwritten at index %d!", tmpCurStrid, tmpExistingIdx));
              existingJsonArray.set(tmpExistingIdx, tmpCurJsonobj);
            } else {
              // insert current statistics into correct position if not existing
              int tmpRotationIdx = GlobalConfig.getCurRotationStatisticsIdx();
              System.out.println(String.format("[INFO][StatisticsHelper] STRID %s does NOT exist, " +
                  "which will be inserted at index %d!", tmpCurStrid, tmpRotationIdx));
              existingJsonArray.add(tmpRotationIdx, tmpCurJsonobj);
            }
          }
        } else {
          // directly merge current statistics into existing statistics if NOT single rotation mode
          for (int i = 0; i < statisticsJsonArray.size(); i++) { // NOTE: size must be 1
            existingJsonArray.add(statisticsJsonArray.get(i));
          }
        }

        // Dump merged statistics into disk
        System.out.println(String.format("[INFO][StatisticsHelper] merge and dump static statistics into %s", statisticsFilepath));
        Writer writer = new FileWriter(statisticsFile);
        existingJsonArray.writeJSONString(writer);
        writer.close();
        System.out.flush();
      } else if (GlobalConfig.isDynamicPattern()) {
        // delete existing statistics file
        String statisticsFilepath;
        if (GlobalConfig.isTestingLatency()) {
          statisticsFilepath = IOHelper.getDynamicStatisticsFilepath(
            GlobalConfig.getWorkloadName(), GlobalConfig.getDatabase()+"latency",
            GlobalConfig.getDynamicRulePrefix(), GlobalConfig.getClientPhysicalIdx());
        } else {
          statisticsFilepath = IOHelper.getDynamicStatisticsFilepath(
            GlobalConfig.getWorkloadName(), GlobalConfig.getDatabase(),
            GlobalConfig.getDynamicRulePrefix(), GlobalConfig.getClientPhysicalIdx());
        }
        File statisticsFile = new File(statisticsFilepath);
        if (statisticsFile.exists()) {
          System.out.println(String.format("[WARN][StatisticsHelper] %s already exists -> delete...", statisticsFilepath));
          statisticsFile.delete();
        }

        // Dump per-second statistics in the newly-created file
        System.out.println(String.format("[INFO][StatisticsHelper] dump dynamic statistics into %s", statisticsFilepath));
        Writer writer = new FileWriter(statisticsFile);
        statisticsJsonArray.writeJSONString(writer);
        writer.close();
      } else {
        System.out.println(String.format("[ERROR] methodid %d w/ workloadmode %d should not invoke StatisticsHelper",
            GlobalConfig.getCurmethodId(), GlobalConfig.getWorkloadMode()));
        System.exit(-1);
      }
    } catch (Exception e) {
      System.out.println(e.getMessage());
      e.printStackTrace();
      System.exit(-1);
    }
  }

  private static JSONArray array2jsonarray(int[] intArray) {
    JSONArray tmpJsonArray = new JSONArray();
    for (int i = 0; i < intArray.length; i++) {
      tmpJsonArray.add(intArray[i]);
    }
    return tmpJsonArray;
  }

  private static JSONArray array2jsonarray(long[] longArray) {
    JSONArray tmpJsonArray = new JSONArray();
    for (int i = 0; i < longArray.length; i++) {
      tmpJsonArray.add(longArray[i]);
    }
    return tmpJsonArray;
  }

  private static JSONArray array2jsonarray(long[][] longArray) {
    JSONArray tmpJsonArray = new JSONArray();
    for (int i = 0; i < longArray.length; i++) {
      JSONArray tmpJsonSubArray = new JSONArray();
      for (int j = 0; j < longArray[i].length; j++) {
        tmpJsonSubArray.add(longArray[i][j]);
      }
      tmpJsonArray.add(tmpJsonSubArray);
    }
    return tmpJsonArray;
  }
}
