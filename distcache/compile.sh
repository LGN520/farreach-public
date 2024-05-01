#!/bin/bash
cd leafswitch
p4c-bm2-ss --p4v 16 netcache.p4 -o netcache.json
cd ..
cd spineswitch
p4c-bm2-ss --p4v 16 spineswitch.p4 -o spineswitch.json
cd clientrackswitch
p4c-bm2-ss --p4v 16 partitionswitch.p4 -o partitionswitch.json