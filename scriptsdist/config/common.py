#!/usr/bin/env python3
import os
import sys
import configparser

# sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from .globalvar import *


class CommonConfig:
    def __init__(self, DIRNAME):
        self.dirname = DIRNAME
        ##### method-related variables #####
        self.with_controller = (
            self.dirname == "farreach"
            or self.dirname == "netcache"
            or self.dirname == "distreach"
            or self.dirname == "distcache"
        )
        self.with_reflector = (
            self.dirname == "distreach" or self.dirname == "distcache"
        )
        self.is_distributed = (
            self.dirname == "distreach"
            or self.dirname == "distcache"
            or self.dirname == "distnocache"
        )

        ##### Parse ${DRINAME}/config.ini #####

        configfile = "{CLIENT_ROOTPATH}/{DIRNAME}/config.ini".format(
            CLIENT_ROOTPATH=CLIENT_ROOTPATH, DIRNAME=self.dirname
        )
        print(
            "[COMMON] load configuration from {configfile}".format(
                configfile=configfile
            )
        )
        config = configparser.ConfigParser()
        with open(configfile, "r") as f:
            config.read_file(f)
        # # for server rotation (must be consistent with each config.ini)

        self.workloadname = config.get("global", "workload_name")
        self.workloadmode = int(config.get("global", "workload_mode"))
        self.dynamicpattern = config.get("global", "dynamic_ruleprefix")
        self.bottleneck_serveridx = int(
            config.get("global", "bottleneck_serveridx_for_rotation")
        )
        self.server_total_logical_num = int(
            config.get("global", "server_total_logical_num")
        )
        self.server_total_logical_num_for_rotation = int(
            config.get("global", "server_total_logical_num_for_rotation")
        )

        self.snapshot_period = 0
        self.cache_size = 0
        if self.dirname == "farreach" or self.dirname == "distreach":
            self.snapshot_period = int(
                config.get("controller", "controller_snapshot_period")
            )  # in units of ms
            self.cache_size = int(config.get("switch", "switch_kv_bucket_num"))

        self.is_common_included = 1
