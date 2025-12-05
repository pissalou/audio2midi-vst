FROM mcr.microsoft.com/devcontainers/cpp:2-ubuntu24.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt install \
    libasound2-dev \
    libx11-dev \
    libxinerama-dev \
    libxext-dev \
    libfreetype6-dev \
    libwebkit2gtk-4.1-dev \
    libglu1-mesa-dev \
    xvfb \
    cmake \
    ninja-build
