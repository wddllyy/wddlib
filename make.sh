#!/bin/sh
NOW_PATH=`pwd`
#set -x

BUILD_DIR=build
BUILD_TYPE=Debug
BUILD_EXAMPLES=1



mkdir -p $BUILD_DIR/$BUILD_TYPE \
  && cd $BUILD_DIR/$BUILD_TYPE \
  && cmake \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_BUILD_EXAMPLES=$BUILD_EXAMPLES \
           $NOW_PATH \
  && make $*
  
  
if [ "$1" = "clean" ];then
    rm $NOW_PATH/build/* -rf
fi