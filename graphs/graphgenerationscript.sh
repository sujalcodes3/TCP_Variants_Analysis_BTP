#!/bin/bash

# Define array of placeholder values
placeholders=("Vegas" "Cubic" "Bic" "NewReno")

for variant in "${placeholders[@]}"; do 
    # Delete existing files if they exist
    removeCmd="rm DelayVDataRate.plt PacketLossVDataRate.plt"
    eval $removeCmd 
    # Write code to DelayVDataRate.txt
    echo "set terminal png
    set title \"Mean Delay vs. Data Rate\"

    set xlabel \"Data Rate (in bps)\"

    set ylabel \"Mean Delay (in ms)\"

    set datafile separator whitespace 

    set output \"./graphics/${variant}MeanDelay.png\"

    plot \"${variant}Stats.txt\" using 1:2 with linespoints title \"Mean Delay\"" >> "DelayVDataRate.plt"

    # Write code to PacketLossDataRate.txt
    echo "set terminal png
    set title \"Packet Loss vs. Data Rate\"

    set xlabel \"Data Rate (in bps)\"

    set ylabel \"Packet Loss Fraction\"

    set datafile separator whitespace 

    set output \"./graphics/${variant}PacketLoss.png\"

    plot \"${variant}Stats.txt\" using 1:3 with linespoints title \"Packet Loss\"" >> "PacketLossVDataRate.plt"

    echo "Files created successfully."

    command="gnuplot DelayVDataRate.plt && gnuplot PacketLossVDataRate.plt"

    eval $command
done 
