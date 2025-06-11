#!/bin/bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

COMPILER_INCLUDES=$(clang -print-file-name=include)
sed -i "s|COMPILER_INCLUDES|$COMPILER_INCLUDES|g" @DANDELION_TARGET@-clang.cfg

if ! [ -f @DANDELION_TARGET@-clang ]; then
    CLANG_PATH=$(which clang)
    cp $CLANG_PATH @DANDELION_TARGET@-clang
    ln -s @DANDELION_TARGET@-clang @DANDELION_TARGET@-clang++
fi