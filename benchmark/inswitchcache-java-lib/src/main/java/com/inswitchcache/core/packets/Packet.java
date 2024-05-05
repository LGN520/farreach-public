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
 * Farreach client packets implementation.
 */

package com.inswitchcache.core.packets;

import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.Key;
import com.inswitchcache.core.EndianConverter;
import com.inswitchcache.core.DynamicArray;
import com.inswitchcache.core.MyUtils;

import java.util.Arrays;

/**
 * Farreach request Packet structure.
 */
public abstract class Packet<T extends Key> {
  public static final int SIZEOF_UINT32 = 4;
  public static final int SIZEOF_UINT16 = 2;
  public static final int SIZEOF_UINT8 = 1;

  protected int methodid;
  protected short type;
  protected T key;
  // for distfarreach/distnocache
  protected short globalSwitchidx;
  // for distcache
  protected short spineSwitchidx;
  protected short leafSwitchidx;

  public static int getStatPaddingBytes(int methodid) {
    int result = 0;
    switch (methodid) {
    case GlobalConfig.FARREACH_ID:
      result = 1;
      break;
    case GlobalConfig.NOCACHE_ID:
      result = 1;
      break;
    case GlobalConfig.NETCACHE_ID:
      result = 1;
      break;
    case GlobalConfig.DISTFARREACH_ID:
      result = 1;
      break;
    case GlobalConfig.DISTNOCACHE_ID:
      result = 1;
      break;
    case GlobalConfig.DISTCACHE_ID:
      result = 1;
      break;
    default:
      System.out.println(String.format("Invalid method id: %d", methodid));
      System.exit(-1);
    }
    return result;
  }

  public static int getSplitPrevBytes(int methodid) {
    int result = 0;
    switch (methodid) {
    case GlobalConfig.FARREACH_ID:
      result = 3;
      break;
    case GlobalConfig.NOCACHE_ID:
      result = 3;
      break;
    case GlobalConfig.NETCACHE_ID:
      result = 3;
      break;
    case GlobalConfig.DISTFARREACH_ID:
      result = 4;
      break;
    case GlobalConfig.DISTNOCACHE_ID:
      result = 4;
      break;
    case GlobalConfig.DISTCACHE_ID:
      result = 4;
      break;
    default:
      System.out.println(String.format("Invalid method id: %d", methodid));
      System.exit(-1);
    }
    return result;
  }

  public static int getOphdrsize(int methodid) {
    int result = 0;
    switch (methodid) {
    case GlobalConfig.FARREACH_ID:
    case GlobalConfig.NOCACHE_ID:
    case GlobalConfig.NETCACHE_ID:
      result = SIZEOF_UINT16 + Key.size(); // T.size();
      break;
    case GlobalConfig.DISTFARREACH_ID:
    case GlobalConfig.DISTNOCACHE_ID:
      result = SIZEOF_UINT16 + SIZEOF_UINT16 + Key.size(); // T.size()
      break;
    case GlobalConfig.DISTCACHE_ID:
      result = SIZEOF_UINT16 + SIZEOF_UINT16 + SIZEOF_UINT16 + Key.size(); // T.size()
      break;
    default:
      System.out.println(String.format("Invalid methodid: %d", methodid));
      System.exit(-1);
    }
    return result;
  }

  public static boolean isSingleswitch(int methodid) {
    if (methodid == GlobalConfig.FARREACH_ID || methodid == GlobalConfig.NOCACHE_ID || methodid == GlobalConfig.NETCACHE_ID) {
      return true;
    }
    return false;
  }

  public static short getPacketType(byte[] data, int off, int recvSize) {
    MyUtils.myAssert(off == 0);
    int tmpTypeSize = SIZEOF_UINT16;
    MyUtils.myAssert(recvSize >= tmpTypeSize);

    byte[] tmpTypeBytes = new byte[tmpTypeSize];
    System.arraycopy(data, off, tmpTypeBytes, 0, tmpTypeSize);
    short tmpType = EndianConverter.ntohs(tmpTypeBytes);
    //return PacketType.getPacketType(tmpType);
    return tmpType;
  }

  public Packet() {
    methodid = GlobalConfig.INVALID_ID;
    type = 0;
    key = (T) new Key();
    globalSwitchidx = 0;
    spineSwitchidx = 0;
    leafSwitchidx = 0;
  }

  public Packet(int methodid, PacketType pType, T key) {
    this.methodid = methodid;
    this.type = pType.getType();
    this.key = key;
    this.globalSwitchidx = 0;
    this.spineSwitchidx = 0;
    this.leafSwitchidx = 0;
  }

  public Packet(int methodid, PacketType pType, short globalSwitchidx, T key) {
    this.methodid = methodid;
    this.type = pType.getType();
    this.key = key;
    this.globalSwitchidx = globalSwitchidx;
    this.spineSwitchidx = 0;
    this.leafSwitchidx = 0;
  }

  public Packet(int methodid, PacketType pType, short spineSwitchidx, short leafSwitchidx, T key) {
    this.methodid = methodid;
    this.type = pType.getType();
    this.key = key;
    this.globalSwitchidx = 0;
    this.spineSwitchidx = spineSwitchidx;
    this.leafSwitchidx = leafSwitchidx;
  }

