package com.inswitchcache.core;

import java.util.HashMap;
import java.util.Map;

public class OutbandPacketFormat {
  // FORMAT: <int num>, <Key k0, int seq0>, <Key k1, int seq1>, ...
  public static HashMap<Key, Integer> deserializeUpstreamBackupNotification(byte[] buf, int maxSize) {
    HashMap<Key, Integer> result = new HashMap<Key, Integer>();

    int offset = 0;

    final int numSize = 4;
    final int keySize = Key.size();
    final int seqSize = 4;

    // num
    MyUtils.myAssert(maxSize >= (offset + numSize));
    byte[] numBytes = new byte[numSize];
    System.arraycopy(buf, offset, numBytes, 0, numSize);
    int tmpnum = EndianConverter.ntohi(numBytes);
    offset += numSize;

    // each key-seq pair
    for (int i = 0; i < tmpnum; i++) {
      MyUtils.myAssert(maxSize >= (offset + keySize));
      Key tmpkey = new Key();
      tmpkey.deserialize(buf, offset, maxSize);
      offset += keySize;

      MyUtils.myAssert(maxSize >= (offset + seqSize));
      byte[] seqBytes = new byte[seqSize];
      System.arraycopy(buf, offset, seqBytes, 0, seqSize);
      int tmpseq = EndianConverter.ntohi(seqBytes);
      offset += seqSize;

      result.put(tmpkey, tmpseq);
    }

    return result;
  }

  // FORMAT: <int num>, <Key k0, Val v0, int seq0, bool stat0>, <Key k1, Val v1,
  // int seq1, bool stat1>, ...
  public static int dynamicSerializeUpstreamBackups(DynamicArray dynamicBuf,
      HashMap<Key, UpstreamBackup> keyBackupMap) {
    int offset = 0;

    // num
    int tmpnum = keyBackupMap.size();
    byte[] numBytes = EndianConverter.htoni(tmpnum);
    dynamicBuf.dynamicMemcpy(offset, numBytes, numBytes.length);
    offset += numBytes.length;

    // each upstream backup
    for (Map.Entry<Key, UpstreamBackup> entry : keyBackupMap.entrySet()) {
      // key
      int tmpkeySize = entry.getKey().dynamicSerialize(dynamicBuf, offset);
      offset += tmpkeySize;

      // value
      int tmpvalSize = entry.getValue().getValue().dynamicSerialize(dynamicBuf, offset);
      offset += tmpvalSize;

      // seq
      byte[] seqBytes = EndianConverter.htoni(entry.getValue().getSeq());
      dynamicBuf.dynamicMemcpy(offset, seqBytes, seqBytes.length);
      offset += seqBytes.length;

      // stat
      byte[] statByte = { (byte) (entry.getValue().isStat() ? 1 : 0) };
      dynamicBuf.dynamicMemcpy(offset, statByte, statByte.length);
      offset += statByte.length;
    }
    return offset;
  }
}
