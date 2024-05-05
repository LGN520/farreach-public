package com.inswitchcache.db;

import java.util.ArrayList;

import com.inswitchcache.InswitchCacheClient;
import com.inswitchcache.core.DynamicArray;
import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.Key;
import com.inswitchcache.core.MyUtils;
import com.inswitchcache.core.SocketHelper;
import com.inswitchcache.core.Value;
import com.inswitchcache.core.packets.DelRequest;
import com.inswitchcache.core.packets.DelResponse;
import com.inswitchcache.core.packets.DelResponseSeq;
import com.inswitchcache.core.packets.GetRequest;
import com.inswitchcache.core.packets.GetResponse;
import com.inswitchcache.core.packets.GetResponseLargeValue;
import com.inswitchcache.core.packets.GetResponseLargeValueSeq;
import com.inswitchcache.core.packets.GetResponseSeq;
import com.inswitchcache.core.packets.LoadAck;
import com.inswitchcache.core.packets.LoadRequest;
import com.inswitchcache.core.packets.Packet;
import com.inswitchcache.core.packets.PacketType;
import com.inswitchcache.core.packets.PutRequest;
import com.inswitchcache.core.packets.PutRequestLargeValue;
import com.inswitchcache.core.packets.PutResponse;
import com.inswitchcache.core.packets.PutResponseSeq;
import com.inswitchcache.core.packets.ScanRequest;
import com.inswitchcache.core.packets.ScanResponseSplit;

/**
 * Helper class providing an interface for all database methods
 * NOTE: each sub-thread of logical client should have a DbUdpNative instance
 * NOTE: NOT rely on any structure of YCSB!
 * NOTE: DbUdpNative should NOT use GlobalConfig, which has the information
 * of logical clients
 */
public class DbUdpNative {
  // for UDP socket programming
  private byte[] buf = null;
  private int bufsize = 0;
  private DynamicArray dynamicBuf;
  private int clientUdpSock = -1;

  // for statistics related with UDP socket
  private int unmatchedCnt = 0;
  private int timeoutCnt = 0;

  // for PUTREQ_LARGEVALUE
  private int fragseq = 0;

  public DbUdpNative(int bufsize, int maxcapacity, int timeoutSec, int timeoutUsec, int recvBufsize) {
    this.bufsize = bufsize;
    this.buf = new byte[bufsize];
    this.dynamicBuf = new DynamicArray(bufsize, maxcapacity);

    clientUdpSock = SocketHelper.createUdpsockWithport(true, "logicalclient", timeoutSec, timeoutUsec, recvBufsize,1);
  }

