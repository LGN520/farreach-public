/**
 * Farreach client binding for YCSB.
 */

package com.inswitchcache.core;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;

/**
 * Socket helper for Farreach client transmission.
 */
public final class JavaSocketHelper {
  public static final int MAX_TRIAL = 3;

  private JavaSocketHelper() {
    // not used
  }

  public static byte[] sendUdpSocket(DatagramSocket socket, InetAddress server, short serverPort, byte[] buffer,
      int reqSize) {
    DatagramPacket sendPacket = new DatagramPacket(buffer, reqSize, server, serverPort);
    int numOfTrial = 0;
    while (numOfTrial < MAX_TRIAL) {
      boolean isTimeout = false;
      try {
        socket.send(sendPacket);
      } catch (IOException e) {
        System.out.println("Sending packet IOException, resent");
        numOfTrial++;
        continue;
      }

      while (true) {
        try {
          byte[] recvBuffer = new byte[GlobalConfig.MAX_BUFFER_SIZE];
          DatagramPacket recvPacket = new DatagramPacket(recvBuffer, recvBuffer.length);
          socket.receive(recvPacket);
          if (recvPacket.getData().length < 0) {
            // invalid received data, check the next one
            continue;
          } else {
            byte[] recvData = recvPacket.getData();
            return recvData;
          }
        } catch (SocketTimeoutException e) {
          System.out.println("Receive response timeout");
          isTimeout = true;
          break;
        } catch (IOException e) {
          System.out.println("Receiving response IOException");
          isTimeout = true;
        }
      }
      if (isTimeout) {
        // error receiving data, resend packet
        numOfTrial++;
        continue;
      }
    }

    return null;
  }
}