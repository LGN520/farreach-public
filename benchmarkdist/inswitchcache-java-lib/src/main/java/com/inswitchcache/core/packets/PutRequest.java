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

import com.inswitchcache.core.Key;
import com.inswitchcache.core.Value;
import com.inswitchcache.core.EndianConverter;
import com.inswitchcache.core.MyUtils;

/**
 * Farreach put request Packet structure.
 */
public class PutRequest<T extends Key, V extends Value> extends Packet<T> {
  private V val;

  public PutRequest() {
    super();
    setType(PacketType.PUTREQ.getType());
    this.val = (V) new Value();
  }

  public PutRequest(int methodid, T key, V value) {
    super(methodid, PacketType.PUTREQ, key);
    this.val = value;
  }

  public PutRequest(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.deserialize(data, recvSize);
    MyUtils.myAssert(type() == PacketType.PUTREQ.getType());
    MyUtils.myAssert(this.val.getBytesNum() <= Value.getSwitchMaxValLen());
  }

  public V val() {
    return this.val;
  }

  int size() {
    // return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) +
    // val_t::MAX_VALLEN + sizeof(optype_t);
    return getOphdrsize(this.methodid) + SIZEOF_UINT16 + Value.getSwitchMaxValLen() + SIZEOF_UINT16;
  }

  void deserialize(byte[] data, int recvSize) {
    //int mySize = this.size();
    //MyUtils.myAssert(mySize <= recvSize);

    int offset = 0;

    // ophdr
    int tmpOphdrsize = deserializeOphdr(data, offset, recvSize);
    offset += tmpOphdrsize;

    // val
    int tmpValsize = this.val.deserialize(data, offset, recvSize);
    offset += tmpValsize;

    offset += SIZEOF_UINT16; // shadowtype

    return;
  }

  public int serialize(byte[] data, int maxSize) {
    //MyUtils.myAssert(maxSize >= this.size());

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
    return offset;
  }
}
