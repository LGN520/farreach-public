package com.inswitchcache.core;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;

import com.inswitchcache.core.packets.Packet;
import com.inswitchcache.core.packets.PacketType;

public class InmemoryReq {
  private short type;
  private Key key;

  private int recordCnt;
  // private Key endkey;

  // NOTE: use int vallen instead of Value value to save disk space of
  // pre-generated workload files -> around 20GB for server rotation with 128
  // servers
  private int vallen;
  // private Value value;

  public InmemoryReq() {
    this.type = 0;
    this.key = new Key();
    // this.endkey = new Key();
    this.recordCnt = 0;
    // this.value = new Value();
    this.vallen = 0;
  }

  public InmemoryReq(short type, Key key) {
    this.type = type;
    this.key = key;
    // this.endkey = new Key();
    this.recordCnt = 0;
    // this.value = new Value();
    this.vallen = 0;
    MyUtils.myAssert(type == PacketType.GETREQ.getType() || type == PacketType.DELREQ.getType());
  }

  // public InmemoryReq(short type, Key key, Key endkey) {
  public InmemoryReq(short type, Key key, int recordCnt) {
    this.type = type;
    this.key = key;
    // this.endkey = endkey;
    this.recordCnt = recordCnt;
    // this.value = new Value();
    this.vallen = 0;
    MyUtils.myAssert(type == PacketType.SCANREQ.getType());
  }

  public InmemoryReq(short type, Key key, Value value) {
    this.type = type;
    this.key = key;
    // this.endkey = new Key();
    this.recordCnt = 0;
    // this.value = value;
    this.vallen = value.getValData().length;
    MyUtils.myAssert(type == PacketType.PUTREQ.getType());
  }

  public static int size(short type) {
    int size = Packet.SIZEOF_UINT16 + Key.size();
    if (type == PacketType.PUTREQ.getType()) {
      // size += Packet.SIZEOF_UINT32 + value.getValData().length;
      size += Packet.SIZEOF_UINT32;
    } else if (type == PacketType.SCANREQ.getType()) {
      // size += Key.size();
      size += Packet.SIZEOF_UINT32;
    }
    return size;
  }

  public int serialize(byte[] data, int maxSize) {
    int offset = 0;

    // type
    byte[] bigEndianType = EndianConverter.htons(this.type);
    int tmpTypeSize = bigEndianType.length;
    MyUtils.myAssert(maxSize - offset >= tmpTypeSize);
    System.arraycopy(bigEndianType, 0, data, offset, tmpTypeSize);
    offset += tmpTypeSize;

    // key
    int tmpKeySize = this.key.serialize(data, offset, maxSize);
    offset += tmpKeySize;

    if (this.type == PacketType.PUTREQ.getType()) {
      // NOTE: value could > 128B
      // int tmpValSize = this.value.serializeLarge(data, offset, maxSize);
      // offset += tmpValSize;

      byte[] bigEndianVallen = EndianConverter.htoni(this.vallen);
      int tmpVallenSize = bigEndianVallen.length;
      MyUtils.myAssert(maxSize - offset >= tmpVallenSize);
      System.arraycopy(bigEndianVallen, 0, data, offset, tmpVallenSize);
      offset += tmpVallenSize;
    } else if (this.type == PacketType.SCANREQ.getType()) {
      // int tmpEndkeySize = this.endkey.serialize(data, offset, maxSize);
      // offset += tmpEndkeySize;

      byte[] bigEndianRecordCnt = EndianConverter.htoni(this.recordCnt);
      int tmpRecordCntSize = bigEndianRecordCnt.length;
      MyUtils.myAssert(maxSize - offset >= tmpRecordCntSize);
      System.arraycopy(bigEndianRecordCnt, 0, data, offset, tmpRecordCntSize);
      offset += tmpRecordCntSize;
    }

    return offset;
  }

  public int bufferSerialize(ByteBuffer data) throws BufferUnderflowException, IndexOutOfBoundsException {
    int offset = 0;

    // type
    byte[] bigEndianType = EndianConverter.htons(this.type);
    int tmpTypeSize = bigEndianType.length;
    data.put(bigEndianType);
    offset += tmpTypeSize;

    // key
    int tmpKeySize = this.key.bufferSerialize(data);
    offset += tmpKeySize;

    if (this.type == PacketType.PUTREQ.getType()) {
      // NOTE: value could > 128B
      // int tmpValSize = this.value.bufferSerializeLarge(data, offset, maxSize);
      // offset += tmpValSize;

      byte[] bigEndianVallen = EndianConverter.htoni(this.vallen);
      int tmpVallenSize = bigEndianVallen.length;
      data.put(bigEndianVallen);
      offset += tmpVallenSize;
    } else if (this.type == PacketType.SCANREQ.getType()) {
      // int tmpEndkeySize = this.endkey.bufferSerialize(tmpreqBytes, offset,
      // maxSize);
      // offset += tmpEndkeySize;

      byte[] bigEndianRecordCnt = EndianConverter.htoni(this.recordCnt);
      int tmpRecordCntSize = bigEndianRecordCnt.length;
      data.put(bigEndianRecordCnt);
      offset += tmpRecordCntSize;
    }

    return offset;
  }

  public int bufferDeserialize(ByteBuffer data) throws BufferUnderflowException, IndexOutOfBoundsException {
    int offset = 0;

    // type
    int tmpTypeSize = Packet.SIZEOF_UINT16;
    byte[] bigEndianTypeBytes = new byte[tmpTypeSize];
    data.get(bigEndianTypeBytes);
    this.type = EndianConverter.ntohs(bigEndianTypeBytes);
    offset += tmpTypeSize;

    // key
    int tmpKeySize = this.key.bufferDeserialize(data);
    offset += tmpKeySize;

    if (this.type == PacketType.PUTREQ.getType()) {
      // NOTE: value could > 128B
      // int tmpValSize = this.value.bufferDeserializeLarge(tmpreqBytes, offset,
      // maxSize);
      // offset += tmpValSize;

      int tmpVallenSize = Packet.SIZEOF_UINT32;
      byte[] bigEndianVallenBytes = new byte[tmpVallenSize];
      data.get(bigEndianVallenBytes);
      this.vallen = EndianConverter.ntohi(bigEndianVallenBytes);
      offset += tmpVallenSize;
    } else if (this.type == PacketType.SCANREQ.getType()) {
      // int tmpEndkeySize = this.endkey.deserialize(tmpreqBytes, offset, maxSize);
      // offset += tmpEndkeySize;

      int tmpRecordCntSize = Packet.SIZEOF_UINT32;
      byte[] bigEndianRecordCntBytes = new byte[tmpRecordCntSize];
      data.get(tmpRecordCntSize);
      this.recordCnt = EndianConverter.ntohi(bigEndianRecordCntBytes);
      offset += tmpRecordCntSize;
    }

    return offset;
  }

  public short getType() {
    return type;
  }

  public Key getKey() {
    return key;
  }

  // public Value getValue() {
  // //return value;
  // byte[] tmpvalBytes = new byte[this.vallen];
  // Arrays.fill(tmpvalBytes, (byte) 255);
  // Value tmpval = new Value(tmpvalBytes, this.vallen);
  // return tmpval;
  // }

  public int getVallen() {
    return vallen;
  }

  public int getRecordCnt() {
    return recordCnt;
  }
}
