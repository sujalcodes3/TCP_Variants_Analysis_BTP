#!/bin/bash

tcpVariants=("vegas" "bic" "newreno" "cubic")
dataRates=("512Kbps" "1Mbps" "2Mbps" "3Mbps" "4Mbps")

resultsFile="./scratch/stats.csv"

echo "TCP Variant,Data Rate,Tx Packets,Rx Packets,Mean Delay,Lost Packets,Packet Loss Ratio,TCP Throughput" > "$resultsFile"

for variant in "${tcpVariants[@]}"; do
    for rate in "${dataRates[@]}"; do
        command="./ns3 run \"scratch/test-$variant.cc --rate=$rate\""

        echo $command

        output=$(eval $command)
        txPackets=$(echo "$output" | grep -oP 'Tx Packets = \K\d+')
        rxPackets=$(echo "$output" | grep -oP 'Rx Packets = \K\d+')
        meanDelay=$(echo "$output" | grep -oP 'Mean Delay: \K[\d.]+')
        lostPackets=$(echo "$output" | grep -oP 'Lost Packets = \K\d+')
        packetLossRatio=$(echo "$output" | grep -oP 'Packet Loss Ratio = \K[\d.]+')
        tcpThroughput=$(echo "$output" | grep -oP 'TCP Throughput: \K[\d.]+')

        # Append data to CSV file
        echo "$variant,$rate, $txPackets,$rxPackets,$meanDelay,$lostPackets,$packetLossRatio,$tcpThroughput" >> $resultsFile
    done 
done


