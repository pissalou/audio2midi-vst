# Audio2Midi VST3/AU/CLAP Plugin

## How to build the project

```shell
cmake -B Builds -DCMAKE_BUILD_TYPE=Release .
cmake --build Builds --config Release --target audio2midi_All
```

## Unittest

```shell
ctest --test-dir Builds --verbose --output-on-failure
```

## Run the standalone application

```shell
./Builds/audio2midi_artefacts/Release/Standalone/audio2midi
```