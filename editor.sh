#!/bin/bash

# Compile and run
cd build && cmake .. && make && cd - && ./build/bin/rapid_editor ./impel_down.json
