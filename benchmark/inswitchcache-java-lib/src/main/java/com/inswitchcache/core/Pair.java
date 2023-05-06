package com.inswitchcache.core;

/**
 * Pair class for ScanResponseSplit.
 */
public class Pair {
  private Key key;

  private SnapshotRecord snapshotRecord;

  public Pair() {
    key = new Key();
    snapshotRecord = new SnapshotRecord();
  }

  public Pair(Key key, SnapshotRecord snapshotRecord) {
    this.key = key;
    this.snapshotRecord = snapshotRecord;
  }

  public Key getKey() {
    return key;
  }

  public SnapshotRecord getSnapshotRecord() {
    return snapshotRecord;
  }

  public void setKey(Key key) {
    this.key = key;
  }

  public void setSnapshotRecord(SnapshotRecord snapshotRecord) {
    this.snapshotRecord = snapshotRecord;
  }

}
