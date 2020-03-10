#!/bin/bash

set -e

EXT_DIR=$(pwd)

git clone https://github.com/alliedmodders/sourcemod --recursive --branch $BRANCH --single-branch "$EXT_DIR/sourcemod-$BRANCH"

mkdir -p "$EXT_DIR/build"
pushd "$EXT_DIR/build"
python "$EXT_DIR/configure.py" --enable-optimize --sm-path "$EXT_DIR/sourcemod-$BRANCH"
ambuild
popd