  public int serializePacketType(byte[] data, int off, int maxSize) {
    //MyUtils.myAssert(off == 0); // NOTE: may be shadowtype
    byte[] bigEndianType = EndianConverter.htons(this.type);
    int tmpTypeSize = bigEndianType.length;
    MyUtils.myAssert(maxSize - off >= tmpTypeSize);
    System.arraycopy(bigEndianType, 0, data, off, tmpTypeSize);
    return tmpTypeSize;
  }

  public int dynamicSerializePacketType(DynamicArray dynamicData, int off) {
    byte[] bigEndianType = EndianConverter.htons(this.type);
    int tmpTypeSize = bigEndianType.length;
    dynamicData.dynamicMemcpy(off, bigEndianType, tmpTypeSize);
    return tmpTypeSize;
  }

  public int deserializePacketType(byte[] data, int off, int recvsize) {
    //MyUtils.myAssert(off == 0); // NOTE: may be shadowtype
    int tmpTypeSize = SIZEOF_UINT16;
    MyUtils.myAssert(recvsize - off >= tmpTypeSize);

    byte[] typeBytes = Arrays.copyOfRange(data, off, off + tmpTypeSize);
    MyUtils.myAssert(typeBytes.length == tmpTypeSize);
    this.type = EndianConverter.ntohs(typeBytes);
    return tmpTypeSize;
  }

  public int serializeGlobalSwitchidx(byte[] data, int off, int maxSize) {
    byte[] bigEndianSwitchidx = EndianConverter.htons(this.globalSwitchidx);
    int tmpSwitchidxSize = bigEndianSwitchidx.length;
    MyUtils.myAssert(maxSize - off >= tmpSwitchidxSize);
    System.arraycopy(bigEndianSwitchidx, 0, data, off, tmpSwitchidxSize);
    return tmpSwitchidxSize;
  }

  public int dynamicSerializeGlobalSwitchidx(DynamicArray dynamicData, int off) {
    byte[] bigEndianSwitchidx = EndianConverter.htons(this.globalSwitchidx);
    int tmpSwitchidxSize = bigEndianSwitchidx.length;
    dynamicData.dynamicMemcpy(off, bigEndianSwitchidx, tmpSwitchidxSize);
    return tmpSwitchidxSize;
  }

  public int deserializeGlobalSwitchidx(byte[] data, int off, int recvsize) {
    int tmpSwitchidxSize = SIZEOF_UINT16;
    MyUtils.myAssert(recvsize - off >= tmpSwitchidxSize);

    byte[] switchidxBytes = Arrays.copyOfRange(data, off, off + tmpSwitchidxSize);
    MyUtils.myAssert(switchidxBytes.length == tmpSwitchidxSize);
    this.globalSwitchidx = EndianConverter.ntohs(switchidxBytes);
    return tmpSwitchidxSize;
  }

  public int serializeSpineSwitchidx(byte[] data, int off, int maxSize) {
    byte[] bigEndianSwitchidx = EndianConverter.htons(this.spineSwitchidx);
    int tmpSwitchidxSize = bigEndianSwitchidx.length;
    MyUtils.myAssert(maxSize - off >= tmpSwitchidxSize);
    System.arraycopy(bigEndianSwitchidx, 0, data, off, tmpSwitchidxSize);
    return tmpSwitchidxSize;
  }

  public int dynamicSerializeSpineSwitchidx(DynamicArray dynamicData, int off) {
    byte[] bigEndianSwitchidx = EndianConverter.htons(this.spineSwitchidx);
    int tmpSwitchidxSize = bigEndianSwitchidx.length;
    dynamicData.dynamicMemcpy(off, bigEndianSwitchidx, tmpSwitchidxSize);
    return tmpSwitchidxSize;
  }

  public int deserializeSpineSwitchidx(byte[] data, int off, int recvsize) {
    int tmpSwitchidxSize = SIZEOF_UINT16;
    MyUtils.myAssert(recvsize - off >= tmpSwitchidxSize);

    byte[] switchidxBytes = Arrays.copyOfRange(data, off, off + tmpSwitchidxSize);
    MyUtils.myAssert(switchidxBytes.length == tmpSwitchidxSize);
    this.spineSwitchidx = EndianConverter.ntohs(switchidxBytes);
    return tmpSwitchidxSize;
  }

  public int serializeLeafSwitchidx(byte[] data, int off, int maxSize) {
    byte[] bigEndianSwitchidx = EndianConverter.htons(this.leafSwitchidx);
    int tmpSwitchidxSize = bigEndianSwitchidx.length;
    MyUtils.myAssert(maxSize - off >= tmpSwitchidxSize);
    System.arraycopy(bigEndianSwitchidx, 0, data, off, tmpSwitchidxSize);
    return tmpSwitchidxSize;
  }

  public int dynamicSerializeLeafSwitchidx(DynamicArray dynamicData, int off) {
    byte[] bigEndianSwitchidx = EndianConverter.htons(this.leafSwitchidx);
    int tmpSwitchidxSize = bigEndianSwitchidx.length;
    dynamicData.dynamicMemcpy(off, bigEndianSwitchidx, tmpSwitchidxSize);
    return tmpSwitchidxSize;
  }

