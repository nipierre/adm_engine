FROM debian:buster as builder

RUN apt-get update && \
    apt-get install -y \
      git \
      g++ \
      make \
      cmake \
      libboost-dev \
      libyaml-cpp-dev

RUN git clone --recursive https://github.com/ebu/libear.git && \
    cd libear/ && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

RUN git clone https://github.com/IRT-Open-Source/libadm.git && \
    cd libadm && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

RUN git clone https://github.com/IRT-Open-Source/libbw64.git && \
    cd libbw64 && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

ADD . ./adm_audio_render

RUN cd adm_audio_render && \
    rm -Rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

FROM mediacloudai/rs_command_line_worker:latest

COPY --from=builder /usr/local/bin/adm_audio_renderer /app/adm_render/bin/adm_audio_renderer
COPY --from=builder /usr/local/lib/ /app/adm_render/lib/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libyaml-cpp.so* /app/adm_render/lib/

WORKDIR /app/adm_render

ENV AMQP_QUEUE job_adm_render
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/app/adm_render/lib
ENV PATH $PATH:/app/adm_render/bin

CMD command_line_worker
