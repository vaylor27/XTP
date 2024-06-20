mkdir macos-arm64-build && cd macos-arm64-build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../macos_arm.cmake -G Ninja
ninja
