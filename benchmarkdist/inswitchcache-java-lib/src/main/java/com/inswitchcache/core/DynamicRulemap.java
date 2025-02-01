package com.inswitchcache.core;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Scanner;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Class for saving mapping rules under dynamic workloads.
 */
public final class DynamicRulemap {
  private AtomicInteger periodidx = new AtomicInteger();
  private ArrayList<HashMap<Key, Key>> mappings = new ArrayList<HashMap<Key, Key>>();

  public DynamicRulemap(int periodnum, String rulepath) {
    mappings.clear();
    Scanner tmpScanner;
    for (int i = 0; i < periodnum; i++) {
      String tmpRulefilePath = String.format("%s%d.out", rulepath, i);
      System.out.println("[DEBUG][DynamicRulemap] Read rulemap file from path: " + tmpRulefilePath);
      mappings.add(new HashMap<Key, Key>());
      try {
        File tmpRulefile = new File(tmpRulefilePath);
        tmpScanner = new Scanner(tmpRulefile);
        while (tmpScanner.hasNextLine()) {
          String tmpLine = tmpScanner.nextLine();
          String[] tmpKeystrs = tmpLine.split(" ");
          MyUtils.myAssert(tmpKeystrs.length == 2);

          Key tmpOriginalKey = new Key();
          tmpOriginalKey.fromString(tmpKeystrs[0]);
          Key tmpNewKey = new Key();
          tmpNewKey.fromString(tmpKeystrs[1]);

          mappings.get(i).put(tmpOriginalKey, tmpNewKey);
        }
        tmpScanner.close();
      } catch (FileNotFoundException e) {
        e.printStackTrace();
        System.exit(-1);
      }
    }

    periodidx.set(-1);
  }

  // try mapping of current period
  public Key trymap(Key originalkey) {
    int tmpidx = periodidx.get();
    if (!mappings.get(tmpidx).containsKey(originalkey)) {
      return originalkey;
    } else {
      return mappings.get(tmpidx).get(originalkey);
    }
  }

  // move to next period
  public boolean nextperiod() {
    int tmpidx = periodidx.incrementAndGet(); // increase by 1 atomically, and return updated value
    return (tmpidx >= 0 && tmpidx < mappings.size());
  }
}
