FROM alpine:latest
LABEL authors="elsa"

WORKDIR /usr/local/beemu
RUN apk --no-cache add cmake make gcc g++ libc-dev linux-headers

COPY include/ ./include
COPY CMakeLists.txt ./
COPY src/ ./src
COPY tests ./tests

RUN mkdir build

WORKDIR /usr/local/beemu/build
RUN cmake ..
RUN cmake --build ./

WORKDIR /usr/local/beemu


