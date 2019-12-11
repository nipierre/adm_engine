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
    git checkout 0.11.0 && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

RUN git clone https://github.com/IRT-Open-Source/libbw64.git && \
    cd libbw64 && \
    git checkout 0.10.0 && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

ADD . ./adm_engine

RUN cd adm_engine && \
    rm -Rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

FROM mediacloudai/c_amqp_worker:latest

COPY --from=builder /usr/local/bin/adm-engine /app/adm_engine/bin/adm-engine
COPY --from=builder /usr/local/lib/ /app/adm_engine/lib/
COPY --from=builder /usr/local/worker/ /app/adm_engine/worker/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libyaml-cpp.so* /app/adm_engine/lib/

WORKDIR /app/adm_engine

ENV AMQP_QUEUE job_adm_engine
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/app/adm_engine/lib
ENV WORKER_LIBRARY_FILE /app/adm_engine/worker/libadmengineworker.so
ENV PATH $PATH:/app/adm_engine/bin

CMD c_amqp_worker
