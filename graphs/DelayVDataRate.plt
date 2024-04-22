set terminal png
    set title "Mean Delay vs. Data Rate"

    set xlabel "Data Rate (in bps)"

    set ylabel "Mean Delay (in ms)"

    set datafile separator whitespace 

    set output "./graphics/NewRenoMeanDelay.png"

    plot "NewRenoStats.txt" using 1:2 with linespoints title "Mean Delay"
