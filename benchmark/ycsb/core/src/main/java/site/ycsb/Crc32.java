/**
 * Copyright (c) 2012 YCSB contributors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You
 * may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 */

/**
 * Manually implemented Crc32 Algorithm class.
 */

package site.ycsb;

/**
 * Crc32 helper funciton.
 */
public final class Crc32 {
  private Crc32() {
    // not used;
  }

  public static int crc32(byte[] message, int size) {
    int i, j;
    int by, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    while (true) {
      by = message[i];

      crc = crc ^ by;
      for (j = 7; j >= 0; j--) {
        mask = -(crc & 1);
        crc = (crc >>> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
      if (i == size) {
        break;
      }
    }
    return ~crc;
  }

  public static long getUnsigned(int signed) {
    return signed >= 0 ? signed : 2 * (long) Integer.MAX_VALUE + 2 + signed;
  }
}