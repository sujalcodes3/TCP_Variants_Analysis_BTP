# Set plot style and labels
set terminal png
set output "./graphics/CombinedThroughput.png"
set xlabel "Channel DataRate(in bps)"
set title "Throughput vs. Channel DataRate"

# Define line styles and colors for each key
set style line 1 lt 1 lc rgb "blue" lw 2
set style line 2 lt 1 lc rgb "green" lw 2
set style line 3 lt 1 lc rgb "red" lw 2
set style line 4 lt 1 lc rgb "purple" lw 2


# Define key labels
set key top left

# Plot data for each key
plot "CombinedStats.txt" using ($1 == 0 ? $2 : 1/0):5 with linespoints  title "Vegas" ls 1, \
     "" using ($1 == 1 ? $2 : 1/0):5 with linespoints title "CUBIC" ls 2, \
     "" using ($1 == 2 ? $2 : 1/0):5 with linespoints title "BIC" ls 3, \
     "" using ($1 == 3 ? $2 : 1/0):5 with linespoints title "NewReno" ls 4

