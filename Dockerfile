# Use Ubuntu as base image
FROM ubuntu:22.04

# Set environment variables for non-interactive installs
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        doxygen \
        libglfw3 \
        libglfw3-dev \
        libglew-dev \
        libglm-dev \
        libx11-dev \
        libxi-dev \
        libxrandr-dev \
        libxinerama-dev \
        libxcursor-dev \
        libxxf86vm-dev \
        libxext-dev \
        libxfixes-dev \
        libxrender-dev \
        libxkbcommon-dev \
        libwayland-dev \
        libdbus-1-dev \
        pkg-config \
        git \
        x11-apps \
        && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

# Copy project files
COPY . /app


# Clean up any previous build directory to avoid CMakeCache.txt conflicts
RUN rm -rf build && mkdir -p build && cd build && cmake .. && make

# Expose X11 socket for GUI
ENV DISPLAY=:0

# Entry point (can be customized)
CMD ["/bin/bash"]
