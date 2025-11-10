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
        echo "Unknown argument to script"
        ;;
    esac
done

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

# Detect BSD vs GNU sed
if sed --version >/dev/null 2>&1; then
    # GNU sed
    SED_INPLACE() { sed -i "$@"; }
else
    # BSD sed (macOS)
    SED_INPLACE() { sed -i '' "$@"; }
fi

COMPILER_INCLUDES=$($CLANG_NAME -print-file-name=include)
SED_INPLACE "s|COMPILER_INCLUDES|$COMPILER_INCLUDES|g" @DANDELION_TARGET@-clang.cfg

CLANG_PATH="$(which $CLANG_NAME)"
CLANG_DIR="$(dirname $CLANG_PATH)"

if ! [ -f @DANDELION_TARGET@-clang ]; then
    cp $CLANG_PATH @DANDELION_TARGET@-clang
    ln -s @DANDELION_TARGET@-clang @DANDELION_TARGET@-clang++
fi

if [ $DEFAULT_CLANG = true ] && ! [[ -f $CLANG_DIR/clang ]]; then
    cp "$CLANG_PATH" "$CLANG_DIR/clang"
    ln -s "$CLANG_DIR/clang" "$CLANG_DIR/clang++"
    sed "s|<CFGDIR>|$SCRIPT_DIR|g" @DANDELION_TARGET@-clang.cfg >> "$CLANG_DIR/clang.cfg"
    # enable stdinc, as we assume it has been cleared when we install our clang as system clang
    SED_INPLACE "s|-nostdinc||g" "$CLANG_DIR/clang.cfg"
    sed "s|<CFGDIR>|$SCRIPT_DIR|g" @DANDELION_TARGET@-clang++.cfg >> "$CLANG_DIR/clang++.cfg"
    # need to change the config to use the clang.cfg in the same folder, so remove prefix
    SED_INPLACE "s|@DANDELION_TARGET@-||g" "$CLANG_DIR/clang++.cfg"
fi