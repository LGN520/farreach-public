/**
 * Copyright (c) 2010-2016 Yahoo! Inc., 2017 YCSB contributors All rights reserved.
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You
 * may obtain a copy of the License at
 * <p>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 */

package site.ycsb;

import java.util.Properties;

import com.inswitchcache.InswitchCacheClient;
import com.inswitchcache.core.GlobalConfig;

/**
 * RemoteDB inheritates information for later evaluation.
 * RemoteDB inherits all abstract functions and variables from DB
 * which should be implemented by each database class
 */
public abstract class RemoteDB extends DB {
  protected int localLogicalClientIndex;
  protected int globalClientLogicalIndex;
  protected boolean isDynamicWorkload;

  public void init() throws DBException {
    Properties props = getProperties();
    localLogicalClientIndex = Integer.parseInt(props.getProperty(InswitchCacheClient.LOCAL_LOGICAL_CLIENT_INDEX));
    globalClientLogicalIndex = GlobalConfig.getGlobalClientLogicalIndex(localLogicalClientIndex);
    isDynamicWorkload = (GlobalConfig.getWorkloadMode() == 1)
        && (!GlobalConfig.getDynamicRulePrefix().equals("stable"));
  }

  public int getLocalLogicalClientIndex() {
    return localLogicalClientIndex;
  }
}
