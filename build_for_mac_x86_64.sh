mkdir -p macos-x86_64-build && cd macos-x86_64-build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../macos_x86_64.cmake -G Ninja
ninja
