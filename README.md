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
Usage: ./adm-engine INPUT [OUTPUT] [ELEMENT_ID=GAIN]
   with INPUT              BW64/ADM audio file
        OUTPUT             Destination directory (optional)
                             - if specified, enable ADM rendering to BW64/ADM file
                             - otherwise, dump input BW64/ADM file information
        ELEMENT_ID=GAIN    GAIN value (in dB) to apply to ADM element defined by its ELEMENT_ID (optional)
```
