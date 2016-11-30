#!/bin/sh

CUR_DIR=$(cd $(dirname $0); pwd)
ROOT_DIR=$(cd $CUR_DIR/../; pwd)
NDK_HOME=$(dirname $(which ndk-build))
PROG_NAME=`basename $1`
BUILD_ABI=$2

echo $NDK_HOME
echo $ROOT_DIR

if [ x$PROG_NAME = x ]; then
    echo "Usage: $0 name_of_prog  abi_to_build"
    exit
fi


if [ x$BUILD_ABI = x ]; then
    BUILD_ABI=armeabi-v7a 
fi

make -C $ROOT_DIR push_debug ABI=$BUILD_ABI

echo "DEBUG BEGIN"

adb shell /data/local/tmp/gdbserver :12345 /data/local/tmp/$PROG_NAME &

adb forward tcp:12345 tcp:12345

echo "target remote:12345" >> $ROOT_DIR/libs/$BUILD_ABI/gdb.setup

$NDK_HOME/prebuilt/linux-x86_64/bin/gdb -x $ROOT_DIR/libs/$BUILD_ABI/gdb.setup

SERVER_PID=$(echo $(adb shell ps | grep -i /data/local/tmp/gdbserver) | awk '{print $2}')
PROG_PID=$(echo $(adb shell ps | grep -i /data/local/tmp/$PROG_NAME) | awk '{print $2}')

if [ x$SERVER_PID = x ]; then
    echo "gdbserver has exit"
else
    adb shell kill -KILL $SERVER_PID
fi

if [ x$PROG_PID = x ]; then
    echo "$PROG_NAME has exit"
else
    adb shell kill -KILL $PROG_PID
fi

echo "DEBUG END"
