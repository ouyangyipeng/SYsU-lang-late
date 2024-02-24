#!/bin/sh
LLVMDIR=$1
LLVMDIR=$(realpath $LLVMDIR)

# Download the source code, try mirrors first
wget -c https://mirror.ghproxy.com/github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/llvm-17.0.6.src.tar.xz -P $LLVMDIR || wget -c https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/llvm-17.0.6.src.tar.xz -P $LLVMDIR
wget -c https://mirror.ghproxy.com/github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang-17.0.6.src.tar.xz -P $LLVMDIR || wget -c https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang-17.0.6.src.tar.xz -P $LLVMDIR
wget -c https://mirror.ghproxy.com/github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/cmake-17.0.6.src.tar.xz -P $LLVMDIR || wget -c https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/cmake-17.0.6.src.tar.xz -P $LLVMDIR
rm -rf $LLVMDIR/llvm $LLVMDIR/clang $LLVMDIR/cmake $LLVMDIR/build $LLVMDIR/install

tar -xJvf $LLVMDIR/llvm-17.0.6.src.tar.xz -C $LLVMDIR
tar -xJvf $LLVMDIR/clang-17.0.6.src.tar.xz -C $LLVMDIR
tar -xJvf $LLVMDIR/cmake-17.0.6.src.tar.xz -C $LLVMDIR

mv $LLVMDIR/llvm-17.0.6.src $LLVMDIR/llvm
mv $LLVMDIR/clang-17.0.6.src $LLVMDIR/clang
mv $LLVMDIR/cmake-17.0.6.src $LLVMDIR/cmake

mkdir $LLVMDIR/build $LLVMDIR/install
cmake $LLVMDIR/llvm -B $LLVMDIR/build -G Ninja\
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$LLVMDIR/install \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DLLVM_USE_LINKER=lld \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_TESTS=OFF
cmake --build $LLVMDIR/build --target install -j $(nproc)
rm -rf $LLVMDIR/llvm $LLVMDIR/clang $LLVMDIR/cmake $LLVMDIR/build
