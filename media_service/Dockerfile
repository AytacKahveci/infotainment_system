FROM toolchain:latest AS builder

WORKDIR /app/media_service

COPY media_service/ ./

RUN . build/conanbuild.sh && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake && \
    cmake --build build --target media_service -j$(nproc)

RUN chmod +x build/conanrun.sh

EXPOSE 50001

CMD ["/bin/sh", "-c", ". build/conanrun.sh && build/media_service"]