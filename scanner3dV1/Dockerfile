FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    unzip \
    libopencv-dev \
    libeigen3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt CMakeLists.txt
COPY Makefile Makefile
COPY src/ src/
COPY external/ external/
COPY data/ data/

RUN mkdir -p build

RUN make build

# Set the default command to run the built executable
CMD ["make", "run"]
