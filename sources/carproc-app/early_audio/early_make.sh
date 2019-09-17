#!/bin/sh

# Configure DSP effects and save DSP memories
amixer -c3 set "Scaler Volume Master" 0
amixer -c3 set "Scaler Volume Primary Left" 1200
amixer -c3 set "Scaler Volume Primary Right" 1200
amixer -c3 set "Scaler Volume Secondary Left" 1200
amixer -c3 set "Scaler Volume Secondary Right" 1200
amixer -c3 set "Scaler Volume Generator Left" 1200
amixer -c3 set "Scaler Volume Generator Right" 1200
amixer -c3 set "Volume Master" 1200
sleep 1
mem-access d 0x48980000 0x4000
mv /home/root/reg_dump.bin emerald-firmware.X0.noheader
mem-access d 0x489c0000 0x4000
mv /home/root/reg_dump.bin emerald-firmware.Y0.noheader
mem-access d 0x48a80000 0x4000
mv /home/root/reg_dump.bin emerald-firmware.X1.noheader
mem-access d 0x48ac0000 0x4000
mv /home/root/reg_dump.bin emerald-firmware.Y1.noheader

# Configure Source and save AIF registers
amixer -c3 set Source sai4rx1fm
cat /sys/devices/platform/soc/48d00000.codec_aif/aif_dump > sai4rx1fm
early_make --source sai4rx1fm > early

amixer -c3 set Source auxmedia
cat /sys/devices/platform/soc/48d00000.codec_aif/aif_dump > auxmedia
early_make --source auxmedia >> early

# Save early audio functions
echo 1 > /sys/devices/platform/soc/48900000.codec_dsp/trace
amixer -c3 set Beep On
cat /proc/sta/trace >beep
echo 0 > /sys/devices/platform/soc/48900000.codec_dsp/trace
early_make --trace beep >> early
sleep 2

echo 1 > /sys/devices/platform/soc/48900000.codec_dsp/trace
amixer -c3 set "Belt Chime Repeat" 1
amixer -c3 set "Belt Chime Ramp Amp" 1000,1000,500,0,0,0,0
amixer -c3 set "Belt Chime Ramp Dur" 5,80,100,200,0,0,0
amixer -c3 set "Belt Chime" On
echo 0 > /sys/devices/platform/soc/48900000.codec_dsp/trace
cat /proc/sta/trace >chime
early_make --trace chime >> early

echo 1 > /sys/devices/platform/soc/48900000.codec_dsp/trace
amixer -c3 set "Equalizer Mode" Flat
amixer -c3 set Equalizer On
echo 0 > /sys/devices/platform/soc/48900000.codec_dsp/trace
cat /proc/sta/trace >flat
early_make --trace flat >> early
amixer -c3 set Equalizer Off

echo 1 > /sys/devices/platform/soc/48900000.codec_dsp/trace
amixer -c3 set "Equalizer Mode" Classic
amixer -c3 set Equalizer On
echo 0 > /sys/devices/platform/soc/48900000.codec_dsp/trace
cat /proc/sta/trace >classic
early_make --trace classic >> early
amixer -c3 set Equalizer Off

echo 1 > /sys/devices/platform/soc/48900000.codec_dsp/trace
amixer -c3 set "Equalizer Mode" Pop
amixer -c3 set Equalizer On
echo 0 > /sys/devices/platform/soc/48900000.codec_dsp/trace
cat /proc/sta/trace >pop
early_make --trace pop >> early
amixer -c3 set Equalizer Off

echo 1 > /sys/devices/platform/soc/48900000.codec_dsp/trace
amixer -c3 set "Equalizer Mode" Rock
amixer -c3 set Equalizer On
echo 0 > /sys/devices/platform/soc/48900000.codec_dsp/trace
cat /proc/sta/trace >rock
early_make --trace rock >> early
amixer -c3 set Equalizer Off

echo 1 > /sys/devices/platform/soc/48900000.codec_dsp/trace
amixer -c3 set "Equalizer Mode" Jazz
amixer -c3 set Equalizer On
echo 0 > /sys/devices/platform/soc/48900000.codec_dsp/trace
cat /proc/sta/trace >jazz
early_make --trace jazz >> early
amixer -c3 set Equalizer Off

# Save infos of dsp modules
cat /sys/devices/platform/soc/48900000.codec_dsp/modinfo >modinfo
early_make --modules modinfo >> early
