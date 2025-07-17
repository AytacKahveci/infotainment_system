FROM toolchain:latest AS builder

WORKDIR /app/qt_information_ui

COPY qt_information_ui/ ./
COPY proto /app/proto

RUN . /app/toolchain/build/conanbuild.sh && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/app/toolchain/build/conan_toolchain.cmake && \
    cmake --build build --target qt_information_ui -j$(nproc)

RUN chmod +x /app/toolchain/build/conanrun.sh

CMD ["/bin/sh", "-c", "DISPLAY=:0 . /app/toolchain/build/conanrun.sh && build/qt_information_ui"]
