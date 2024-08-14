function make_for() {
    echo "=================="
    echo "Making $1"
    echo "=================="
    rm build-$1 -r
    mkdir -p build-$1
    cd build-$1
    cmd="cmake .. -DCMAKE_BUILD_TYPE=Release -DPLATFORM=$2 -DRETRO68_ROOT=$RETRO68_TOOLCHAIN_PATH -DCMAKE_TOOLCHAIN_FILE=$RETRO68_INSTALL_PATH/cmake/$2.toolchain.cmake.in"
    cmd2="make"
    $cmd && $cmd2
    status=$?
    cd .. 
    return $status
}

make_for "68k" "retro68" && make_for "PowerPC" "retroppc"