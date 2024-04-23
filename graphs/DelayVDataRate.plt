set terminal png
    set title "Mean Delay vs. Data Rate (TCP-NewReno)"

    set xlabel "Data Rate (kbps)"

    set ylabel "Mean Delay (ms)"

    set datafile separator whitespace 

    set output "./graphics/NewRenoMeanDelay.png"

    plot "NewRenoStats.txt" using 1:2 with linespoints title "Mean Delay"
