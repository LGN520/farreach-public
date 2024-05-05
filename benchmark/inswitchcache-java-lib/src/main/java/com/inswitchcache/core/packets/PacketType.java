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

/**
 * Farreach packet type enum.
 */
public enum PacketType {
  PUTREQ((short) 0x0001),
  DISTNOCACHE_PUTREQ_SPINE((short) 0x0041),
  PUTREQ_SEQ((short) 0x0003),
  PUTREQ_POP_SEQ((short) 0x0013),
  PUTREQ_SEQ_CASE3((short) 0x0023),
  PUTREQ_POP_SEQ_CASE3((short) 0x0033),
  NETCACHE_PUTREQ_SEQ_CACHED((short) 0x0043),

  GETRES_LATEST_SEQ_INSWITCH((short) 0x000f),
  GETRES_DELETED_SEQ_INSWITCH((short) 0x001f),
  GETRES_LATEST_SEQ_INSWITCH_CASE1((short) 0x002f),
  GETRES_DELETED_SEQ_INSWITCH_CASE1((short) 0x003f),
  PUTREQ_SEQ_INSWITCH_CASE1((short) 0x004f),
  DELREQ_SEQ_INSWITCH_CASE1((short) 0x005f),
  LOADSNAPSHOTDATA_INSWITCH_ACK((short) 0x006f),
  CACHE_POP_INSWITCH((short) 0x007f),
  NETCACHE_VALUEUPDATE_INSWITCH((short) 0x008f),
  GETRES_LATEST_SEQ_SERVER_INSWITCH((short) 0x009f),
  GETRES_DELETED_SEQ_SERVER_INSWITCH((short) 0x010f),
  DISTCACHE_SPINE_VALUEUPDATE_INSWITCH((short) 0x011f),
  DISTCACHE_LEAF_VALUEUPDATE_INSWITCH((short) 0x012f),
  DISTCACHE_VALUEUPDATE_INSWITCH((short) 0x013f),
  DISTCACHE_VALUEUPDATE_INSWITCH_ORIGIN((short) 0x014f),

  GETRES_LATEST_SEQ((short) 0x000b),
  GETRES_DELETED_SEQ((short) 0x001b),
  CACHE_EVICT_LOADDATA_INSWITCH_ACK((short) 0x002b),
  NETCACHE_VALUEUPDATE((short) 0x003b),
  GETRES_LATEST_SEQ_SERVER((short) 0x004b),
  GETRES_DELETED_SEQ_SERVER((short) 0x005b),
  GETRES_SEQ((short) 0x006b),

  GETRES((short) 0x0009),
  GETRES_SERVER((short) 0x0019),
  DISTCACHE_GETRES_SPINE((short) 0x0029),
  PUTREQ_INSWITCH((short) 0x0005),
  DELREQ_SEQ_INSWITCH((short) 0x0006),
  PUTREQ_LARGEVALUE_SEQ_INSWITCH((short) 0x0016),
  PUTREQ_SEQ_INSWITCH((short) 0x0007),

  GETREQ_INSWITCH((short) 0x0004),
  DELREQ_INSWITCH((short) 0x0014),
  CACHE_EVICT_LOADFREQ_INSWITCH((short) 0x0024),
  CACHE_EVICT_LOADDATA_INSWITCH((short) 0x0034),

  LOADSNAPSHOTDATA_INSWITCH((short) 0x0044),
  SETVALID_INSWITCH((short) 0x0054),
  NETCACHE_WARMUPREQ_INSWITCH((short) 0x0064),

  NETCACHE_WARMUPREQ_INSWITCH_POP((short) 0x0074),
  DISTCACHE_INVALIDATE_INSWITCH((short) 0x0084),

  DISTCACHE_VALUEUPDATE_INSWITCH_ACK((short) 0x0094),
  PUTREQ_LARGEVALUE_INSWITCH((short) 0x00a4),

  DELREQ_SEQ((short) 0x0002),
  DELREQ_SEQ_CASE3((short) 0x0012),
  NETCACHE_DELREQ_SEQ_CACHED((short) 0x0022),
  PUTREQ_LARGEVALUE_SEQ((short) 0x0032),

