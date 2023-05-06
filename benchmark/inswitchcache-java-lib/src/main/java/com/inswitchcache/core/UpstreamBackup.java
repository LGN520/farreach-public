package com.inswitchcache.core;

public class UpstreamBackup {
  private Value value;
  private int seq;
  private boolean stat;

  public UpstreamBackup(Value value, int seq, boolean stat) {
    this.value = value;
    this.seq = seq;
    this.stat = stat;
  }

  public Value getValue() {
    return value;
  }

  public void setValue(Value value) {
    this.value = value;
  }

  public int getSeq() {
    return seq;
  }

  public void setSeq(int seq) {
    this.seq = seq;
  }

  public boolean isStat() {
    return stat;
  }

  public void setStat(boolean stat) {
    this.stat = stat;
  }
}
