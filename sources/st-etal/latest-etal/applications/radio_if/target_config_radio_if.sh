#!/bin/bash
# Set WSPATH to work space directory
#export WSPATH=../../..

# Set BUILDPATH to current directory
#export BUILDPATH=./

../../tools/linux/gconf/src/gconf ./target_config_radio_if.cml ./target_config_radio_if.def ./target_config_radio_if.mak ./target_config_radio_if.h

# Delete temporary config file
rm -f .kconfig.d
