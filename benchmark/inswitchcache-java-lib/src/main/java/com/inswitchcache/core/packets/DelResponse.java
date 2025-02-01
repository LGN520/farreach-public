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
import com.inswitchcache.core.Key;

/**
 * Farreach del response Packet structure.
 */
public class DelResponse<T extends Key> extends Packet<T> {
  private boolean stat;
  private short nodeIdxForEval;

  public DelResponse(int methodid, T key, boolean stat, short nodeIdxForEval) {
    super(methodid, PacketType.DELRES, key);
    this.stat = stat;
    this.nodeIdxForEval = nodeIdxForEval;
  }

  public DelResponse(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.deserialize(data, recvSize);
    if (this.type != PacketType.DELRES.getType()) {
      System.out.println(String.format("[DelResponse] Wrong converted packet type 0x%x!", this.type));
      System.exit(-1);
    }
  }

  public boolean stat() {
    return this.stat;
  }

  public short nodeIdxForEval() {
    return this.nodeIdxForEval;
  }
  public int serialize(byte[] data, int maxSize) {
    int offset = 0;

    // ophdr
    int tmpOphdrsize = serializeOphdr(data, offset, maxSize);
    offset += tmpOphdrsize;

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

    return offset;
  }

  protected int size() {
    // return sizeof(optype_t) + sizeof(key_t) + sizeof(optype_t) + sizeof(bool) +
    // sizeof(uint16_t) + STAT_PADDING_BYTES;
    return getOphdrsize(this.methodid) + SIZEOF_UINT16 + SIZEOF_UINT8 + SIZEOF_UINT16 + getStatPaddingBytes(this.methodid);
  }

  protected void deserialize(byte[] data, int recvSize) {
    int offset = 0;

    // ophdr
    int tmpOphdrsize = deserializeOphdr(data, offset, recvSize);
    offset += tmpOphdrsize;

    // shadow type
    offset += SIZEOF_UINT16;

    // stat
    this.stat = (data[offset] != 0);
    offset += SIZEOF_UINT8;

    // nodeIdxForEval
    byte[] nodeIdxForEvalBytes = new byte[2];
    System.arraycopy(data, offset, nodeIdxForEvalBytes, 0, SIZEOF_UINT16);
    this.nodeIdxForEval = EndianConverter.ntohs(nodeIdxForEvalBytes);
    offset += SIZEOF_UINT16;

    offset += getStatPaddingBytes(this.methodid);
  }
}