  PUTRES((short) 0x0008),
  DELRES((short) 0x0018),
  PUTRES_SERVER((short) 0x0028),
  DELRES_SERVER((short) 0x0038),

  PUTRES_SEQ((short) 0x000a),
  DELRES_SEQ((short) 0x001a),

  WARMUPREQ((short) 0x0000),
  SCANREQ((short) 0x0010),
  SCANREQ_SPLIT((short) 0x0020),
  GETREQ((short) 0x0030),
  DELREQ((short) 0x0040),
  GETREQ_POP((short) 0x0050),
  GETREQ_NLATEST((short) 0x0060),
  CACHE_POP_INSWITCH_ACK((short) 0x0070),
  SCANRES_SPLIT((short) 0x0080),
  CACHE_POP((short) 0x0090),
  CACHE_EVICT((short) 0x00a0),
  CACHE_EVICT_ACK((short) 0x00b0),
  CACHE_EVICT_CASE2((short) 0x00c0),
  WARMUPACK((short) 0x00d0),
  LOADACK((short) 0x00e0),
  CACHE_POP_ACK((short) 0x00f0),

  CACHE_EVICT_LOADFREQ_INSWITCH_ACK((short) 0x0100),
  SETVALID_INSWITCH_ACK((short) 0x0110),
  NETCACHE_GETREQ_POP((short) 0x0120),
  NETCACHE_CACHE_POP((short) 0x0130),
  NETCACHE_CACHE_POP_ACK((short) 0x0140),
  NETCACHE_CACHE_POP_FINISH((short) 0x0150),
  NETCACHE_CACHE_POP_FINISH_ACK((short) 0x0160),
  NETCACHE_CACHE_EVICT((short) 0x0170),
  NETCACHE_CACHE_EVICT_ACK((short) 0x0180),
  NETCACHE_VALUEUPDATE_ACK((short) 0x0190),

  GETREQ_SPINE((short) 0x0200),
  SCANRES_SPLIT_SERVER((short) 0x0210),
  WARMUPREQ_SPINE((short) 0x0220),
  WARMUPACK_SERVER((short) 0x0230),
  LOADACK_SERVER((short) 0x0240),
  DISTCACHE_CACHE_EVICT_VICTIM((short) 0x0250),
  DISTCACHE_CACHE_EVICT_VICTIM_ACK((short) 0x0260),
  DISTNOCACHE_DELREQ_SPINE((short) 0x0270),
  DISTCACHE_INVALIDATE((short) 0x0280),
  DISTCACHE_INVALIDATE_ACK((short) 0x0290),
  DISTCACHE_UPDATE_TRAFFICLOAD((short) 0x02a0),
  DISTCACHE_SPINE_VALUEUPDATE_INSWITCH_ACK((short) 0x02b0),
  DISTCACHE_LEAF_VALUEUPDATE_INSWITCH_ACK((short) 0x02c0),

  PUTREQ_LARGEVALUE((short) 0x02d0),
  DISTNOCACHE_PUTREQ_LARGEVALUE_SPINE((short) 0x02e0),
  GETRES_LARGEVALUE_SERVER((short) 0x02f0),

  GETRES_LARGEVALUE((short) 0x0300),
  LOADREQ((short) 0x0310),
  LOADREQ_SPINE((short) 0x0320),
  GETRES_LARGEVALUE_SEQ((short) 0x0350);

  private final short packetType;

  // private enum constructor
  private PacketType(short packetType) {
    this.packetType = packetType;
  }

  public short getType() {
    return packetType;
  }

  public boolean comparePacket(short i) {
    return packetType == i;
  }

  public boolean comparePacket(PacketType pt) {
    return this.getType() == pt.getType();
  }

  // NOTE: not used due to expensive operation
  public static PacketType getPacketType(short packetTypeInput) {
    PacketType[] pts = PacketType.values();
    for (int i = 0; i < pts.length; i++) {
      if (pts[i].comparePacket(packetTypeInput)) {
        return pts[i];
      }
    }
    System.out.println(String.format("Invalid packet type: 0x%x", packetTypeInput));
    System.exit(-1);

    // never arrive
    return PacketType.PUTREQ;
  }
}