  public int deserializeLeafSwitchidx(byte[] data, int off, int recvsize) {
    int tmpSwitchidxSize = SIZEOF_UINT16;
    MyUtils.myAssert(recvsize - off >= tmpSwitchidxSize);

    byte[] switchidxBytes = Arrays.copyOfRange(data, off, off + tmpSwitchidxSize);
    MyUtils.myAssert(switchidxBytes.length == tmpSwitchidxSize);
    this.leafSwitchidx = EndianConverter.ntohs(switchidxBytes);
    return tmpSwitchidxSize;
  }

  public final T key() {
    return key;
  }

  public final short type() {
    return type;
  }

  public final short globalSwitchidx() {
    return globalSwitchidx;
  }

  public final short spineSwitchidx() {
    return spineSwitchidx;
  }

  public final short leafSwitchidx() {
    return leafSwitchidx;
  }

  void setKey(T keyInput) {
    this.key = keyInput;
  }

  void setType(short typeInput) {
    this.type = typeInput;
  }

  public int serializeOphdr(byte[] data, int off, int maxSize) {
    MyUtils.myAssert(off == 0);
    MyUtils.myAssert(this.methodid != GlobalConfig.INVALID_ID);
    int ophdrSize = getOphdrsize(this.methodid);
    MyUtils.myAssert(maxSize >= ophdrSize);

    int tmpoff = off;
    int tmpTypeSize = serializePacketType(data, tmpoff, maxSize);
    tmpoff += tmpTypeSize;
    if (!isSingleswitch(this.methodid)) {
      if (this.methodid == GlobalConfig.DISTCACHE_ID) {
        int tmpSpineswitchidxSize = serializeSpineSwitchidx(data, tmpoff, maxSize);
        tmpoff += tmpSpineswitchidxSize;
        int tmpLeafswitchidxSize = serializeLeafSwitchidx(data, tmpoff, maxSize);
        tmpoff += tmpLeafswitchidxSize;
      } else {
        int tmpSwitchidxSize = serializeGlobalSwitchidx(data, tmpoff, maxSize);
        tmpoff += tmpSwitchidxSize;
      }
    }

    int tmpKeySize = this.key.serialize(data, tmpoff, maxSize);
    tmpoff += tmpKeySize;
    return tmpoff - off;
  }

  public int dynamicSerializeOphdr(DynamicArray dynamicData, int off) {
    MyUtils.myAssert(off == 0);
    MyUtils.myAssert(this.methodid != GlobalConfig.INVALID_ID);

    int tmpoff = off;
    int tmpTypesize = dynamicSerializePacketType(dynamicData, tmpoff);
    tmpoff += tmpTypesize;
    if (!isSingleswitch(this.methodid)) {
      if (this.methodid == GlobalConfig.DISTCACHE_ID) {
        int tmpSpineswitchidxSize = dynamicSerializeSpineSwitchidx(dynamicData, tmpoff);
        tmpoff += tmpSpineswitchidxSize;
        int tmpLeafswitchidxSize = dynamicSerializeLeafSwitchidx(dynamicData, tmpoff);
        tmpoff += tmpLeafswitchidxSize;
      } else {
        int tmpSwitchidxSize = dynamicSerializeGlobalSwitchidx(dynamicData, tmpoff);
        tmpoff += tmpSwitchidxSize;
      }
    }
    int tmpKeySize = this.key.dynamicSerialize(dynamicData, tmpoff);
    tmpoff += tmpKeySize;
    return tmpoff - off;
  }

  public int deserializeOphdr(byte[] data, int off, int recvsize) {
    MyUtils.myAssert(off == 0);
    MyUtils.myAssert(this.methodid != GlobalConfig.INVALID_ID);
    int ophdrSize = getOphdrsize(this.methodid);
    MyUtils.myAssert(recvsize >= ophdrSize);

    int tmpoff = off;
    int tmpTypeSize = deserializePacketType(data, tmpoff, recvsize);
    tmpoff += tmpTypeSize;
    if (!isSingleswitch(this.methodid)) {
      if (this.methodid == GlobalConfig.DISTCACHE_ID) {
        int tmpSpineSwitchidxSize = deserializeSpineSwitchidx(data, tmpoff, recvsize);
        tmpoff += tmpSpineSwitchidxSize;
        int tmpLeafSwitchidxSize = deserializeLeafSwitchidx(data, tmpoff, recvsize);
        tmpoff += tmpLeafSwitchidxSize;
      } else {
        int tmpGlobalSwitchidxSize = deserializeGlobalSwitchidx(data, tmpoff, recvsize);
        tmpoff += tmpGlobalSwitchidxSize;
      }
    }
    int tmpKeySize = this.key.deserialize(data, tmpoff, recvsize);
    tmpoff += tmpKeySize;
    return tmpoff - off;
  }

  public abstract int serialize(byte[] data, int maxSize);

  abstract void deserialize(byte[] data, int recvSize);

  abstract int size();
}