  public DbUdpNativeResult getNative(int methodid, Key key, String serverIp, short serverPort) {
    DbUdpNativeResult result = new DbUdpNativeResult();

    // Timeout-and-retry
    while (true) {
      // Prepare packet
      GetRequest<Key> req = new GetRequest<Key>(methodid, key);
      int reqSize = req.serialize(buf, bufsize);

      // Send packet
      SocketHelper.udpsendto(clientUdpSock, buf, reqSize, serverIp,
          serverPort, "farreach.client.read");

      // Check recv
      int recvSize = -1;
      while (true) {
        // filter unmatched response
        // UdpAddr svrAddr = new UdpAddr(); // NOTE: NOT used
        // recvSize = SocketHelper.udprecvlargeipfrag(methodid, clientUdpSock,
        // dynamicBuf, svrAddr, "farreach.client.read");
        dynamicBuf.clear();
        recvSize = SocketHelper.udprecvlargeipfrag(methodid, clientUdpSock, dynamicBuf, "logicalclient.get");

        if (recvSize == -1) { // timeout
          break;
        } else {
          short tmpPktTypeForGetReq = Packet.getPacketType(dynamicBuf.array(), 0, recvSize);
          if (methodid == GlobalConfig.FARREACH_ID) {
            if (tmpPktTypeForGetReq != PacketType.GETRES_SEQ.getType() &&
                tmpPktTypeForGetReq != PacketType.GETRES_LARGEVALUE_SEQ.getType()) {
              unmatchedCnt += 1;
              continue;
            } else {
              Key tmpKeyForGetReq;
              Value tmpValueForGetReq;
              int tmpSeqForGetReq = 0;
              boolean tmpStatForGetReq = false;
              short tmpNodeIdxForEvalGetReq = 0;
              if (tmpPktTypeForGetReq == PacketType.GETRES_SEQ.getType()) {
                GetResponseSeq<Key, Value> rsp = new GetResponseSeq<>(methodid, dynamicBuf.array(), recvSize);
                tmpKeyForGetReq = rsp.key();
                tmpValueForGetReq = rsp.val();
                tmpSeqForGetReq = rsp.seq();
                tmpStatForGetReq = rsp.stat();
                tmpNodeIdxForEvalGetReq = rsp.nodeIdxForEval();
              } else {
                GetResponseLargeValueSeq<Key, Value> rsp = new GetResponseLargeValueSeq<>(methodid, dynamicBuf.array(),
                    recvSize);
                tmpKeyForGetReq = rsp.key();
                tmpValueForGetReq = rsp.val();
                tmpSeqForGetReq = rsp.seq();
                tmpStatForGetReq = rsp.stat();
                tmpNodeIdxForEvalGetReq = rsp.nodeIdxForEval();
              }
              if (Key.compareKeys(tmpKeyForGetReq, key) != 0) {
                unmatchedCnt += 1;
                continue;
              } else {
                result.init(dynamicBuf.array(), tmpKeyForGetReq, tmpValueForGetReq, tmpSeqForGetReq, tmpStatForGetReq,
                    tmpNodeIdxForEvalGetReq);
                break;
              } // key match or not
            } // type match or not
          } else {
            if (tmpPktTypeForGetReq != PacketType.GETRES.getType() &&
                tmpPktTypeForGetReq != PacketType.GETRES_LARGEVALUE.getType()) {
              unmatchedCnt += 1;
              continue;
            } else {
              Key tmpKeyForGetReq;
              Value tmpValueForGetReq;
              boolean tmpStatForGetReq = false;
              short tmpNodeIdxForEvalGetReq = 0;
              if (tmpPktTypeForGetReq == PacketType.GETRES.getType()) {
                GetResponse<Key, Value> rsp = new GetResponse<>(methodid, dynamicBuf.array(), recvSize);
                tmpKeyForGetReq = rsp.key();
                tmpValueForGetReq = rsp.val();
                tmpStatForGetReq = rsp.stat();
                tmpNodeIdxForEvalGetReq = rsp.nodeIdxForEval();
              } else {
                GetResponseLargeValue<Key, Value> rsp = new GetResponseLargeValue<>(methodid, dynamicBuf.array(),
                    recvSize);
                tmpKeyForGetReq = rsp.key();
                tmpValueForGetReq = rsp.val();
                tmpStatForGetReq = rsp.stat();
                tmpNodeIdxForEvalGetReq = rsp.nodeIdxForEval();
              }
              if (Key.compareKeys(tmpKeyForGetReq, key) != 0) {
                unmatchedCnt += 1;
                continue;
              } else {
                result.init(dynamicBuf.array(), tmpKeyForGetReq, tmpValueForGetReq, 0, tmpStatForGetReq,
                    tmpNodeIdxForEvalGetReq);
                break;
              } // key match or not
            } // type match or not
          } // farreach or not
        } // timeout or not
      } // end of while true
      if (recvSize == -1) {
        int serveridx = key.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
            GlobalConfig.getMaxServerTotalLogicalNum());
        System.out.println(String.format("[WARN][DbUdpNative] GETREQ of key %s of serveridx %d/%d timeout!",
            key.toString(), serveridx, GlobalConfig.getMaxServerTotalLogicalNum())); // TMPDEBUG
        timeoutCnt += 1;
        if (InswitchCacheClient.isStop()) {
          System.out
              .println("[WARN][DbUdpNative] detect InswitchCacheClient.isStop = true, stop retry for the timeout!");
          break;
        } else {
          continue;
        }
      }
      break;
    }
    return result;
  }

  public DbUdpNativeResult putNative(int methodid, Key key, Value value, short globalClientLogicalIdx, String serverIp,
      short serverPort) {
    DbUdpNativeResult result = new DbUdpNativeResult();

    // Timeout-and-retry
    while (true) {
      // Prepare packet
      boolean tmpIsLarge = false;
      int reqSize = 0;
      if (value.getValData() == null || value.getValData().length <= Value.getSwitchMaxValLen()) {
        tmpIsLarge = false;
        PutRequest<Key, Value> req = new PutRequest<Key, Value>(methodid, key, value);
        reqSize = req.serialize(buf, bufsize);
      } else {
        tmpIsLarge = true;
        PutRequestLargeValue<Key, Value> req = new PutRequestLargeValue<Key, Value>(methodid, key, value,
            globalClientLogicalIdx, this.fragseq);
        this.fragseq += 1;
        this.dynamicBuf.clear();
        reqSize = req.dynamicSerialize(this.dynamicBuf);
      }

      // Send packet
      if (!tmpIsLarge) {
        SocketHelper.udpsendto(clientUdpSock, buf, reqSize, serverIp, serverPort,
            "logicalclient.put");
      } else {
        SocketHelper.udpsendlargeipfrag(clientUdpSock, dynamicBuf.array(), reqSize, serverIp, serverPort,
            "logicalclient.put", PutRequestLargeValue.getFraghdrSize(methodid));
      }

      // check recv
      int recvSize = -1;
      while (true) {
        // filter unmatched response
        // UdpAddr svrAddr = new UdpAddr();
        // recvSize = SocketHelper.udprecvfrom(clientUdpSock, recvRespBuf,
        // GlobalConfig.MAX_BUFFER_SIZE, svrAddr,
        // "farreach.client.update");
        recvSize = SocketHelper.udprecvfrom(clientUdpSock, buf, bufsize, "logicalclient.put");
        if (recvSize == -1) { // timeout
          break;
        } else {
          short tmpPktTypeForPutReq = Packet.getPacketType(buf, 0, recvSize);
          if (methodid == GlobalConfig.FARREACH_ID) {
            if (tmpPktTypeForPutReq != PacketType.PUTRES_SEQ.getType()) {
              this.unmatchedCnt += 1;
              continue;
            } else {
              PutResponseSeq<Key> rsp = new PutResponseSeq<Key>(methodid, buf, recvSize);
              if (Key.compareKeys(rsp.key(), key) != 0) {
                // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
                // + ", key = " + key
                // + ": unmatched key for update request and response.");
                this.unmatchedCnt += 1;
                continue;
              } else {
                // break to update and send the next packet
                result.init(buf, rsp.key(), value, rsp.seq(), true, rsp.nodeIdxForEval());
                break;
              } // key match or not
            } // type match or not
          } else {
            if (tmpPktTypeForPutReq != PacketType.PUTRES.getType()) {
              this.unmatchedCnt += 1;
              continue;
            } else {
              PutResponse<Key> rsp = new PutResponse<Key>(methodid, buf, recvSize);
              if (Key.compareKeys(rsp.key(), key) != 0) {
                // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
                // + ", key = " + key
                // + ": unmatched key for update request and response.");
                this.unmatchedCnt += 1;
                continue;
              } else {
                // break to update and send the next packet
                result.init(buf, rsp.key(), value, 0, true, rsp.nodeIdxForEval());
                break;
              } // key match or not
            } // type match or not
          } // farreach or not
        } // timeout or not
      } // end of while true
      if (recvSize == -1) {
        int tmpVallen = value.getValData().length;
        int serveridx = key.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
            GlobalConfig.getMaxServerTotalLogicalNum());
        if (tmpVallen <= Value.getSwitchMaxValLen()) {
          System.out
              .println(String.format("[WARNING][DbUdpNative] PUTREQ of key %s w/ vallen %d of serveridx %d/%d timeout!",
                  key.toString(), tmpVallen, serveridx, GlobalConfig.getMaxServerTotalLogicalNum())); // TMPDEBUG
        } else {
          System.out.println(String.format(
              "[WARNING][DbUdpNative] PUTREQ_LARGEVALUE of key %s w/ vallen %d of serveridx %d/%d timeout!",
              key.toString(), tmpVallen, serveridx, GlobalConfig.getMaxServerTotalLogicalNum())); // TMPDEBUG
        }
        timeoutCnt += 1;
        if (InswitchCacheClient.isStop()) {
          System.out
              .println("[WARN][DbUdpNative] detect InswitchCacheClient.isStop = true, stop retry for the timeout!");
          break;
        } else {
          continue;
        }
      }
      break;
    }
    return result;
  }

  public DbUdpNativeResult delNative(int methodid, Key key, String serverIp, short serverPort) {
    DbUdpNativeResult result = new DbUdpNativeResult();

    // Timeout-and-retry
    while (true) {
      // prepare packet
      DelRequest<Key> req = new DelRequest<Key>(methodid, key);
      // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
      // + ", key = " + key);
      int reqSize = (int) req.serialize(buf, bufsize);

      // send packet
      SocketHelper.udpsendto(clientUdpSock, buf, reqSize, serverIp,
          serverPort, "logicalclient.del");

      // check recv
      int recvSize = -1;
      while (true) {
        // filter unmatched response
        // UdpAddr svrAddr = new UdpAddr();
        // recvSize = SocketHelper.udprecvfrom(clientUdpSock, recvRespBuf,
        // GlobalConfig.MAX_BUFFER_SIZE, svrAddr,
        // "farreach.client.delete");
        recvSize = SocketHelper.udprecvfrom(clientUdpSock, buf, bufsize, "logicalclient.del");
        if (recvSize == -1) { // timeout
          break;
        } else {
          short tmpPktTypeForDelReq = Packet.getPacketType(buf, 0, recvSize);
          if (methodid == GlobalConfig.FARREACH_ID) {
            if (tmpPktTypeForDelReq != PacketType.DELRES_SEQ.getType()) {
              this.unmatchedCnt += 1;
              continue;
            } else {
              DelResponseSeq<Key> rsp = new DelResponseSeq<Key>(methodid, buf, recvSize);
              if (Key.compareKeys(rsp.key(), key) != 0) {
                // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
                // + ", key = " + key
                // + ": unmatched key for delete request and response.");
                this.unmatchedCnt += 1;
                continue;
              } else {
                // break to update and send the next packet
                result.init(buf, rsp.key(), new Value(), rsp.seq(), false, rsp.nodeIdxForEval());
                break;
              } // key match or not
            } // type match or not
          } else {
            if (tmpPktTypeForDelReq != PacketType.DELRES.getType()) {
              this.unmatchedCnt += 1;
              continue;
            } else {
              DelResponse<Key> rsp = new DelResponse<Key>(methodid, buf, recvSize);
              if (Key.compareKeys(rsp.key(), key) != 0) {
                // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
                // + ", key = " + key
                // + ": unmatched key for delete request and response.");
                this.unmatchedCnt += 1;
                continue;
              } else {
                // break to update and send the next packet
                result.init(buf, rsp.key(), new Value(), 0, false, rsp.nodeIdxForEval());
                break;
              } // key match or not
            } // type match or not
          } // farreach or not
        } // timeout or not
      } // end of while true
      if (recvSize == -1) {
        int serveridx = key.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
            GlobalConfig.getMaxServerTotalLogicalNum());
        System.out.println(String.format("[WARNING][DbUdpNative] DELREQ of key %s of serveridx %d/%d timeout!",
            key.toString(), serveridx, GlobalConfig.getMaxServerTotalLogicalNum())); // TMPDEBUG
        timeoutCnt += 1;
        if (InswitchCacheClient.isStop()) {
          System.out
              .println("[WARN][DbUdpNative] detect InswitchCacheClient.isStop = true, stop retry for the timeout!");
          break;
        } else {
          continue;
        }
      }
      break;
    }
    return result;
  }

  public DbUdpNativeResult loadNative(int methodid, Key key, Value value, short globalClientLogicalIdx, String serverIp,
      short serverPort) {
    DbUdpNativeResult result = new DbUdpNativeResult();

    // Timeout-and-retry
    while (true) {
      // Prepare packet
      int reqSize = 0;
      LoadRequest<Key, Value> req = new LoadRequest<Key, Value>(methodid, key, value, globalClientLogicalIdx,
          this.fragseq);
      this.fragseq += 1;
      this.dynamicBuf.clear();
      reqSize = req.dynamicSerialize(this.dynamicBuf);

      // TMPDEBUG
      // System.out.println(String.format("[LOADREQ] val size %d %d: %s",
      // req.val().getValData().length,
      // req.val().toString().length(), req.val().toString()));
      // System.out.println(MyUtils.dynamicArrayToHexString(dynamicBuf));
      // System.out.flush();

      // Send packet
      SocketHelper.udpsendlargeipfrag(clientUdpSock, dynamicBuf.array(), reqSize, serverIp, serverPort,
          "logicalclient.load", LoadRequest.getFraghdrSize(methodid));

      // check recv
      int recvSize = -1;
      while (true) {
        // filter unmatched response
        // UdpAddr svrAddr = new UdpAddr();
        // recvSize = SocketHelper.udprecvfrom(clientUdpSock, recvRespBuf,
        // GlobalConfig.MAX_BUFFER_SIZE, svrAddr,
        // "farreach.client.update");
        recvSize = SocketHelper.udprecvfrom(clientUdpSock, buf, bufsize, "logicalclient.load");
        if (recvSize == -1) { // timeout
          break;
        } else {
          short tmpPktTypeForPutReq = Packet.getPacketType(buf, 0, recvSize);
          if (tmpPktTypeForPutReq != PacketType.LOADACK.getType()) {
            this.unmatchedCnt += 1;
            continue;
          } else {
            LoadAck<Key> rsp = new LoadAck<Key>(methodid, buf, recvSize);
            if (Key.compareKeys(rsp.key(), key) != 0) {
              // System.out.println("[INFO][FarreachClient] client " + localLogicalClientIndex
              // + ", key = " + key
              // + ": unmatched key for update request and response.");
              this.unmatchedCnt += 1;
              continue;
            } else {
              // break to update and send the next packet
              result.init(buf, rsp.key(), value, 0, false, (short) 0);
              break;
            }
          }
        }
      }
      if (recvSize == -1) {
        int tmpVallen = value.getValData().length;
        int serveridx = key.getHashPartitionIdx(GlobalConfig.getSwitchPartitionCount(),
            GlobalConfig.getMaxServerTotalLogicalNum());
        System.out
            .println(String.format("[WARNING][DbUdpNative] LOADREQ of key %s w/ vallen %d of serveridx %d/%d timeout!",
                key.toString(), tmpVallen, serveridx, GlobalConfig.getMaxServerTotalLogicalNum())); // TMPDEBUG
        timeoutCnt += 1;
        if (InswitchCacheClient.isStop()) {
          System.out
              .println("[WARN][DbUdpNative] detect InswitchCacheClient.isStop = true, stop retry for the timeout!");
          break;
        } else {
          continue;
        }
      }
      break;
    }
    return result;
  }

  public DbUdpNativeResult scanNative(int methodid, Key startKey, Key endKey, String serverIp, short serverPort) {
    DbUdpNativeResult result = new DbUdpNativeResult();

    // Timeout-and-retry
    while (true) {
      // Serialize packet
      ScanRequest<Key> req = new ScanRequest<Key>(methodid, startKey, endKey);
      int reqSize = req.serialize(buf, bufsize);

      // send packet
      SocketHelper.udpsendto(clientUdpSock, buf, reqSize, serverIp,
          serverPort, "logicalclient.scan");

      // check recv
      int recvSize = -1;
      while (true) {
        // filter unmatched response
        dynamicBuf.clear();
        recvSize = SocketHelper.udprecvlargeipfragMultisrc(methodid, clientUdpSock, dynamicBuf, "logicalclient.load",
            startKey);
        if (recvSize == -1) { // timeout
          break;
        } else {
          ArrayList<DynamicArray> dynamicbufs = SocketHelper.decodeDynamicbufs(dynamicBuf);
          MyUtils.myAssert(dynamicbufs.size() > 0);
          ScanResponseSplit rsp = new ScanResponseSplit<Key, Value>(methodid, dynamicbufs.get(0).array(),
              dynamicbufs.get(0).size());
          result.init(dynamicBuf.array(), rsp.key(), new Value(), 0, false, rsp.nodeIdxForEval());
          break;
        }
      }
      if (recvSize == -1) {
        System.out.println("[WARNING][DbUdpNative] timeout!"); // TMPDEBUG
        this.timeoutCnt += 1;
        if (InswitchCacheClient.isStop()) {
          System.out
              .println("[WARN][DbUdpNative] detect InswitchCacheClient.isStop = true, stop retry for the timeout!");
          break;
        } else {
          continue;
        }
      }
      break;
    }
    return result;
  }

  public int getUnmatchedCnt() {
    return unmatchedCnt;
  }

  public void setUnmatchedCnt(int unmatchedCnt) {
    this.unmatchedCnt = unmatchedCnt;
  }

  public int getTimeoutCnt() {
    return timeoutCnt;
  }

  public void setTimeoutCnt(int timeoutCnt) {
    this.timeoutCnt = timeoutCnt;
  }

}
