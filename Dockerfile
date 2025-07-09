# syntax=docker/dockerfile:1
FROM ubuntu:24.04

# arguments
ARG BUILD="Debug"
ARG PLATFORM="debug"
ARG TARGET_ARCH="x86_64"
ARG VERSION="experimental"

# install packages required packages to get things running
RUN apt update
RUN apt-get install -y wget gpg
RUN apt-get install -y lsb-release
RUN apt-get install -y software-properties-common
RUN apt-get install -y gcc g++ # clang uses gcc to interface with the linker

# download initial clang
RUN wget https://apt.llvm.org/llvm.sh -O ~/llvm.sh
RUN chmod +x ~/llvm.sh
RUN ~/llvm.sh
RUN ln -s /usr/bin/ld.lld-19 /usr/bin/ld.lld
RUN rm -r /usr/include/*

# download and set up dandelionSDK
RUN wget https://github.com/eth-easl/dandelionSDK/releases/download/${VERSION}/dandelion_sdk_${BUILD}_${PLATFORM}_${TARGET_ARCH}.tar.gz -O ~/dandelionSDK.tar.gz
RUN tar -xzf ~/dandelionSDK.tar.gz -C ~/
RUN ~/dandelion_sdk/create-compiler.sh -d -c clang-19

ENV CC="clang"
ENV CXX="clang++"

# additional useful tooling
RUN apt-get install -y cmake
RUN apt-get install -y git