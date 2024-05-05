/**
 * Remote DB keys.
 */

package com.inswitchcache.core;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;
import java.util.zip.CRC32;

/**
 * Remote DB Key struct.
 */
public class Key implements Comparable<Key> {
  protected static final int SIZEOF_UINT32 = Integer.BYTES;
  protected static final int SIZEOF_UINT16 = Short.BYTES;
  protected static final int SIZEOF_UINT8 = Byte.BYTES;

  protected static boolean largeKey = true;

  protected int keylolo;
  protected int keylohi;
  protected int keyhilo;
  protected int keyhihi;
  protected int keylo, keyhi;

  public static int size() {
    int size = 0;
    if (largeKey) {
      size = 4 * SIZEOF_UINT32;
    } else {
      size = 2 * SIZEOF_UINT32;
    }
    return size;
  }

  public Key() {
    if (largeKey) {
      keylolo = 0;
      keylohi = 0;
      keyhilo = 0;
      keyhihi = 0;
    } else {
      keylo = 0;
      keyhi = 0;
    }
  }

  // if !largeKey
  public Key(int keyLo, int keyHi) {
    this.keylo = keyLo;
    this.keyhi = keyHi;
  }

  // if largeKey
  public Key(int keyLoLo, int keyLoHi, int keyHiLo, int keyHiHi) {
    this.keylolo = keyLoLo;
    this.keylohi = keyLoHi;
    this.keyhilo = keyHiLo;
    this.keyhihi = keyHiHi;
  }

  public Key(Key other) {
    if (largeKey) {
      this.keylolo = other.getkeylolo();
      this.keylohi = other.getkeylohi();
      this.keyhilo = other.getkeyhilo();
      this.keyhihi = other.getkeyhihi();
    } else {
      this.keylo = other.getkeylo();
      this.keyhi = other.getkeyhi();
    }
  }

  public int getkeylolo() {
    return this.keylolo;
  }

  public int getkeylohi() {
    return this.keylohi;
  }

  public int getkeyhilo() {
    return this.keyhilo;
  }

  public int getkeyhihi() {
    return this.keyhihi;
  }

  public int getkeylo() {
    return this.keylo;
  }

  public int getkeyhi() {
    return this.keyhi;
  }

