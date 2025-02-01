package com.inswitchcache.core;

/**
 * UDP addrlen for jni-based socket.
 */
public class UdpAddr {
  private String ipAddr = "";
  private short udpPort = 0;

  public UdpAddr() {
    ipAddr = "";
    udpPort = 0;
  }

  public UdpAddr(String tmpipAddr, short tmpudpPort) {
    ipAddr = tmpipAddr;
    udpPort = tmpudpPort;
  }

  public String getIpAddr() {
    return ipAddr;
  }

  public void setIpAddr(String tmpipAddr) {
    ipAddr = tmpipAddr;
  }

  public short getUdpPort() {
    return udpPort;
  }

  public void setUdpPort(short tmpudpPort) {
    udpPort = tmpudpPort;
  }
}
