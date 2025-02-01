package com.inswitchcache.core;

import java.nio.ByteBuffer;

/**
 * InswitchCache helper class.
 * Convert bytes and int endianess.
 */
public final class EndianConverter {
  private EndianConverter() {
    // not used
  }

  // ntoh: big-endian to small-endian
  public static short ntohs(byte[] value) {
    ByteBuffer buf = ByteBuffer.wrap(value);
    return buf.getShort();
  }

  public static int ntohi(byte[] value) {
    ByteBuffer buf = ByteBuffer.wrap(value);
    return buf.getInt();
  }

  public static long ntohl(byte[] value) {
    ByteBuffer buf = ByteBuffer.wrap(value);
    return buf.getLong();
  }

  // hton: small-endian to big-endian
  public static byte[] htons(short sValue) {
    byte[] baValue = new byte[2];
    ByteBuffer buf = ByteBuffer.wrap(baValue);
    return buf.putShort(sValue).array();
  }

  public static byte[] htoni(int sValue) {
    byte[] baValue = new byte[4];
    ByteBuffer buf = ByteBuffer.wrap(baValue);
    return buf.putInt(sValue).array();
  }

  public static byte[] htonl(long sValue) {
    byte[] baValue = new byte[8];
    ByteBuffer buf = ByteBuffer.wrap(baValue);
    return buf.putLong(sValue).array();
  }
}