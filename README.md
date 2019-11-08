# ADM Engine

[![Build Status](https://travis-ci.org/media-cloud-ai/adm_engine.svg?branch=master)](https://travis-ci.org/media-cloud-ai/adm_engine)

### Required dependencies

The project depends on these libraries:
- [libadm](https://github.com/IRT-Open-Source/libadm/)
- [libear](https://github.com/ebu/libear)
- [libbw64](https://github.com/IRT-Open-Source/libbw64)

### Build & Install
```
mkdir build && cd build
cmake ..
make
make install
```

### Usage
```
Usage: ./adm-engine INPUT [OPTIONS]

  INPUT                   BW64/ADM audio file path
  OPTIONS:
    -o OUTPUT            Destination directory
    -e ELEMENT_ID        ADM element defined by its ELEMENT_ID to render (AudioProgramme or AudioObject only)
    -g ELEMENT_ID=GAIN   GAIN value (in dB) to apply to ADM element defined by its ELEMENT_ID

  If no OUTPUT argument is specified, this program dumps the input BW64/ADM file information.
  Otherwise, it enables ADM rendering to BW64/ADM file into destination directory.

  Examples:
    - Dumping BW64/ADM file info:
          ./adm-engine /path/to/input/file.wav
    - Rendering ADM:
          ./adm-engine /path/to/input/file.wav -o /path/to/output/directory
    - Rendering specified ADM element:
          ./adm-engine /path/to/input/file.wav  -e APR_1002 -o /path/to/output/directory
    - Rendering ADM, applying gains to elements:
          ./adm-engine /path/to/input/file.wav -o /path/to/output/directory -g AO_1001=-4.0 -g ACO_1002=5.0

```
