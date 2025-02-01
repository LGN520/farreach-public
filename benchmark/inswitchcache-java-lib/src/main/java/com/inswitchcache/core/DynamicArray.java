package com.inswitchcache.core;

import java.util.Arrays;

/**
 * Class for sending/receiving request/response with large value.
 */
public final class DynamicArray {
  private int cursize = 0;
  private int curcapability = 0;
  private int mincapability = 0;
  private int maxcapability = 0;
  private byte[] array = null;

  public DynamicArray() {
    cursize = 0;
    curcapability = 0;
    mincapability = 0;
    maxcapability = 0;
    array = null;
  }

  public DynamicArray(int tmpmincapability, int tmpmaxcapability) {
    cursize = 0;
    init(tmpmincapability, tmpmaxcapability);
  }

  @Override
  protected void finalize() {
    array = null; // free memory of array
  }

  public void init(int tmpmincapability, int tmpmaxcapability) {
    MyUtils.myAssert(tmpmaxcapability > tmpmincapability);

    mincapability = tmpmincapability;
    maxcapability = tmpmaxcapability;

    curcapability = mincapability;
    array = new byte[mincapability];
    Arrays.fill(array, (byte) 0);
  }

  public byte get(int idx) {
    MyUtils.myAssert(idx >= 0 && idx < cursize);
    return array[idx];
  }

  public void set(int idx, byte val) {
    MyUtils.myAssert(idx >= 0 && idx < cursize);
    array[idx] = val;
  }

  public void dynamicMemcpy(int off, byte[] srcarray, int len) {
    dynamicReserve(off, len);

    System.arraycopy(srcarray, 0, array, off, len);
    if (off + len > cursize) {
      cursize = off + len;
    }
  }

  public void dynamicMemset(int off, int value, int len) {
    dynamicReserve(off, len);

    Arrays.fill(array, off, off + len, (byte) value);
    if (off + len > cursize) {
      cursize = off + len;
    }
  }

  public void clear() {
    if (curcapability != mincapability) {
      MyUtils.myAssert(array != null);
      array = null; // free memory of array
      array = new byte[mincapability];
      curcapability = mincapability;
    }
    Arrays.fill(array, (byte) 0);
    cursize = 0;
  }

  public int size() {
    return cursize;
  }

  public byte[] array() {
    return array;
  }

  public void dynamicReserve(int off, int len) {
    if (off + len > maxcapability) {
      System.out
          .println(String.format("[DynamicArray] off %d + len %d exceeds maxcapability %d", off, len, maxcapability));
      System.out.flush();
      System.exit(-1);
    }

    if (off + len > curcapability) {
      // NOTE: curcapability < off + len <= maxcapability
      MyUtils.myAssert(curcapability < maxcapability);

      int newcapability = 2 * curcapability;
      while (newcapability < off + len) {
        newcapability = 2 * newcapability;
      }
      if (newcapability > maxcapability) {
        newcapability = maxcapability;
      }

      byte[] newarray = new byte[newcapability];
      Arrays.fill(newarray, (byte) 0);
      System.arraycopy(array, 0, newarray, 0, curcapability);
      array = null; // free memory of array
      array = newarray; // shallow copy
      newarray = null; // NOT free memory of newarray
      curcapability = newcapability;
    }
  }
}
