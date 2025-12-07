#!/bin/bash

set -euxo pipefail

# Fetch submodules (JUCE, metatonin...)
git submodule update --init --recursive

# Configure the build system (cmake) right from the start.
cmake . -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release