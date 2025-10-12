#!/usr/bin/env bash
docker run  \
   --mount type=bind,source="$(pwd)"/dist,target=/usr/local/beemu/dist\
   "$(docker build -q .)" \
   cp build/libbeemu.so dist/libbeemu.so
