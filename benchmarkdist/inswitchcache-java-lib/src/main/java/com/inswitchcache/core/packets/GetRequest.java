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
import com.inswitchcache.core.MyUtils;

/**
 * Farreach get request Packet structure.
 */
public class GetRequest<T extends Key> extends Packet<T> {

  // for distcache
  private int spineload;
  private int leafload;

  public GetRequest() {
    super();
    setType(PacketType.GETREQ.getType());
    this.spineload = 0;
    this.leafload = 0;
  }

  public GetRequest(int methodid, T key) {
    super(methodid, PacketType.GETREQ, key);
    MyUtils.myAssert(methodid != GlobalConfig.DISTCACHE_ID);
    this.spineload = 0;
    this.leafload = 0;
  }

  public GetRequest(int methodid, short spineSwitchidx, short leafSwitchidx, T key) {
    super(methodid, PacketType.GETREQ, spineSwitchidx, leafSwitchidx, key);
    MyUtils.myAssert(methodid == GlobalConfig.DISTCACHE_ID);
    this.spineload = 0;
    this.leafload = 0;
  }

  public GetRequest(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.deserialize(data, recvSize);
    if (this.type != PacketType.GETREQ.getType()) {
      System.out.println(String.format("[GetRequest] Wrong converted packet type 0x%x!", this.type));
      System.exit(-1);
    }
  }

  int size() {
    // return sizeof(optype_t) + sizeof(key_t);
    int size = getOphdrsize(this.methodid);
    if (this.methodid == GlobalConfig.DISTCACHE_ID) {
      size += (SIZEOF_UINT16 + SIZEOF_UINT32 + SIZEOF_UINT32);
    }
    return size;
  }

  void deserialize(byte[] data, int recvSize) {
    int mySize = this.size();
    MyUtils.myAssert(mySize <= recvSize);

    int offset = 0;
    int tmpOphdrsize = deserializeOphdr(data, offset, recvSize);
    offset += tmpOphdrsize;

    if (this.methodid == GlobalConfig.DISTCACHE_ID) {
      offset += SIZEOF_UINT16; // shadowtype

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

  public int serialize(byte[] data, int maxSize) {
    MyUtils.myAssert(maxSize >= this.size());

    int offset = 0;
    int tmpOphdrsize = serializeOphdr(data, offset, maxSize);
    offset += tmpOphdrsize;

    if (this.methodid == GlobalConfig.DISTCACHE_ID) {
      int tmpShadowTypeSize = serializePacketType(data, offset, maxSize);
      offset += tmpShadowTypeSize;

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
