/**
 * Copyright (c) 2012 YCSB contributors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You
 * may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 */

/**
 * Farreach client binding for YCSB.
 */

package site.ycsb.db;

import java.util.*;

import site.ycsb.ByteArrayByteIterator;
import site.ycsb.ByteIterator;
import site.ycsb.DBException;
import site.ycsb.RemoteDB;
import site.ycsb.Status;
import site.ycsb.StringByteIterator;

import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.Key;
import com.inswitchcache.core.Value;
import com.inswitchcache.InswitchCacheClient;
import com.inswitchcache.db.DbUdpNative;
import com.inswitchcache.db.DbUdpNativeResult;
import com.inswitchcache.core.MyUtils;
import com.inswitchcache.core.Pair;
import com.inswitchcache.core.DynamicArray;
import com.inswitchcache.core.SocketHelper;
import com.inswitchcache.core.packets.ScanResponseSplit;

/**
 * Farreach Client.
 */
public class FarreachClient extends RemoteDB {
  private DbUdpNative dbInterface;

  @Override
  public void init() throws DBException {
    super.init();

    MyUtils.myAssert(GlobalConfig.getCurmethodId() == GlobalConfig.FARREACH_ID);

    this.dbInterface = new DbUdpNative(GlobalConfig.MAX_BUFFER_SIZE, GlobalConfig.MAX_LARGE_BUFFER_SIZE,
        GlobalConfig.CLIENT_SOCKET_TIMEOUT_SECS, 0, GlobalConfig.UDP_DEFAULT_RCVBUFSIZE);
  }

  @Override
  public void cleanup() throws DBException {
    // System.out.println("[DEBUG][FarreachClient] Client " +
    // localLogicalClientIndex + " cleanup");
    super.cleanup();
  }

