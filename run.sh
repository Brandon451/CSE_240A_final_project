#!/bin/sh

make all

benchmarks=(./traces/*)

total_static_mpi=0
total_gshare_mpi=0
total_tournament_mpi=0
total_perceptron_mpi=0

for benchmark in "${benchmarks[@]}"
do
    value=$(echo $benchmark | cut -d'/' -f 3)
    echo "Evaluating $value benchmark..."

    output=`bunzip2 -kc $benchmark | ./src/predictor --static`
    echo "static    \c"; echo $output
    mpi=`echo $output | awk '{print $7;}'`
    total_static_mpi=`echo "$total_static_mpi + $mpi" | bc`

    output=`bunzip2 -kc $benchmark | ./src/predictor --gshare:13`
    echo "gshare    \c"; echo $output
    mpi=`echo $output | awk '{print $7;}'`
    total_gshare_mpi=`echo "$total_gshare_mpi + $mpi" | bc`

    output=`bunzip2 -kc $benchmark | ./src/predictor --tournament:8:9:9`
    echo "tournament    \c"; echo $output
    mpi=`echo $output | awk '{print $7;}'`
    total_tournament_mpi=`echo "$total_tournament_mpi + $mpi" | bc`

    output=`bunzip2 -kc $benchmark | ./src/predictor --perceptron:256:31`
    echo "perceptron    \c"; echo $output
    mpi=`echo $output | awk '{print $7;}'`
    total_perceptron_mpi=`echo "$total_perceptron_mpi + $mpi" | bc`

done

echo "Average misprediction for static predictor is: \c"
echo "scale=4; $total_static_mpi/6" | bc

echo "Average misprediction for gshare predictor is: \c"
echo "scale=4; $total_gshare_mpi/6" | bc

echo "Average misprediction for tournament predictor is: \c"
echo "scale=4; $total_tournament_mpi/6" | bc

echo "Average misprediction for perceptron predictor is: \c"
echo "scale=4; $total_perceptron_mpi/6" | bc
