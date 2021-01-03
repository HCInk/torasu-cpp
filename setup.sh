#!/bin/sh
git submodule update --init
git config --local core.hooksPath .githooks