  @Override
  public Status read(String table, String key, Set<String> fields, Map<String, ByteIterator> result) {
    // Log information
    // System.out.println("[INFO][FarreachClient] Client " + localLogicalClientIndex
    // + " read request for key " + key);

    // Prepare request key
    Key tmpKey = new Key();
    tmpKey.fromString(key);
    if (isDynamicWorkload) {
      tmpKey = InswitchCacheClient.getDynamicRulemap().trymap(tmpKey);
      // String mappedKey = tmpKey.toString();
      // int periodIdx = InswitchCacheClient.getCurrentDynamicPeriod();
      // InswitchCacheClient.getPeriodKeyKeyRecordsList(periodIdx)
      // .put(key, mappedKey);
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx)
      // .put(key,
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx).getOrDefault(key, 0) +
      // 1);
    }

    // send and recv
    DbUdpNativeResult dbResult = this.dbInterface.getNative(GlobalConfig.getCurmethodId(), tmpKey,
        GlobalConfig.getServerIp(), 
         (short)(GlobalConfig.getServerWorkerPortStart() +
          (short) tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
                  GlobalConfig.getServerTotalLogicalNum())%GlobalConfig.getserverPerServerLogicalNum()));
    try {
      result.put(key, new ByteArrayByteIterator(dbResult.getPktContent()));
    } catch (NullPointerException e) {
      
    }
    InswitchCacheClient.updateLoadStatistics(this.localLogicalClientIndex, dbResult);

    if (result.isEmpty()) {
      return Status.ERROR;
    } else {
      if (dbResult.getNodeIdxForEval() == -1) {
        return Status.CACHE_HIT;
      } else {
        return Status.OK;
      }
    }
  }

  @Override
  public Status scan(String table, String startkey, int recordcount, Set<String> fields,
      Vector<HashMap<String, ByteIterator>> result) {
    // Log information
    // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
    // + ": scan request for starkey "
    // + startkey + " range " + recordcount);

    // Construct keys
    Key startKeyStruct = new Key();
    startKeyStruct.fromString(startkey);
    Key endKeyStruct = startKeyStruct.getEndkey(recordcount);
    // String endKey = endKeyStruct.toString();
    // System.out.println(
    // "[INFO][FarreachClient] client " + localLogicalClientIndex + ", key = " +
    // startkey + ", end key = " + endKey);

    // send and recv
    DbUdpNativeResult dbResult = this.dbInterface.scanNative(GlobalConfig.getCurmethodId(), startKeyStruct,
        endKeyStruct, GlobalConfig.getServerIp(), GlobalConfig.getServerWorkerPortStart());

    ArrayList<DynamicArray> dynamicbufs = SocketHelper.decodeDynamicbufs(dbResult.getPktContent());
    for (int i = 0; i < dynamicbufs.size(); i++) {
      HashMap<String, ByteIterator> tmpmap = new HashMap<>();

      ScanResponseSplit tmprsp = new ScanResponseSplit<Key, Value>(GlobalConfig.getCurmethodId(),
          dynamicbufs.get(i).array(), dynamicbufs.get(i).size());
      ArrayList<Pair> tmppairs = tmprsp.pairs();
      for (int j = 0; j < tmppairs.size(); j++) {
        tmpmap.put(tmppairs.get(j).getKey().toString(),
            new ByteArrayByteIterator(tmppairs.get(j).getSnapshotRecord().getVal().getValData()));
      }

      result.add(tmpmap);
    }

    InswitchCacheClient.updateLoadStatistics(this.localLogicalClientIndex, dbResult);

    return Status.OK;
  }

  @Override
  public Status update(String table, String key, Map<String, ByteIterator> values) {
    // Log information
    // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
    // + ", update request for key = "
    // + entry.getKey() + " value = " + entry.getValue());

    // Check value requirement
    Map<String, String> valueMap = StringByteIterator.getStringMap(values);
    if (valueMap.entrySet().size() > 1) {
      System.err.println("[ERROR][FarreachClient] client " + localLogicalClientIndex + ", key = " + key
          + ": farreach update only accept one update value");
      return Status.BAD_REQUEST;
    }

    // prepare request key and value
    Map.Entry<String, String> entry = valueMap.entrySet().iterator().next();
    Value tmpValue = new Value();
    tmpValue.fromString(entry.getValue());
    Key tmpKey = new Key();
    tmpKey.fromString(key);
    if (isDynamicWorkload) {
      tmpKey = InswitchCacheClient.getDynamicRulemap().trymap(tmpKey);
      // String mappedKey = tmpKey.toString();
      // int periodIdx = InswitchCacheClient.getCurrentDynamicPeriod();
      // InswitchCacheClient.getPeriodKeyKeyRecordsList(periodIdx)
      // .put(key, mappedKey);
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx)
      // .put(key,
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx).getOrDefault(key, 0) +
      // 1);
    }

    // send and recv
    // System.out.println("debug port:"+(short)(GlobalConfig.getServerWorkerPortStart() +
    // (short) tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
    //         GlobalConfig.getServerTotalLogicalNum())%GlobalConfig.getserverPerServerLogicalNum())+"debug ip:" + GlobalConfig.getServerIp());
    DbUdpNativeResult dbResult = this.dbInterface.putNative(GlobalConfig.getCurmethodId(), tmpKey, tmpValue,
        (short) this.globalClientLogicalIndex, GlobalConfig.getServerIp(),  (short)(GlobalConfig.getServerWorkerPortStart() +
          (short) tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
                  GlobalConfig.getServerTotalLogicalNum())%GlobalConfig.getserverPerServerLogicalNum()));

    InswitchCacheClient.updateLoadStatistics(this.localLogicalClientIndex, dbResult);

    if (dbResult.getNodeIdxForEval() == -1) {
      return Status.CACHE_HIT;
    } else {
      return Status.OK;
    }
  }

  @Override
  public Status delete(String table, String key) {
    // Log information
    // System.out.println("[INFO][FarreachClient] Delete request for key " + key);

    // Prepare key
    Key tmpKey = new Key();
    tmpKey.fromString(key);
    if (isDynamicWorkload) {
      tmpKey = InswitchCacheClient.getDynamicRulemap().trymap(tmpKey);
      // String mappedKey = tmpKey.toString();
      // int periodIdx = InswitchCacheClient.getCurrentDynamicPeriod();
      // InswitchCacheClient.getPeriodKeyKeyRecordsList(periodIdx)
      // .put(key, mappedKey);
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx)
      // .put(key,
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx).getOrDefault(key, 0) +
      // 1);
    }

    // send and recv
    DbUdpNativeResult dbResult = this.dbInterface.delNative(GlobalConfig.getCurmethodId(), tmpKey,
        GlobalConfig.getServerIp(),  (short)(GlobalConfig.getServerWorkerPortStart() +
          (short) tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
                  GlobalConfig.getServerTotalLogicalNum())%GlobalConfig.getserverPerServerLogicalNum()));

    InswitchCacheClient.updateLoadStatistics(this.localLogicalClientIndex, dbResult);

    if (dbResult.getNodeIdxForEval() == -1) {
      return Status.CACHE_HIT;
    } else {
      return Status.OK;
    }
  }

  @Override
  public Status insert(String table, String key, Map<String, ByteIterator> values) {
    // Log information
    // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
    // + ", update request for key = "
    // + entry.getKey() + " value = " + entry.getValue());

    // Check value requirement
    Map<String, String> valueMap = StringByteIterator.getStringMap(values);
    if (valueMap.entrySet().size() > 1) {
      // System.err.println("[ERROR][FarreachClient] client " +
      // localLogicalClientIndex + ", key = " + key
      // + ": farreach update only accept one update value");
      return Status.BAD_REQUEST;
    }

    // prepare request key and value
    Map.Entry<String, String> entry = valueMap.entrySet().iterator().next();
    Value tmpValue = new Value();
    tmpValue.fromString(entry.getValue());
    Key tmpKey = new Key();
    tmpKey.fromString(key);
    if (isDynamicWorkload) {
      tmpKey = InswitchCacheClient.getDynamicRulemap().trymap(tmpKey);
      // String mappedKey = tmpKey.toString();
      // int periodIdx = InswitchCacheClient.getCurrentDynamicPeriod();
      // InswitchCacheClient.getPeriodKeyKeyRecordsList(periodIdx)
      // .put(key, mappedKey);
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx)
      // .put(key,
      // InswitchCacheClient.getPeriodKeyKeyFreqList(periodIdx).getOrDefault(key, 0) +
      // 1);
    }

    // send and recv
    DbUdpNativeResult dbResult = this.dbInterface.putNative(GlobalConfig.getCurmethodId(), tmpKey, tmpValue,
        (short) this.globalClientLogicalIndex, GlobalConfig.getServerIp(),  (short)(GlobalConfig.getServerWorkerPortStart() +
          (short) tmpKey.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
                  GlobalConfig.getServerTotalLogicalNum())%GlobalConfig.getserverPerServerLogicalNum()));

    InswitchCacheClient.updateLoadStatistics(this.localLogicalClientIndex, dbResult);

    if (dbResult.getNodeIdxForEval() == -1) {
      return Status.CACHE_HIT;
    } else {
      return Status.OK;
    }
  }
}
