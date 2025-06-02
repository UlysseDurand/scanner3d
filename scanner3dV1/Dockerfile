# Use an official C++ base image with CMake and build tools
FROM ubuntu:22.04

# Set environment variables to noninteractive
ENV DEBIAN_FRONTEND=noninteractive

# Install required dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    unzip \
    libopencv-dev \
    libeigen3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the entire project into the container
COPY CMakeLists.txt CMakeLists.txt
COPY Makefile Makefile
COPY src/ src/
COPY external/ external/
COPY data/ data/

# Create a build directory
RUN mkdir -p build

# Build the project
RUN make build

# Set the default command to run the built executable
CMD ["make", "run"]
