#!/bin/sh

EXT_DIR=$(pwd)

git clone https://github.com/alliedmodders/sourcemod --recursive --branch $BRANCH --single-branch $EXT_DIR/sourcemod-$BRANCH

mkdir build
pushd build
python ../configure.py --enable-optimize --sm-path $EXT_DIR/sourcemod-$BRANCH && ambuild
popd
