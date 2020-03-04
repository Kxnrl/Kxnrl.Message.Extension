#!/bin/sh

EXT_DIR=$(pwd)

git clone https://github.com/alliedmodders/sourcemod --recursive --branch $BRANCH --single-branch $(pwd)/sourcemod-$BRANCH

mkdir build
pushd build
python ../configure.py --enable-optimize --sm-path $EXT_DIR && ambuild
popd
