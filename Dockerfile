FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-base-dev-tools \
    libgl1-mesa-dev \
    libopengl-dev \
    xvfb \
    x11vnc \
    xauth \
    libqt6sql6-mysql \
    tshark \
    tcpdump \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN cmake -S . -B build && cmake --build build -j

CMD ["/bin/bash"]
