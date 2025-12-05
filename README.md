# Audio2Midi VST3/AU/CLAP Plugin

## How to build the project

```shell
cmake -B Builds -DCMAKE_BUILD_TYPE=Release .
cmake --build Builds --config Release --target Pamplejuce_All
```

## Run the standalone application

```shell
./Builds/Pamplejuce_artefacts/Release/Standalone/Pamplejuce\ Demo
```