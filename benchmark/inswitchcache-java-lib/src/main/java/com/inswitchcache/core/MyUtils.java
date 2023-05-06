package com.inswitchcache.core;

public final class MyUtils {

  public static void myAssert(boolean assertion) {
    if (!assertion) {
      System.out.println("[ERROR] myAssert fails with the following statcktrace:");
      StackTraceElement[] elements = Thread.currentThread().getStackTrace();
      for (int i = 0; i < elements.length; i++) {
        System.out.println(elements[i].toString());
      }
      System.exit(-1);
    }
  }

  private static final char[] HEX_ARRAY = "0123456789ABCDEF".toCharArray();

  public static String bytesToHexString(byte[] bytes, int length) {
    char[] hexChars = new char[length * 2];
    for (int j = 0; j < length; j++) {
      int v = bytes[j] & 0xFF;
      hexChars[j * 2] = HEX_ARRAY[v >>> 4];
      hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
    }
    return String.format("size %d: %s", length, new String(hexChars));
  }

  public static String dynamicArrayToHexString(DynamicArray dynamicbuf) {
    return bytesToHexString(dynamicbuf.array(), dynamicbuf.size());
  }
}
