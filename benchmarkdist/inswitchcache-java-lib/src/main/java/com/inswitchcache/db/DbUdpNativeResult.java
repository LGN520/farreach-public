package com.inswitchcache.db;

import com.inswitchcache.core.Key;
import com.inswitchcache.core.Value;

public class DbUdpNativeResult {
  private byte[] pktContent;
  private Key key;

  private Value value;
  private int seq;
  private boolean stat;
  private short nodeIdxForEval;

  public DbUdpNativeResult() {
    this.pktContent = null;
    this.key = new Key();
    this.value = new Value();
    this.seq = 0;
    this.stat = false;
    this.nodeIdxForEval = 0;
  }

  public DbUdpNativeResult(byte[] pktContent, Key key, Value value, int seq, boolean stat, short nodeIdxForEval) {
    this.init(pktContent, key, value, seq, stat, nodeIdxForEval);
  }

  public void init(byte[] pktContent, Key key, Value value, int seq, boolean stat, short nodeIdxForEval) {
    this.pktContent = pktContent;
    this.key = key;
    this.value = value;
    this.seq = seq;
    this.stat = stat;
    this.nodeIdxForEval = nodeIdxForEval;
  }

  public byte[] getPktContent() {
    return pktContent;
  }

  public Key getKey() {
    return key;
  }

  public Value getValue() {
    return value;
  }

  public int getSeq() {
    return seq;
  }

  public boolean isStat() {
    return stat;
  }

  public short getNodeIdxForEval() {
    return nodeIdxForEval;
  }
}
