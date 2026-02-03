#!/bin/bash
set -euo pipefail

install_dir=$HOME/.local

readonly autoconf_version="2.69"

if [ ! -d "$install_dir/bin" ]; then
    mkdir "$install_dir/bin"
    echo "export PATH=\"$install_dir/bin:\$PATH\"" >> ~/.bashrc
    source ~/.bashrc
fi

# install m4 (required to build autoconf)
if [ ! type m4 &> /dev/null ]; then
    sudo apt install m4
fi

# install automake
if [ ! type automake &> /dev/null ]; then
    sudo apt install automake
fi

# install (specific version of) autoconf
wget --continue --quiet "https://ftp.gnu.org/gnu/autoconf/autoconf-${autoconf_version}.tar.gz" -O _autoconf.tar.gz
mkdir _autoconf
tar -xzf _autoconf.tar.gz -C _autoconf --strip-components=1
pushd _autoconf
./configure --prefix="$install_dir"
make -j$(nproc)
make install
popd
rm -rf _autoconf*
