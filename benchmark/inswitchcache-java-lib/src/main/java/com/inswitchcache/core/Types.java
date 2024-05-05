package com.inswitchcache.core;

/**
 * Farreach Types.
 */
public final class Types {
  private Types() {
    // not used
  }

  public static short getUint8(short s) {
    return (short) (s & 0x00ff);
  }

  public static int getUint16(int i) {
    return i & 0x0000ffff;
  }

  public static long getUin32(long l) {
    return l & 0x00000000ffffffff;
  }
}