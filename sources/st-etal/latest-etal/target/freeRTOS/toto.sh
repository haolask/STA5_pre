#! /bin/sh

cp target/freeRTOS/target_config.h  target/freeRTOS/CMakeLists.txt

f=target/freeRTOS/CMakeLists.txt
sed -i 's/^#define \([A-Z_0-9]*\) \([0-9]*\)\?/set(\1 \2)/g' $f
sed -i 's/^#define \([A-Z_0-9]*\)/set(\1)/g' $f
sed -i 's/^#undef \([A-Z_0-9]*\)/unset(\1)/g' $f
sed -i 's#//.*##g' $f
sed -i 's#^/\*##g' $f
sed -i 's#^ \*.*##g' $f
sed -i '/^\s*$/d' $f
