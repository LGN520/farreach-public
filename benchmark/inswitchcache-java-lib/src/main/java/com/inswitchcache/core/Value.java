package com.inswitchcache.core;

import java.nio.charset.StandardCharsets;

/**
 * Remote DB Value structures.
 */
public class Value {
  protected static final int SIZEOF_UINT32 = Integer.BYTES;
  protected static final int SIZEOF_UINT16 = Short.BYTES;
  protected static final int SIZEOF_UINT8 = Byte.BYTES;

  private static int maxValLen = 128;
  private static int switchMaxValLen = 128;

  protected byte[] valData;
  protected int valLength;

  public Value() {
    this.valLength = 0;
    this.valData = null;
  }

  public Value(byte[] buf, int length) {
    MyUtils.myAssert(buf != null);

    this.valData = new byte[length];
    MyUtils.myAssert(this.valData != null);
    System.arraycopy(buf, 0, this.valData, 0, length);
    valLength = length;
  }

  public Value(Value other) {
    if (other.valData != null && other.valLength != 0) {
      this.valData = new byte[other.valLength];
      MyUtils.myAssert(this.valData != null);
      System.arraycopy(other.valData, 0, this.valData, 0, other.valLength);
      this.valLength = other.valLength;
    } else {
      valLength = 0;
      valData = null;
    }
  }

  public static int getSwitchMaxValLen() {
    return Value.switchMaxValLen;
  }

  public static int getMaxValLen() {
    return maxValLen;
  }

  public byte[] getValData() {
    return this.valData;
  }

  public int getBytesNum() {
    return this.valLength;
  }

  public String toString() {
    if (valData != null) {
      return new String(valData, StandardCharsets.UTF_8);
    } else {
      return "";
    }
  }

  public int fromString(String value) {
    valData = value.getBytes();
    valLength = valData.length;
    return valLength;
  }

  /**
   * Serialize value type into array of bytes.
   * 
   * @param buf
   * @param offset
   * @param maxSize
   * @return length of converted array
   */
  public int serialize(byte[] buf, int offset, int maxSize) {
    MyUtils.myAssert((valLength != 0 && valData != null) || (valLength == 0 && valData == null));

    int paddingSize = getPaddingSize(valLength);
    int serializeSize = SIZEOF_UINT16 + valLength + paddingSize;
    MyUtils.myAssert(serializeSize >= 0 && (maxSize - offset >= serializeSize));

    byte[] bigEndianValLenBytes = EndianConverter.htons((short) valLength);
    System.arraycopy(bigEndianValLenBytes, 0, buf, offset, bigEndianValLenBytes.length);
    offset += bigEndianValLenBytes.length;

    if (valLength > 0) {
      System.arraycopy(valData, 0, buf, offset, valLength);
      offset += valLength;
      if (paddingSize > 0) {
        for (int i = offset; i < offset + paddingSize; i++) {
          buf[i] = 0;
        }
      }
    }

    return serializeSize;
  }

  public int dynamicSerialize(DynamicArray dynamicData, int offset) {
    MyUtils.myAssert((valLength != 0 && valData != null) || (valLength == 0 && valData == null));

    int paddingSize = getPaddingSize(valLength);
    int serializeSize = SIZEOF_UINT16 + valLength + paddingSize;
    MyUtils.myAssert(serializeSize >= 0);

    byte[] bigEndianValLenBytes = EndianConverter.htons((short) valLength);
    dynamicData.dynamicMemcpy(offset, bigEndianValLenBytes, bigEndianValLenBytes.length);
    offset += bigEndianValLenBytes.length;

    if (valLength > 0) {
      dynamicData.dynamicMemcpy(offset, valData, valLength);
      offset += valLength;
      if (paddingSize > 0) {
        for (int i = offset; i < offset + paddingSize; i++) {
          dynamicData.set(i, (byte) 0);
        }
      }
    }

    return serializeSize;
  }

