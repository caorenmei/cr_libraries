#!/bin/bash

pushd cmake
cmake -D_CR_MSVC=0 -P third_party.cmake
popd