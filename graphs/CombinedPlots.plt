set terminal png
set output "./graphics/CombinedPacketLoss.png"
set xlabel "Channel DataRate(in bps)"
set ylabel "Packet Loss Fraction"
set title "Packet Loss vs. Channel DataRate (Comparision)"

set style line 1 lt 1 lc rgb "blue" lw 2
set style line 2 lt 1 lc rgb "green" lw 2
set style line 3 lt 1 lc rgb "red" lw 2
set style line 4 lt 1 lc rgb "purple" lw 2

set key top left

plot "CombinedStats.txt" using ($1 == 0 ? $2 : 1/0):4 with linespoints  title "Vegas" ls 1, \
    "" using ($1 == 1 ? $2 : 1/0):4 with linespoints title "CUBIC" ls 2, \
    "" using ($1 == 2 ? $2 : 1/0):4 with linespoints title "BIC" ls 3, \
    "" using ($1 == 3 ? $2 : 1/0):4 with linespoints title "NewReno" ls 4
