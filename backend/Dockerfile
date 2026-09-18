# syntax=docker/dockerfile:1

##############################
# Stage 1: build the binary  #
##############################
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Same package list as backend/README.md's "Prerequisites" section.
# libdrogon-dev pulls in trantor + the drogon runtime lib as a dependency.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        ca-certificates \
        pkg-config \
        libdrogon-dev \
        libssl-dev \
        libpq-dev \
        libjsoncpp-dev \
        libsqlite3-dev \
        libmariadb-dev \
        libmariadb-dev-compat \
        libhiredis-dev \
        libyaml-cpp-dev \
        zlib1g-dev \
        uuid-dev \
        libbrotli-dev \
        libc-ares-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy only what CMake needs to configure first, so dependency
# fetching/configuring is cached even when application source changes.
COPY CMakeLists.txt vcpkg.json ./
COPY main.cc ./main.cc
COPY src ./src

RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && cmake --build . -j"$(nproc)"

##############################
# Stage 2: runtime image     #
##############################
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# Installing the same -dev packages as the builder guarantees the exact
# matching runtime .so versions are present (apt resolves the concrete
# runtime package for you), avoiding hand-guessing versioned runtime
# package names (libjsoncpp25 vs libjsoncpp1, etc). The extra header/
# static-lib bytes this pulls in are negligible next to the libraries
# themselves.
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        libdrogon-dev \
        libssl-dev \
        libpq-dev \
        libjsoncpp-dev \
        libsqlite3-dev \
        libmariadb-dev \
        libhiredis-dev \
        libyaml-cpp-dev \
        zlib1g \
        uuid-runtime \
        libbrotli-dev \
        libc-ares-dev \
    && rm -rf /var/lib/apt/lists/*

RUN useradd --create-home --shell /usr/sbin/nologin appuser

WORKDIR /app

COPY --from=builder /src/build/SmartBusTracker ./SmartBusTracker
COPY config.json ./config.json
COPY database ./database

RUN mkdir -p /app/uploads/tmp && chown -R appuser:appuser /app

USER appuser

# APP_PORT is expanded into config.json at startup by the app itself.
ENV APP_PORT=8080
EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=3s --start-period=10s --retries=3 \
    CMD curl -fsS "http://127.0.0.1:${APP_PORT}/api/health" || exit 1

ENTRYPOINT ["./SmartBusTracker"]