  /**
   * Deserialize array of bytes into value type.
   * 
   * @param data
   * @param offset
   * @param recvSize
   * @return
   */
  public int deserialize(byte[] data, int offset, int recvSize) {
    MyUtils.myAssert(data != null);

    // deserialize vallen
    MyUtils.myAssert(recvSize - offset >= SIZEOF_UINT16);
    byte[] valLenBytes = new byte[SIZEOF_UINT16];
    System.arraycopy(data, offset, valLenBytes, 0, SIZEOF_UINT16);
    valLength = (int) EndianConverter.ntohs(valLenBytes);
    int valLenSize = SIZEOF_UINT16;
    offset += valLenSize;

    // deserialize val
    MyUtils.myAssert(data != null);
    if (valData != null) {
      valData = null;
    }
    int valDeserializeSize;
    if (valLength > 0) {
      int paddingSize = getPaddingSize(valLength);
      int deserializeSize = valLength + paddingSize;
      MyUtils.myAssert(deserializeSize >= 0 && (recvSize - offset) > deserializeSize);

      valData = new byte[valLength];
      MyUtils.myAssert(valData != null);
      System.arraycopy(data, offset, valData, 0, valLength);
      valDeserializeSize = deserializeSize;
    } else {
      valDeserializeSize = 0;
    }
    return valLenSize + valDeserializeSize;
  }

  // for LOADREQ, GETRES_LARGEVALUE, and PUTREQ_LARGEVALUE

  public int serializeLarge(byte[] buf, int offset, int maxSize) {
    MyUtils.myAssert((valLength != 0 && valData != null) || (valLength == 0 && valData == null));

    int paddingSize = getPaddingSize(valLength);
    int serializeSize = SIZEOF_UINT32 + valLength + paddingSize;
    MyUtils.myAssert(serializeSize >= 0 && (maxSize - offset >= serializeSize));

    byte[] bigEndianValLenBytes = EndianConverter.htoni(valLength);
    System.arraycopy(bigEndianValLenBytes, 0, buf, offset, bigEndianValLenBytes.length);
    offset += bigEndianValLenBytes.length;

    if (valLength > 0) {
      System.arraycopy(valData, 0, buf, offset, valLength);
      offset += valLength;
      if (paddingSize > 0) {
        for (int i = offset; i < offset + paddingSize; i++) {
          buf[i] = 0;
        }
      }
    }

    return serializeSize;
  }

  public int dynamicSerializeLarge(DynamicArray dynamicData, int offset) {
    MyUtils.myAssert((valLength != 0 && valData != null) || (valLength == 0 && valData == null));

    int paddingSize = getPaddingSize(valLength);
    int serializeSize = SIZEOF_UINT32 + valLength + paddingSize;
    MyUtils.myAssert(serializeSize >= 0);

    byte[] bigEndianValLenBytes = EndianConverter.htoni(valLength);
    dynamicData.dynamicMemcpy(offset, bigEndianValLenBytes, bigEndianValLenBytes.length);
    offset += bigEndianValLenBytes.length;

    if (valLength > 0) {
      dynamicData.dynamicMemcpy(offset, valData, valLength);
      offset += valLength;
      if (paddingSize > 0) {
        for (int i = offset; i < offset + paddingSize; i++) {
          dynamicData.set(i, (byte) 0);
        }
      }
    }

    return serializeSize;
  }

  public int deserializeLarge(byte[] data, int offset, int recvSize) {
    MyUtils.myAssert(data != null);

    // deserialize vallen
    MyUtils.myAssert(recvSize - offset >= SIZEOF_UINT32);
    byte[] valLenBytes = new byte[SIZEOF_UINT32];
    System.arraycopy(data, offset, valLenBytes, 0, SIZEOF_UINT32);
    valLength = EndianConverter.ntohi(valLenBytes);
    int valLenSize = SIZEOF_UINT32;
    offset += valLenSize;

    // deserialize val
    MyUtils.myAssert(data != null);
    if (valData != null) {
      valData = null;
    }
    int valDeserializeSize;
    if (valLength > 0) {
      int paddingSize = getPaddingSize(valLength);
      int deserializeSize = valLength + paddingSize;
      MyUtils.myAssert(deserializeSize >= 0 && (recvSize - offset) > deserializeSize);

      valData = new byte[valLength];
      MyUtils.myAssert(valData != null);
      System.arraycopy(data, offset, valData, 0, valLength);
      valDeserializeSize = deserializeSize;
    } else {
      valDeserializeSize = 0;
    }
    return valLenSize + valDeserializeSize;
  }

  public int getPaddingSize(int vallen) {
    int paddingSize = 0;
    if (vallen <= Value.getSwitchMaxValLen()) {
      if (vallen % 8 != 0) {
        paddingSize = 8 - vallen % 8;
      }
    }
    MyUtils.myAssert(paddingSize >= 0 && paddingSize < 8);
    return paddingSize;
  }
}
