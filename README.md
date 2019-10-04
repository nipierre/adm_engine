# ADM Audio Renderer

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
Usage: ./adm_audio_renderer INPUT [OUTPUT] [GAIN]
   with INPUT   BW64/ADM audio file
        OUTPUT  Destination directory (optional)
                  - if specified, enable ADM rendering to BW64/ADM file
                  - otherwise, dump input BW64/ADM file information
        GAIN    Gain to apply to Dialogue (optional)
```
