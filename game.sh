#!/bin/bash

# Compile and run
cd build && cmake .. && make && cd - && ./build/bin/rapid_shooter game ./impel_down.json
