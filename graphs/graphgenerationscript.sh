#!/bin/bash

# Define array of placeholder values
placeholders=("Vegas" "Cubic" "Bic" "NewReno")

for variant in "${placeholders[@]}"; do 
    # Delete existing files if they exist
    removeCmd="rm DelayVDataRate.plt PacketLossVDataRate.plt"
    eval $removeCmd 
    # Write code to DelayVDataRate.txt
    echo "set terminal png
    set title \"Mean Delay vs. Data Rate (TCP-${variant})\"

    set xlabel \"Data Rate (kbps)\"

    set ylabel \"Mean Delay (ms)\"

    set datafile separator whitespace 

    set output \"./graphics/${variant}MeanDelay.png\"

    plot \"${variant}Stats.txt\" using 1:2 with linespoints title \"Mean Delay\"" >> "DelayVDataRate.plt"

    # Write code to PacketLossDataRate.txt
    echo "set terminal png
    set title \"Packet Loss vs. Data Rate (TCP-${variant})\"

    set xlabel \"Data Rate (kbps)\"

    set ylabel \"Packet Loss Fraction\"

    set datafile separator whitespace 

    set output \"./graphics/${variant}PacketLoss.png\"

    plot \"${variant}Stats.txt\" using 1:3 with linespoints title \"Packet Loss\"" >> "PacketLossVDataRate.plt"

    echo "Files created successfully."

    command="gnuplot DelayVDataRate.plt && gnuplot PacketLossVDataRate.plt"

    eval $command
done 

# Combined Stats
removeCmd="rm CombinedPlots.plt"
eval $removeCmd

echo "set terminal png
set output \"./graphics/CombinedThroughput.png\"
set xlabel \"Channel DataRate(kbps)\"
set ylabel \"Throughput(kbps)\"
set title \"Throughput vs. Channel DataRate (Comparision)\"

set style line 1 lt 1 lc rgb \"blue\" lw 2
set style line 2 lt 1 lc rgb \"green\" lw 2
set style line 3 lt 1 lc rgb \"red\" lw 2
set style line 4 lt 1 lc rgb \"purple\" lw 2

set key top right 

plot \"CombinedStats.txt\" using (\$1 == 0 ? \$2 : 1/0):5 with linespoints  title \"Vegas\" ls 1, \\
    \"\" using (\$1 == 1 ? \$2 : 1/0):5 with linespoints title \"CUBIC\" ls 2, \\
    \"\" using (\$1 == 2 ? \$2 : 1/0):5 with linespoints title \"BIC\" ls 3, \\
    \"\" using (\$1 == 3 ? \$2 : 1/0):5 with linespoints title \"NewReno\" ls 4" >> "CombinedPlots.plt"

command="gnuplot CombinedPlots.plt"
eval $command

removeCmd="rm CombinedPlots.plt"
eval $removeCmd

echo "set terminal png
set output \"./graphics/CombinedDelay.png\"
set xlabel \"Channel DataRate(kbps)\"
set ylabel \"Mean Delay(ms)\"
set title \"Mean Delay vs. Channel DataRate (Comparision)\"

set style line 1 lt 1 lc rgb \"blue\" lw 2
set style line 2 lt 1 lc rgb \"green\" lw 2
set style line 3 lt 1 lc rgb \"red\" lw 2
set style line 4 lt 1 lc rgb \"purple\" lw 2

set key top right


plot \"CombinedStats.txt\" using (\$1 == 0 ? \$2 : 1/0):3 with linespoints  title \"Vegas\" ls 1, \\
    \"\" using (\$1 == 1 ? \$2 : 1/0):3 with linespoints title \"CUBIC\" ls 2, \\
    \"\" using (\$1 == 2 ? \$2 : 1/0):3 with linespoints title \"BIC\" ls 3, \\
    \"\" using (\$1 == 3 ? \$2 : 1/0):3 with linespoints title \"NewReno\" ls 4" >> "CombinedPlots.plt"

command="gnuplot CombinedPlots.plt"
eval $command

removeCmd="rm CombinedPlots.plt"
eval $removeCmd

echo "set terminal png
set output \"./graphics/CombinedPacketLoss.png\"
set xlabel \"Channel DataRate(kbps)\"
set ylabel \"Packet Loss Fraction\"
set title \"Packet Loss vs. Channel DataRate (Comparision)\"

set style line 1 lt 1 lc rgb \"blue\" lw 2
set style line 2 lt 1 lc rgb \"green\" lw 2
set style line 3 lt 1 lc rgb \"red\" lw 2
set style line 4 lt 1 lc rgb \"purple\" lw 2

set key top right

plot \"CombinedStats.txt\" using (\$1 == 0 ? \$2 : 1/0):4 with linespoints  title \"Vegas\" ls 1, \\
    \"\" using (\$1 == 1 ? \$2 : 1/0):4 with linespoints title \"CUBIC\" ls 2, \\
    \"\" using (\$1 == 2 ? \$2 : 1/0):4 with linespoints title \"BIC\" ls 3, \\
    \"\" using (\$1 == 3 ? \$2 : 1/0):4 with linespoints title \"NewReno\" ls 4" >> "CombinedPlots.plt"

command="gnuplot CombinedPlots.plt"
eval $command
