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

import com.inswitchcache.core.DynamicArray;
import com.inswitchcache.core.EndianConverter;
import com.inswitchcache.core.Key;
import com.inswitchcache.core.Value;
import com.inswitchcache.core.MyUtils;

/**
 * Farreach put request Packet structure.
 */
public class PutRequestLargeValue<T extends Key, V extends Value> extends Packet<T> {
  private V val;

  short clientLogicalIdx;
  int fragseq;

  public static int getFraghdrSize(int methodid) {
    return getOphdrsize(methodid) + SIZEOF_UINT16 + SIZEOF_UINT32; // op_hdr + client_logical_idx + fragseq
  }

  public PutRequestLargeValue() {
    super();
    setType(PacketType.PUTREQ_LARGEVALUE.getType());
    this.val = (V) new Value();
    this.clientLogicalIdx = 0;
    this.fragseq = 0;
  }

  public PutRequestLargeValue(int methodid, T key, V value, short clientLogicalIdx, int fragseq) {
    super(methodid, PacketType.PUTREQ_LARGEVALUE, key);
    this.val = value;
    this.clientLogicalIdx = clientLogicalIdx;
    this.fragseq = fragseq;
    MyUtils.myAssert(value.getValData().length > Value.getSwitchMaxValLen());
  }

  public PutRequestLargeValue(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.deserialize(data, recvSize);
    MyUtils.myAssert(type() == PacketType.PUTREQ_LARGEVALUE.getType());
    MyUtils.myAssert(this.val.getValData().length > Value.getSwitchMaxValLen());
  }

  public V val() {
    return this.val;
  }

  int size() {
    // return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) +
    // val_t::MAX_VALLEN + sizeof(optype_t);
    return getOphdrsize(this.methodid) + SIZEOF_UINT16 + SIZEOF_UINT32 + SIZEOF_UINT32 + Value.getSwitchMaxValLen();
  }

  void deserialize(byte[] data, int recvSize) {
    //int mySize = this.size();
    //MyUtils.myAssert(mySize <= recvSize);

    int offset = 0;

    // ophdr
    int tmpOphdrsize = deserializeOphdr(data, offset, recvSize);
    offset += tmpOphdrsize;

    // clientLogicalIdx
    byte[] clientLogicalIdxBytes = new byte[SIZEOF_UINT16];
    System.arraycopy(data, offset, clientLogicalIdxBytes, 0, SIZEOF_UINT16);
    this.clientLogicalIdx = EndianConverter.ntohs(clientLogicalIdxBytes);
    offset += SIZEOF_UINT16;

    // fragseq
    byte[] fragseqBytes = new byte[SIZEOF_UINT32];
    System.arraycopy(data, offset, fragseqBytes, 0, SIZEOF_UINT32);
    this.fragseq = EndianConverter.ntohi(fragseqBytes);
    offset += SIZEOF_UINT32;

    // val
    int tmpValsize = this.val.deserializeLarge(data, offset, recvSize);
    offset += tmpValsize;

    return;
  }

  public int serialize(byte[] data, int maxSize) {
    //MyUtils.myAssert(maxSize >= this.size());

    int offset = 0;

    // ophdr
    int tmpOphdrsize = serializeOphdr(data, offset, maxSize);
    offset += tmpOphdrsize;

    // clientLogicalIdx
    byte[] bigEndianClientLogicalIdxBytes = EndianConverter.htons(this.clientLogicalIdx);
    int bigEndianClientLogicalIdxSize = bigEndianClientLogicalIdxBytes.length;
    System.arraycopy(bigEndianClientLogicalIdxBytes, 0, data, offset, bigEndianClientLogicalIdxSize);
    offset += bigEndianClientLogicalIdxSize;

    // fragseq
    byte[] bigEndianFragseqBytes = EndianConverter.htoni(this.fragseq);
    int bigEndianFragseqSize = bigEndianFragseqBytes.length;
    System.arraycopy(bigEndianFragseqBytes, 0, data, offset, bigEndianFragseqSize);
    offset += bigEndianFragseqSize;

    // val
    int tmpValSize = this.val.serializeLarge(data, offset, maxSize);
    offset += tmpValSize;

    return offset;
  }

  public int dynamicSerialize(DynamicArray dynamicData) {
    //MyUtils.myAssert(maxSize >= this.size());

    int offset = 0;

    // ophdr
    int tmpOphdrsize = dynamicSerializeOphdr(dynamicData, offset);
    offset += tmpOphdrsize;

    // clientLogicalIdx
    byte[] bigEndianClientLogicalIdxBytes = EndianConverter.htons(this.clientLogicalIdx);
    int bigEndianClientLogicalIdxSize = bigEndianClientLogicalIdxBytes.length;
    dynamicData.dynamicMemcpy(offset, bigEndianClientLogicalIdxBytes, bigEndianClientLogicalIdxSize);
    offset += bigEndianClientLogicalIdxSize;

    // fragseq
    byte[] bigEndianFragseqBytes = EndianConverter.htoni(this.fragseq);
    int bigEndianFragseqSize = bigEndianFragseqBytes.length;
    dynamicData.dynamicMemcpy(offset, bigEndianFragseqBytes, bigEndianFragseqSize);
    offset += bigEndianFragseqSize;

    // val
    int tmpValSize = this.val.dynamicSerializeLarge(dynamicData, offset);
    offset += tmpValSize;

    return offset;
  }

  public short getClientLogicalIdx() {
    return clientLogicalIdx;
  }

  public void setClientLogicalIdx(short clientLogicalIdx) {
    this.clientLogicalIdx = clientLogicalIdx;
  }

  public int getFragseq() {
    return fragseq;
  }

  public void setFragseq(int fragseq) {
    this.fragseq = fragseq;
  }
}