  public static int compareKeys(Key l, Key r) {
    if (largeKey) {
      // System.out.println("[DEBUG][Key] l.keyhihi: " + l.keyhihi + ", l.keyhilo: " +
      // l.keyhilo
      // + ", l.keylohi: " + l.keylohi + ", l.keylolo: " + l.keylolo
      // + ", r.keyhihi: " + r.keyhihi + ", r.keyhilo: " + r.keyhilo
      // + ", r.keylohi: " + r.keylohi + ", r.keylolo: " + r.keylolo);

      if ((l.keyhihi < r.keyhihi) || ((l.keyhihi == r.keyhihi) && (l.keyhilo < r.keyhilo)) ||
          ((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi < r.keylohi)) ||
          ((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi)
              && (l.keylolo < r.keylolo))) {
        return -1;
      } else if ((l.keyhihi > r.keyhihi) || ((l.keyhihi == r.keyhihi) && (l.keyhilo > r.keyhilo)) ||
          ((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi > r.keylohi)) ||
          ((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi)
              && (l.keylolo > r.keylolo))) {
        return 1;
      } else if ((l.keyhihi == r.keyhihi) && (l.keyhilo == r.keyhilo) && (l.keylohi == r.keylohi)
          && (l.keylolo == r.keylolo)) {
        return 0;
      } else {
        return -2;
      }
    } else {
      if ((l.keyhi > r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo > r.keylo))) {
        return 1;
      } else if ((l.keyhi < r.keyhi) || ((l.keyhi == r.keyhi) && (l.keylo < r.keylo))) {
        return -1;
      } else if ((l.keyhi == r.keyhi) && (l.keylo == r.keylo)) {
        return 0;
      } else {
        return -2;
      }
    }
  }

  public static Key max() {
    Key maxKey;
    if (largeKey) {
      maxKey = new Key(Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE);
    } else {
      maxKey = new Key(Integer.MAX_VALUE, Integer.MAX_VALUE);
    }
    return maxKey;
  }

  public static Key min() {
    Key minKey;
    if (largeKey) {
      minKey = new Key(Integer.MIN_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE);
    } else {
      minKey = new Key(Integer.MIN_VALUE, Integer.MIN_VALUE);
    }
    return minKey;
  }

  public void fromString(String keystr) {
    int i = 4;
    while (keystr.charAt(i) == '0') {
      i++;
    }
    keystr = keystr.substring(i); // strip 'user' and preceeding 0's
    long keyValue = Long.parseLong(keystr);

    ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES).order(ByteOrder.LITTLE_ENDIAN);
    buffer.putLong(keyValue);
    byte[] keyValueBytes = buffer.array();

    if (largeKey) {
      byte[] keyHiLoBytes = new byte[4];
      System.arraycopy(keyValueBytes, 0, keyHiLoBytes, 0, 4);
      byte[] keyHiHiBytes = new byte[4];
      System.arraycopy(keyValueBytes, 4, keyHiHiBytes, 0, 4);

      keylolo = 0;
      keylohi = 0;
      keyhilo = ByteBuffer.wrap(keyHiLoBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
      keyhihi = ByteBuffer.wrap(keyHiHiBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
      // System.out.println(String.format("%s %x: %x %x", keystr, keyValue, keyhilo,
      // keyhihi));
    } else {
      byte[] keyLoBytes = new byte[4];
      System.arraycopy(keyValueBytes, 0, keyLoBytes, 0, 4);
      byte[] keyHiBytes = new byte[4];
      System.arraycopy(keyValueBytes, 0, keyHiBytes, 0, 4);
      keylo = ByteBuffer.wrap(keyLoBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
      keyhi = ByteBuffer.wrap(keyHiBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
    }
  }

  public String toString() {
    String result = "user";
    byte[] keyBytes = new byte[Long.BYTES];

    if (largeKey) {
      byte[] keyHiLoBytes = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keyhilo).array();
      byte[] keyHiHiBytes = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keyhihi).array();
      System.arraycopy(keyHiLoBytes, 0, keyBytes, 0, Integer.BYTES);
      System.arraycopy(keyHiHiBytes, 0, keyBytes, 4, Integer.BYTES);
    } else {
      byte[] keyLoBytes = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keylo).array();
      byte[] keyHiBytes = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keyhi).array();
      System.arraycopy(keyLoBytes, 0, keyBytes, 0, Integer.BYTES);
      System.arraycopy(keyHiBytes, 0, keyBytes, 4, Integer.BYTES);
    }
    Long keyLong = ByteBuffer.wrap(keyBytes).order(ByteOrder.LITTLE_ENDIAN).getLong();

    return result + keyLong.toString();
  }

  public int serialize(byte[] buf, int offset, int maxSize) {
    if (largeKey) {
      MyUtils.myAssert((buf != null) && (maxSize - offset) >= 16);
      byte[] bigEndianKeyLoLo = EndianConverter.htoni(keylolo);
      byte[] bigEndianKeyLoHi = EndianConverter.htoni(keylohi);
      byte[] bigEndianKeyHiLo = EndianConverter.htoni(keyhilo);
      // int bigEndianKeyHiHi = EndianConverter.htoni(keyhihi);

      short keyHihilo = (short) (keyhihi & 0xFFFF);
      short keyHihihi = (short) ((keyhihi >>> 16) & 0xFFFF);

      byte[] bigEndianKeyHiHiLo = EndianConverter.htons(keyHihilo);
      byte[] bigEndianKeyHiHiHi = EndianConverter.htons(keyHihihi);

      System.arraycopy(bigEndianKeyLoLo, 0, buf, offset, bigEndianKeyLoLo.length);
      offset += bigEndianKeyLoLo.length;
      System.arraycopy(bigEndianKeyLoHi, 0, buf, offset, bigEndianKeyLoHi.length);
      offset += bigEndianKeyLoHi.length;
      System.arraycopy(bigEndianKeyHiLo, 0, buf, offset, bigEndianKeyHiLo.length);
      offset += bigEndianKeyHiLo.length;
      // System.arraycopy(bigEndianKeyHiHi, 0, buf, offset, bigEndianKeyHiHi.length);
      // offset += bigEndianKeyHiHi.length;
      System.arraycopy(bigEndianKeyHiHiLo, 0, buf, offset, bigEndianKeyHiHiLo.length);
      offset += bigEndianKeyHiHiLo.length;
      System.arraycopy(bigEndianKeyHiHiHi, 0, buf, offset, bigEndianKeyHiHiHi.length);
      return 16;
    } else {
      MyUtils.myAssert((buf != null) && (maxSize - offset) >= 8);
      byte[] bigEndianKeyLo = EndianConverter.htoni(keylo);
      // int bigEndianKeyHi = EndianConverter.htoni(keyhi);

      short keyHiLo = (short) (keyhi & 0xFFFF);
      short keyHiHi = (short) ((keyhi >>> 16) & 0xFFFF);
      byte[] bigEndianKeyHiLo = EndianConverter.htons(keyHiLo);
      byte[] bigEndianKeyHiHi = EndianConverter.htons(keyHiHi);
      System.arraycopy(bigEndianKeyLo, offset, buf, 0, bigEndianKeyLo.length);
      offset += bigEndianKeyLo.length;
      // System.arraycopy(bigEndianKeyHi, 0, buf, 0, bigEndianKeyHi.length);
      // offset += bigEndianKeyHi.length;
      System.arraycopy(bigEndianKeyHiLo, 0, buf, offset, bigEndianKeyHiLo.length);
      offset += bigEndianKeyHiLo.length;
      System.arraycopy(bigEndianKeyHiHi, 0, buf, offset, bigEndianKeyHiHi.length);
      return 8;
    }
  }

  public int dynamicSerialize(DynamicArray dynamicData, int offset) {
    if (largeKey) {
      MyUtils.myAssert(offset >= 0);
      byte[] bigEndianKeyLoLo = EndianConverter.htoni(keylolo);
      byte[] bigEndianKeyLoHi = EndianConverter.htoni(keylohi);
      byte[] bigEndianKeyHiLo = EndianConverter.htoni(keyhilo);
      // int bigEndianKeyHiHi = EndianConverter.htoni(keyhihi);

      short keyHihilo = (short) (keyhihi & 0xFFFF);
      short keyHihihi = (short) ((keyhihi >>> 16) & 0xFFFF);

      byte[] bigEndianKeyHiHiLo = EndianConverter.htons(keyHihilo);
      byte[] bigEndianKeyHiHiHi = EndianConverter.htons(keyHihihi);

      dynamicData.dynamicMemcpy(offset, bigEndianKeyLoLo, bigEndianKeyLoLo.length);
      offset += bigEndianKeyLoLo.length;
      dynamicData.dynamicMemcpy(offset, bigEndianKeyLoHi, bigEndianKeyLoHi.length);
      offset += bigEndianKeyLoHi.length;
      dynamicData.dynamicMemcpy(offset, bigEndianKeyHiLo, bigEndianKeyHiLo.length);
      offset += bigEndianKeyHiLo.length;
      // System.arraycopy(bigEndianKeyHiHi, 0, buf, offset, bigEndianKeyHiHi.length);
      // offset += bigEndianKeyHiHi.length;
      dynamicData.dynamicMemcpy(offset, bigEndianKeyHiHiLo, bigEndianKeyHiHiLo.length);
      offset += bigEndianKeyHiHiLo.length;
      dynamicData.dynamicMemcpy(offset, bigEndianKeyHiHiHi, bigEndianKeyHiHiHi.length);
      offset += bigEndianKeyHiHiHi.length;
      return 16; // NOTE: cannot return offset, must return tmpoff - offset
    } else {
      MyUtils.myAssert(offset >= 0);
      byte[] bigEndianKeyLo = EndianConverter.htoni(keylo);
      // int bigEndianKeyHi = EndianConverter.htoni(keyhi);

      short keyHiLo = (short) (keyhi & 0xFFFF);
      short keyHiHi = (short) ((keyhi >>> 16) & 0xFFFF);
      byte[] bigEndianKeyHiLo = EndianConverter.htons(keyHiLo);
      byte[] bigEndianKeyHiHi = EndianConverter.htons(keyHiHi);
      dynamicData.dynamicMemcpy(offset, bigEndianKeyLo, bigEndianKeyLo.length);
      offset += bigEndianKeyLo.length;
      // System.arraycopy(bigEndianKeyHi, 0, buf, 0, bigEndianKeyHi.length);
      // offset += bigEndianKeyHi.length;
      dynamicData.dynamicMemcpy(offset, bigEndianKeyHiLo, bigEndianKeyHiLo.length);
      offset += bigEndianKeyHiLo.length;
      dynamicData.dynamicMemcpy(offset, bigEndianKeyHiHi, bigEndianKeyHiHi.length);
      offset += bigEndianKeyHiHi.length;
      return 8; // NOTE: cannot return offset, must return tmpoff - offset
    }
  }

  public int deserialize(byte[] data, int offset, int recvSize) {
    if (largeKey) {
      MyUtils.myAssert((data != null) && (recvSize - offset >= 16));

      byte[] keyLoloBytes = new byte[SIZEOF_UINT32];
      System.arraycopy(data, offset, keyLoloBytes, 0, SIZEOF_UINT32);
      offset += SIZEOF_UINT32;
      byte[] keyLohiBytes = new byte[SIZEOF_UINT32];
      System.arraycopy(data, offset, keyLohiBytes, 0, SIZEOF_UINT32);
      offset += SIZEOF_UINT32;
      byte[] keyHiloBytes = new byte[SIZEOF_UINT32];
      System.arraycopy(data, offset, keyHiloBytes, 0, SIZEOF_UINT32);
      offset += SIZEOF_UINT32;
      // byte[] keyHihiBytes = new byte[SIZEOF_UINT32];
      // System.arraycopy(data, offset, keyHihiBytes, 0, SIZEOF_UINT32);
      // offset += SIZEOF_UINT32;

      byte[] bigEndianKeyHiHiLo = new byte[SIZEOF_UINT16];
      System.arraycopy(data, offset, bigEndianKeyHiHiLo, 0, SIZEOF_UINT16);
      offset += SIZEOF_UINT16;
      byte[] bigEndianKeyHiHiHi = new byte[SIZEOF_UINT16];
      System.arraycopy(data, offset, bigEndianKeyHiHiHi, 0, SIZEOF_UINT16);
      offset += SIZEOF_UINT16;

      this.keylolo = EndianConverter.ntohi(keyLoloBytes);
      this.keylohi = EndianConverter.ntohi(keyLohiBytes);
      this.keyhilo = EndianConverter.ntohi(keyHiloBytes);

      int keyhihilo = EndianConverter.ntohs(bigEndianKeyHiHiLo);
      int keyhihihi = EndianConverter.ntohs(bigEndianKeyHiHiHi);
      this.keyhihi = (keyhihihi << 16) | (keyhihilo & 0xFFFF);

      return 16;
    } else {
      MyUtils.myAssert((data != null) && (recvSize - offset >= 8));

      byte[] keyLoBytes = new byte[SIZEOF_UINT32];
      System.arraycopy(data, offset, keyLoBytes, 0, SIZEOF_UINT32);
      offset += SIZEOF_UINT32;
      // byte[] keyHiBytes = new byte[SIZEOF_UINT32];
      // System.arraycopy(data, offset, keyHiBytes, 0, SIZEOF_UINT32);
      // offset += SIZEOF_UINT32;

      byte[] bigEndianKeyHiLoBytes = new byte[SIZEOF_UINT16];
      System.arraycopy(data, offset, bigEndianKeyHiLoBytes, 0, SIZEOF_UINT16);
      offset += SIZEOF_UINT16;
      byte[] bigEndianKeyHiHiBytes = new byte[SIZEOF_UINT16];
      System.arraycopy(data, offset, bigEndianKeyHiHiBytes, 0, SIZEOF_UINT16);
      offset += SIZEOF_UINT16;

      this.keylo = EndianConverter.ntohs(keyLoBytes);
      // this.keyhi = EndianConverter.ntohs(keyHiBytes);
      int keyHiLo = EndianConverter.ntohs(bigEndianKeyHiLoBytes);
      int keyHiHi = EndianConverter.ntohs(bigEndianKeyHiHiBytes);
      this.keyhi = (keyHiHi << 16) | (keyHiLo & 0xFFFF);
      return 8;
    }
  }

  public int bufferSerialize(ByteBuffer data) throws BufferUnderflowException, IndexOutOfBoundsException {
    if (largeKey) {
      byte[] bigEndianKeyLoLo = EndianConverter.htoni(keylolo);
      byte[] bigEndianKeyLoHi = EndianConverter.htoni(keylohi);
      byte[] bigEndianKeyHiLo = EndianConverter.htoni(keyhilo);
      // int bigEndianKeyHiHi = EndianConverter.htoni(keyhihi);

      short keyHihilo = (short) (keyhihi & 0xFFFF);
      short keyHihihi = (short) ((keyhihi >>> 16) & 0xFFFF);

      byte[] bigEndianKeyHiHiLo = EndianConverter.htons(keyHihilo);
      byte[] bigEndianKeyHiHiHi = EndianConverter.htons(keyHihihi);

      data.put(bigEndianKeyLoLo);
      data.put(bigEndianKeyLoHi);
      data.put(bigEndianKeyHiLo);
      data.put(bigEndianKeyHiHiLo);
      data.put(bigEndianKeyHiHiHi);
      return 16;
    } else {
      byte[] bigEndianKeyLo = EndianConverter.htoni(keylo);
      // int bigEndianKeyHi = EndianConverter.htoni(keyhi);

      short keyHiLo = (short) (keyhi & 0xFFFF);
      short keyHiHi = (short) ((keyhi >>> 16) & 0xFFFF);
      byte[] bigEndianKeyHiLo = EndianConverter.htons(keyHiLo);
      byte[] bigEndianKeyHiHi = EndianConverter.htons(keyHiHi);

      data.put(bigEndianKeyLo);
      data.put(bigEndianKeyHiLo);
      data.put(bigEndianKeyHiHi);
      return 8;
    }
    // return -1; // never arrive here
  }

  public int bufferDeserialize(ByteBuffer data) throws BufferUnderflowException, IndexOutOfBoundsException {
    if (largeKey) {
      byte[] keyLoloBytes = new byte[SIZEOF_UINT32];
      data.get(keyLoloBytes);
      byte[] keyLohiBytes = new byte[SIZEOF_UINT32];
      data.get(keyLohiBytes);
      byte[] keyHiloBytes = new byte[SIZEOF_UINT32];
      data.get(keyHiloBytes);

      byte[] bigEndianKeyHiHiLo = new byte[SIZEOF_UINT16];
      data.get(bigEndianKeyHiHiLo);
      byte[] bigEndianKeyHiHiHi = new byte[SIZEOF_UINT16];
      data.get(bigEndianKeyHiHiHi);

      this.keylolo = EndianConverter.ntohi(keyLoloBytes);
      this.keylohi = EndianConverter.ntohi(keyLohiBytes);
      this.keyhilo = EndianConverter.ntohi(keyHiloBytes);

      int keyhihilo = EndianConverter.ntohs(bigEndianKeyHiHiLo);
      int keyhihihi = EndianConverter.ntohs(bigEndianKeyHiHiHi);
      this.keyhihi = (keyhihihi << 16) | (keyhihilo & 0xFFFF);

      return 16;
    } else {
      byte[] keyLoBytes = new byte[SIZEOF_UINT32];
      data.get(keyLoBytes);

      byte[] bigEndianKeyHiLoBytes = new byte[SIZEOF_UINT16];
      data.get(bigEndianKeyHiLoBytes);
      byte[] bigEndianKeyHiHiBytes = new byte[SIZEOF_UINT16];
      data.get(bigEndianKeyHiHiBytes);

      this.keylo = EndianConverter.ntohs(keyLoBytes);
      // this.keyhi = EndianConverter.ntohs(keyHiBytes);
      int keyHiLo = EndianConverter.ntohs(bigEndianKeyHiLoBytes);
      int keyHiHi = EndianConverter.ntohs(bigEndianKeyHiHiBytes);
      this.keyhi = (keyHiHi << 16) | (keyHiLo & 0xFFFF);
      return 8;
    }
    // return -1; // never arrive here
  }

  public int getHashPartitionIdx(int partitionNum, int serverNum) {
    byte[] buffer = new byte[16];
    int tmpKeySize = this.serialize(buffer, 0, 16);
    CRC32 crc = new CRC32();
    crc.update(buffer);

    long hashResultLong = crc.getValue() % partitionNum;
    int hashResult = (int) hashResultLong;
    int targetIdx = hashResult / (partitionNum / serverNum);
    if (targetIdx == serverNum)
      targetIdx--;
    return targetIdx;
  }

  public int getRangePartitionIdx(int serverNum) {
    int targetIdx;
    if (largeKey) {
      int keyHiHiHi = (this.keyhihi >>> 16) & 0xFFFF;
      targetIdx = keyHiHiHi / (64 * 1024 / serverNum);
    } else {
      int keyHiHi = (this.keyhi >>> 16) & 0xFFFF;
      targetIdx = keyHiHi / (64 * 1024 / serverNum);
    }
    return targetIdx;
  }

  @Override
  public int compareTo(Key otherkey) {
    return compareKeys(this, otherkey);
  }

  @Override
  public boolean equals(Object o) {
    // System.out.println("called key equals");
    // if (o instanceof Key) {
    // Key otherkey = (Key) o;
    // return compareKeys(this, otherkey) == 0;
    // } else {
    // return false;
    // }

    if (!(o instanceof Key)) {
      return false;
    }
    Key otherkey = (Key) o;
    if (largeKey) {
      if ((keyhihi == otherkey.keyhihi) && (keyhilo == otherkey.keyhilo) && (keylohi == otherkey.keylohi)
          && (keylolo == otherkey.keylolo)) {
        return true;
      } else {
        return false;
      }
    } else {
      if ((keyhi == otherkey.keyhi) && (keylo == otherkey.keylo)) {
        return true;
      } else {
        return false;
      }
    }

  }

  @Override
  public int hashCode() {
    // byte[] keyBytes = new byte[Long.BYTES];
    // if (largeKey) {
    // byte[] keyHiLoBytes =
    // ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keyhilo).array();
    // byte[] keyHiHiBytes =
    // ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keyhihi).array();
    // System.arraycopy(keyHiLoBytes, 0, keyBytes, 0, Integer.BYTES);
    // System.arraycopy(keyHiHiBytes, 0, keyBytes, 4, Integer.BYTES);
    // } else {
    // byte[] keyLoBytes =
    // ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keylo).array();
    // byte[] keyHiBytes =
    // ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(keyhi).array();
    // System.arraycopy(keyLoBytes, 0, keyBytes, 0, Integer.BYTES);
    // System.arraycopy(keyHiBytes, 0, keyBytes, 4, Integer.BYTES);
    // }
    // Long keyLong =
    // ByteBuffer.wrap(keyBytes).order(ByteOrder.LITTLE_ENDIAN).getLong();
    // return keyLong.intValue();

    int hashcode = 0;
    if (largeKey) {
      hashcode = Objects.hash(keylolo, keylohi, keyhilo, keyhihi);
    } else {
      hashcode = Objects.hash(keylo, keyhi);
    }
    return hashcode;
  }

  public Key getEndkey(int recordCnt) {
    Key result = null;

    // if (largeKey) {
    // result = new Key(keylolo, keylohi, keyhilo, (((keyhihi >>> 16) & 0xFFFF) +
    // recordCnt) << 16);
    // } else {
    // result = new Key(keylo, (((keyhi >>> 16) & 0xFFFF) + recordCnt) << 16);
    // }

    if (largeKey) {
      result = new Key(keylolo + recordCnt, keylohi, keyhilo, keyhihi);
    } else {
      result = new Key(keylo + recordCnt, keyhi);
    }

    return result;
  }
}
