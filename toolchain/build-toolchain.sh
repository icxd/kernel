#!/bin/bash

set -ex

export PREFIX=$(pwd)
export TARGET=i386-elf
export PATH=$PREFIX/bin:$PATH

if [ ! -d binutils ]; then git clone git://sourceware.org/git/binutils-gdb.git binutils; fi
if [ ! -d gcc ]; then git clone git://gcc.gnu.org/git/gcc.git gcc; fi

mkdir -p binutils-build
cd binutils-build

../binutils/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

cd ..
mkdir -p gcc-build
cd gcc-build

which -- $TARGET-as || echo $TARGET-as is not in PATH

../gcc/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c,c++ --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
