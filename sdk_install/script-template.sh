#!/bin/bash

set -e

if ! [ -f @DANDELION_TARGET@-clang ]; then
    CLANG_PATH=$(which clang)
    cp $CLANG_PATH @DANDELION_TARGET@-clang
    ln -s @DANDELION_TARGET@-clang @DANDELION_TARGET@-clang++
fi