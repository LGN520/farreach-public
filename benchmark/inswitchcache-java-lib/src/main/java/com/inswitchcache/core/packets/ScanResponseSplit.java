package com.inswitchcache.core.packets;

import com.inswitchcache.core.*;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * Scan response Packet structure.
 */
public class ScanResponseSplit<T extends Key, V extends Value> extends Packet<T> {
  private T endKey;
  private short curScanidx;
  private short maxScannum;
  private short curScanswitchidx; // ONLY for distributed setting
  private short maxScanswitchnum; // ONLY for distributed setting
  protected short nodeIdxForEval;
  protected int snapshotId;
  protected int pairnum;
  protected ArrayList<Pair> pairs;

  public ScanResponseSplit(int methodid, T key, T endKey, short curScanidx, short maxScannum,
                           short nodeIdxForEval, int snapshotId, int pairnum, ArrayList<Pair> pairs) {
    super(methodid, PacketType.SCANRES_SPLIT, key);

    MyUtils.myAssert(isSingleswitch(methodid));
    this.endKey = endKey;
    this.curScanidx = curScanidx;
    this.maxScannum = maxScannum;

    this.nodeIdxForEval = nodeIdxForEval;
    this.snapshotId = snapshotId;
    this.pairnum = pairnum;
    MyUtils.myAssert(snapshotId >= 0);
    MyUtils.myAssert(pairnum == pairs.size());
    this.pairs = new ArrayList<>();
    for (int i = 0; i < pairs.size(); i++) {
      this.pairs.add(pairs.get(i));
    }
  }

  public ScanResponseSplit(int methodid, T key, T endKey, short curScanidx, short maxScannum,
                           short curScanswitchidx, short maxScanswitchnum,
                           short nodeIdxForEval, int snapshotId, int pairnum, ArrayList<Pair> pairs) {
    super(methodid, PacketType.SCANRES_SPLIT, key);

    MyUtils.myAssert(!isSingleswitch(methodid));
    this.endKey = endKey;
    this.curScanidx = curScanidx;
    this.maxScannum = maxScannum;
    this.curScanswitchidx = curScanswitchidx;
    this.maxScanswitchnum = maxScanswitchnum;

    this.nodeIdxForEval = nodeIdxForEval;
    this.snapshotId = snapshotId;
    this.pairnum = pairnum;
    MyUtils.myAssert(snapshotId >= 0);
    MyUtils.myAssert(pairnum == pairs.size());
    this.pairs = new ArrayList<>();
    for (int i = 0; i < pairs.size(); i++) {
      this.pairs.add(pairs.get(i));
    }
  }

  public ScanResponseSplit(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.deserialize(data, recvSize);
    MyUtils.myAssert(this.type == PacketType.SCANRES_SPLIT.getType());
    MyUtils.myAssert(this.snapshotId >= 0);
  }

  protected int size() {
    int size = getOphdrsize(this.methodid) + Key.size() + getSplitPrevBytes(this.methodid) + SIZEOF_UINT16 + SIZEOF_UINT16;
    if (!isSingleswitch(this.methodid)) {
      size += (SIZEOF_UINT16 + SIZEOF_UINT16);
    }
    return size;
  }

  public int serialize(byte[] data, int maxSize) {
    System.out.println("[ERROR][ScanResponseSplit] not support serialize now!");
    System.exit(-1);
    return -1;
  }

  protected void deserialize(byte[] data, int recvSize) {
    int size = size();
    MyUtils.myAssert(size <= recvSize);

    int tmpoff = 0;
    int tmpOphdrsize = deserializeOphdr(data, tmpoff, recvSize);
    tmpoff += tmpOphdrsize;

    int tmpEndkeySize = this.endKey.deserialize(data, tmpoff, recvSize);
    tmpoff += tmpEndkeySize;

    tmpoff += getSplitPrevBytes(this.methodid);

    int tmpCurScanidxSize = SIZEOF_UINT16;
    byte[] tmpCurScanidxBytes = Arrays.copyOfRange(data, tmpoff, tmpoff + tmpCurScanidxSize);
    MyUtils.myAssert(tmpCurScanidxBytes.length == tmpCurScanidxSize);
    this.curScanidx = EndianConverter.ntohs(tmpCurScanidxBytes);
    tmpoff += tmpCurScanidxSize;

    int tmpMaxScannumSize = SIZEOF_UINT16;
    byte[] tmpMaxScannumBytes = Arrays.copyOfRange(data, tmpoff, tmpoff + tmpMaxScannumSize);
    MyUtils.myAssert(tmpMaxScannumBytes.length == tmpMaxScannumSize);
    this.maxScannum = EndianConverter.ntohs(tmpMaxScannumBytes);
    tmpoff += tmpMaxScannumSize;

    if (!isSingleswitch(this.methodid)) {
      int tmpCurScanswitchidxSize = SIZEOF_UINT16;
      byte[] tmpCurScanswitchidxBytes = Arrays.copyOfRange(data, tmpoff, tmpoff + tmpCurScanswitchidxSize);
      MyUtils.myAssert(tmpCurScanswitchidxBytes.length == tmpCurScanswitchidxSize);
      this.curScanswitchidx = EndianConverter.ntohs(tmpCurScanswitchidxBytes);
      tmpoff += tmpCurScanswitchidxSize;

      int tmpMaxScanswitchnumSize = SIZEOF_UINT16;
      byte[] tmpMaxScanswitchnumBytes = Arrays.copyOfRange(data, tmpoff, tmpoff + tmpMaxScanswitchnumSize);
      MyUtils.myAssert(tmpMaxScanswitchnumBytes.length == tmpMaxScanswitchnumSize);
      this.maxScanswitchnum = EndianConverter.ntohs(tmpMaxScanswitchnumBytes);
      tmpoff += tmpMaxScanswitchnumSize;
    }

    return;
  }

  // Getter and setter

  public short nodeIdxForEval() {
    return this.nodeIdxForEval;
  }

  public int snapshotId() {
    return this.snapshotId;
  }

  public int pairnum() {
    return this.pairnum;
  }

  public ArrayList<Pair> pairs() {
    return this.pairs;
  }

  public int dynamicSerialize() {
    return 0;
  }

  public static int getFraghdrSize() {
    return 0;
  }

  public static int getSrcNumOff() {
    return 0;
  }

  public static int getSrcNumLen() {
    return 0;
  }

  public static boolean getSrcNumConversino() {
    return false;
  }

  public static int getSrcIdOff() {
    return 0;
  }

  public static int getSrcIdLen() {
    return 0;
  }

  public static boolean getSrcIdConversion() {
    return false;
  }
}
