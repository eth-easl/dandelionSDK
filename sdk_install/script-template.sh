#!/bin/bash

set -e

CLANG_NAME="clang"
DEFAULT_CLANG=false

# option c sets the clang name to look for
# option d makes it so the finished clang is installed as default clang, assuming there is no default clang yet
while getopts "c:d" opt; do
    case "$opt" in
        c)
        echo "Looking for clang at $OPTARG"
        CLANG_NAME="$OPTARG"
        ;;
        d)
        echo "Trying to create default clang"
        DEFAULT_CLANG=true
        ;;
        ?)
        echo "Unkown argument to scipt"
        ;;
    esac
done

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

COMPILER_INCLUDES=$($CLANG_NAME -print-file-name=include)
sed -i "s|COMPILER_INCLUDES|$COMPILER_INCLUDES|g" @DANDELION_TARGET@-clang.cfg

CLANG_PATH="$(which $CLANG_NAME)"
CLANG_DIR="$(dirname $CLANG_PATH)"

if ! [ -f @DANDELION_TARGET@-clang ]; then
    cp $CLANG_PATH @DANDELION_TARGET@-clang
    ln -s @DANDELION_TARGET@-clang @DANDELION_TARGET@-clang++
fi

if [ $DEFAULT_CLANG = true ] && ! [[ -f $CLANG_DIR/clang ]]; then
    cp "$CLANG_PATH" "$CLANG_DIR/clang"
    ln -s "$CLANG_DIR/clang" "$CLANG_DIR/clang++"
    cp @DANDELION_TARGET@-clang.cfg "$CLANG_DIR/clang.cfg" 
    cp @DANDELION_TARGET@-clang++.cfg "$CLANG_DIR/clang++.cfg" 
fi