#!/bin/bash -
amixer -c 3 sset Source adcauxdac > /dev/null
amixer -c 3 sset "Scaler Primary Media Volume Master" 1200 > /dev/null
amixer -c 3 sset "Volume Master" 1200 > /dev/null
amixer -c 3 sset "ADCAUX CHSEL" 0 > /dev/null
cd /usr/bin
st_dab_radio
