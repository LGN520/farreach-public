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

import java.util.Arrays;

import com.inswitchcache.core.EndianConverter;
import com.inswitchcache.core.GlobalConfig;
import com.inswitchcache.core.Key;
import com.inswitchcache.core.Value;

/**
 * Farreach get response Packet structure.
 */
public class GetResponse<T extends Key, V extends Value> extends Packet<T> {
  private V val;
  private boolean stat;
  private short nodeIdxForEval;
  // for distcache
  int spineload;
  int leafload;

  public GetResponse() {
    super();
    setType(PacketType.GETRES.getType());
    this.val = (V) new Value();
    this.stat = false;
    this.nodeIdxForEval = 0;
    this.spineload = 0;
    this.leafload = 0;
  }

  public GetResponse(int methodid, T key, V val, boolean stat, short nodeIdxForEval) {
    super(methodid, PacketType.GETRES, key);
    this.val = val;
    this.stat = stat;
    this.nodeIdxForEval = nodeIdxForEval;

    if (this.val.getValData().length > Value.getSwitchMaxValLen()) {
      System.out.println(String.format("[GetResponse] Value length too long: %d", this.val.getValData().length));
      System.exit(-1);
    }
  }

  public GetResponse(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.val = (V) new Value();
    this.deserialize(data, recvSize);
    if (this.type != PacketType.GETRES.getType()) {
      System.out.println(String.format("[GetResponse] Wrong converted packet type 0x%x!", this.type));
      System.exit(-1);
    }
    // String dataString = "data: ";
    // for (int i=0; i< 128; i++) {
    //   dataString = dataString + Integer.toString(data[i]) + " ";
    // }
    // System.out.println(dataString);
    // System.out.println("this.myTmpValSize: " + this.myTmpValSize);
    // System.out.println("this.val: " + this.val.getBytesNum());
    //if (this.val.getBytesNum() == 0) {
    //  return;
    //}
    if (this.val.getBytesNum() == 0) {
      return;
    }
    if (this.val.getValData().length > Value.getSwitchMaxValLen()) {
      System.out.println(String.format("[GetResponse] Value length too long: %d", this.val.getValData().length));
      System.exit(-1);
    }
  }

  public Value val() {
    return this.val;
  }

  public boolean stat() {
    return this.stat;
  }

  public short nodeIdxForEval() {
    return this.nodeIdxForEval;
  }

  public int size() {
    // sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN +
    // sizeof(optype_t) + sizeof(bool) + sizeof(uint16_t) + STAT_PADDING_BYTES;
    int size = getOphdrsize(this.methodid) + SIZEOF_UINT16 + Value.getSwitchMaxValLen() +
        SIZEOF_UINT16 + SIZEOF_UINT8 + SIZEOF_UINT16 + getStatPaddingBytes(this.methodid);
    if (this.methodid == GlobalConfig.DISTCACHE_ID) {
      size += (SIZEOF_UINT32 + SIZEOF_UINT32);
    }
    return size;
  }

  public int serialize(byte[] data, int maxSize) {
    int offset = 0;

    // ophdr
    int tmpOphdrsize = serializeOphdr(data, offset, maxSize);
    offset += tmpOphdrsize;

    // val
    int tmpValSize = this.val.serialize(data, offset, maxSize);
    offset += tmpValSize;

    // shadow type
    int tmpShadowTypeSize = serializePacketType(data, offset, maxSize);
    offset += tmpShadowTypeSize;

    // stat
    byte[] statByte = {(byte) (this.stat ? 1 : 0)};
    System.arraycopy(statByte, 0, data, offset, SIZEOF_UINT8);
    offset += SIZEOF_UINT8;

    // bigEndianNodeIdxForEval
    byte[] bigEndianNodeIDxForEvalBytes = EndianConverter.htons(this.nodeIdxForEval);
    int bigEndianNodeIDxForEvalSize = bigEndianNodeIDxForEvalBytes.length;
    System.arraycopy(bigEndianNodeIDxForEvalBytes, 0, data, offset, bigEndianNodeIDxForEvalSize);
    offset += bigEndianNodeIDxForEvalSize;

    // stat_padding_bytes
    offset += getStatPaddingBytes(this.methodid);

    if (this.methodid == GlobalConfig.DISTCACHE_ID) {
      byte[] bigEndianSpineloadBytes = EndianConverter.htoni(this.spineload);
      int bigEndianSpineloadSize = bigEndianSpineloadBytes.length;
      System.arraycopy(bigEndianSpineloadBytes, 0, data, offset, bigEndianSpineloadSize);
      offset += bigEndianSpineloadSize;

      byte[] bigEndianLeafloadBytes = EndianConverter.htoni(this.leafload);
      int bigEndianLeafloadSize = bigEndianLeafloadBytes.length;
      System.arraycopy(bigEndianLeafloadBytes, 0, data, offset, bigEndianLeafloadSize);
      offset += bigEndianLeafloadSize;
    }

    return offset;
  }

  protected void deserialize(byte[] data, int recvSize) {
    int offset = 0;

    // ophdr
    int tmpOphdrsize = deserializeOphdr(data, offset, recvSize);
    offset += tmpOphdrsize;

    // val
    int tmpValSize = this.val.deserialize(data, offset, recvSize);
    offset += tmpValSize;

    // shadow type
    offset += SIZEOF_UINT16;

    // stat
    this.stat = (data[offset] != 0);
    offset += SIZEOF_UINT8;

    // nodeIdxForEval
    byte[] nodeIdxForEvalBytes = new byte[SIZEOF_UINT16];
    System.arraycopy(data, offset, nodeIdxForEvalBytes, 0, SIZEOF_UINT16);
    this.nodeIdxForEval = EndianConverter.ntohs(nodeIdxForEvalBytes);
    offset += SIZEOF_UINT16;

    offset += getStatPaddingBytes(this.methodid);

    if (this.methodid == GlobalConfig.DISTCACHE_ID) {
      byte[] spineloadBytes = new byte[SIZEOF_UINT32];
      System.arraycopy(data, offset, spineloadBytes, 0, SIZEOF_UINT32);
      this.spineload = EndianConverter.ntohi(spineloadBytes);
      offset += SIZEOF_UINT32;

      byte[] leafloadBytes = new byte[SIZEOF_UINT32];
      System.arraycopy(data, offset, leafloadBytes, 0, SIZEOF_UINT32);
      this.leafload = EndianConverter.ntohi(leafloadBytes);
      offset += SIZEOF_UINT32;
    }
  }

  public int getSpineload() {
    return spineload;
  }

  public void setSpineload(int spineload) {
    this.spineload = spineload;
  }

  public int getLeafload() {
    return leafload;
  }

  public void setLeafload(int leafload) {
    this.leafload = leafload;
  }
}