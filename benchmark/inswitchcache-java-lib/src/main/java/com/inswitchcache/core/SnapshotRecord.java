package com.inswitchcache.core;

/**
 * Snapshot record class for ScanResponseSplit.
 */
public class SnapshotRecord {
  private Value val;
  private int seq;
  private boolean stat;

  public Value getVal() {
    return val;
  }

  public int getSeq() {
    return seq;
  }

  public boolean getStat() {
    return stat;
  }

  public SnapshotRecord() {
    val = new Value();
    seq = 0;
    stat = false;
  }

  public SnapshotRecord(Value tmpval, int tmpseq, boolean tmpstat) {
    val = tmpval;
    seq = tmpseq;
    stat = tmpstat;
  }

  public SnapshotRecord(SnapshotRecord other) {
    val = other.val;
    seq = other.seq;
    stat = other.stat;
  }
}
