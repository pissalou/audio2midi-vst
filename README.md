# Audio2Midi VST3/AU/CLAP Plugin

## How to build the project

```shell
cmake . -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release
targets=$(ninja -C Builds -t targets all | grep -E -v '(/|\.a|cmake|CXX|cache|CMakeCache|Catch|clap|ninja)' | cut -d: -f1)
for target in $targets; do
  alias build_$target="cmake --build $(pwd)/Builds --config Release --target $target"
done
build_all
```

## Unittest

```shell
build_test
alias run_test="ctest --test-dir Builds --verbose --output-on-failure"
run_test
```

## Run the standalone application
``
```shell
alias run="$(pwd)/Builds/audio2midi_artefacts/Release/Standalone/audio2midi"
run
```