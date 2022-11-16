#!/bin/bash

set -e
set -o pipefail

make
./main 2> flowdata

octave -q evaluate_flowdata.m

gnuplot plot.gnuplot
