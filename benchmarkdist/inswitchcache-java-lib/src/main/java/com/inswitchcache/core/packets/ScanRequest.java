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
 * Farreach scan request Packet structure.
 */
public class ScanRequest<T extends Key> extends Packet<T> {
  private T endKey;

  public ScanRequest() {
    super();
    setType(PacketType.SCANREQ.getType());
    this.endKey = (T) Key.min();
  }

  public ScanRequest(int methodid, T key, T endKey) {
    super(methodid, PacketType.SCANREQ, key);
    this.endKey = endKey;
  }

  public ScanRequest(int methodid, byte[] data, int recvSize) {
    super();
    this.methodid = methodid;
    this.deserialize(data, recvSize);
    MyUtils.myAssert(this.type == PacketType.SCANREQ.getType());
  }

  public T endKey() {
    return this.endKey;
  }

  int size() {
    // return sizeof(optype_t) + sizeof(key_t) + sizeof(key_t);
    return getOphdrsize(this.methodid) + Key.size(); // T.size()
  }

  void deserialize(byte[] data, int recvSize) {
    int mySize = this.size();
    MyUtils.myAssert(mySize <= recvSize);

    int offset = 0;

    // ophdr
    int tmpOphdrsize = deserializeOphdr(data, offset, recvSize);
    offset += tmpOphdrsize;

    // Copy deserialized endkey type
    int tmpEndkeysize = this.endKey.deserialize(data, offset, recvSize);
    offset += tmpEndkeysize;
  }

  public int serialize(byte[] data, int maxSize) {
    MyUtils.myAssert(maxSize >= this.size());

    int offset = 0;

    // ophdr
    int tmpOphdrsize = serializeOphdr(data, offset, maxSize);
    offset += tmpOphdrsize;

    // Copy endkey bytes into data
    int tmpEndKeySize = this.endKey.serialize(data, offset, maxSize);
    offset += tmpEndKeySize;

    return offset;
  }
}
