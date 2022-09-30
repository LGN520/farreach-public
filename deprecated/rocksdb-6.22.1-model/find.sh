#!/usr/bin/env bash

find . -name "*.h" -type f | xargs grep "$1"
find . -name "*.cc" -type f | xargs grep "$1"
