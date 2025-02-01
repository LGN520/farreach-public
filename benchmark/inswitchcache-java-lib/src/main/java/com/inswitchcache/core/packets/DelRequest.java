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
import com.inswitchcache.core.MyUtils;

/**
 * Farreach delete request Packet structure.
 */
public class DelRequest<T extends Key> extends Packet<T> {
  public DelRequest() {
    super();
    setType(PacketType.DELREQ.getType());
  }

  public DelRequest(int methodid, T key) {
    super(methodid, PacketType.DELREQ, key);
  }

  public DelRequest(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.deserialize(data, recvSize);
    MyUtils.myAssert(type() == PacketType.DELREQ.getType());
  }

  int size() {
    // return sizeof(optype_t) + sizeof(key_t);
    return getOphdrsize(this.methodid);
  }

  void deserialize(byte[] data, int recvSize) {
    int mySize = this.size();
    MyUtils.myAssert(mySize <= recvSize);

    int tmpoff = 0;
    int tmpOphdrsize = deserializeOphdr(data, tmpoff, recvSize);
    tmpoff += tmpOphdrsize;
    return;
  }

  public int serialize(byte[] data, int maxSize) {
    MyUtils.myAssert(maxSize >= this.size());

    int tmpoff = 0;
    int tmpOphdrsize = serializeOphdr(data, tmpoff, maxSize);
    tmpoff += tmpOphdrsize;

    return tmpoff;
  }
}
