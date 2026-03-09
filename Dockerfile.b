# syntax=docker/dockerfile:1
FROM ubuntu:24.04

# arguments
ARG BUILD="Release"
ARG PLATFORM="debug"
ARG TARGET_ARCH="aarch64"
ARG VERSION="experimental"

# --- All build dependencies (SDK build + libc-test) ---
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential ca-certificates cmake git make file \
      wget gpg lsb-release software-properties-common \
      gcc g++ automake curl && \
    rm -rf /var/lib/apt/lists/*

# clang-20 via the LLVM APT script
RUN wget https://apt.llvm.org/llvm.sh -O ~/llvm.sh && \
    chmod +x ~/llvm.sh && \
    ~/llvm.sh 20 && \
    ln -sf /usr/bin/ld.lld-20 /usr/bin/ld.lld && \
    rm ~/llvm.sh


ENV CC="clang"
ENV CXX="clang++"

# autoconf 2.69 — exact version required by newlib's configure
RUN curl -sL https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz | tar xz && \
    cd autoconf-2.69 && ./configure --quiet && make -s && make install && \
    cd / && rm -rf autoconf-2.69