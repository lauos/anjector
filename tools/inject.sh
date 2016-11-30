#!/bin/bash

CUR_DIR=$(cd $(dirname $0); pwd)
ROOT_DIR=$(cd $CUR_DIR/../; pwd)
NDK_HOME=$(dirname $(which ndk-build))
BUILD_ABI=$1

echo $NDK_HOME
echo $ROOT_DIR

if [ x$BUILD_ABI = x ]; then
    BUILD_ABI=armeabi-v7a 
fi

make -C $ROOT_DIR push ABI=$BUILD_ABI

echo "INEJCT BEGIN"

COUNT=$(adb shell ps | awk '{print $9}' | wc -l)
echo $COUNT

for (( i = 1; i <= $COUNT; i++ )); do
    APP=$(adb shell ps | awk '{print $9}' | sed -n "${i}p")
    if [ x"com." = x${APP:0:4} ]; then
        echo "inject $i $APP"
        adb shell /data/local/tmp/anjector -n $APP -b /data/local/tmp/libbridge.so
    fi
done

echo "INJECT END"
