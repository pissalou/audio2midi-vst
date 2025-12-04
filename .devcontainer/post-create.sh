#!/bin/bash

set -euxo pipefail

# Configure the build system (cmake) right from the start.
cmake . -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release