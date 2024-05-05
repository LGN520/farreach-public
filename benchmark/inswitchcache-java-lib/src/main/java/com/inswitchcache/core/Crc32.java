package com.inswitchcache.core;

/**
 * Manually implemented Crc32 Algorithm class.
 */

public final class Crc32 {
  private Crc32() {
    // not used;
  }

  public static int crc32(byte[] message, int size) {
    int i, j;
    int by, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    while (true) {
      by = message[i];

      crc = crc ^ by;
      for (j = 7; j >= 0; j--) {
        mask = -(crc & 1);
        crc = (crc >>> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
      if (i == size) {
        break;
      }
    }
    return ~crc;
  }

  public static long getUnsigned(int signed) {
    return signed >= 0 ? signed : 2 * (long) Integer.MAX_VALUE + 2 + signed;
  }
}