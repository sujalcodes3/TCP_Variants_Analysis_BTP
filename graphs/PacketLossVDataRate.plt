set terminal png
    set title "Packet Loss vs. Data Rate (TCP-NewReno)"

    set xlabel "Data Rate (in bps)"

    set ylabel "Packet Loss Fraction"

    set datafile separator whitespace 

    set output "./graphics/NewRenoPacketLoss.png"

    plot "NewRenoStats.txt" using 1:3 with linespoints title "Packet Loss"
