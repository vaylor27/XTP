mkdir -p macos-arm64-build && cd macos-arm64-build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../macos_arm.cmake -DBUILD_SHARED_LIBS:BOOL=ON -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
