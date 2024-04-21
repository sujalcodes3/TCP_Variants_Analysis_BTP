//accompanied with throughput and delay.


#include <fstream> // For file I/O
#include <iostream> // For input and output to the console
#include <string> // For using the std::string class

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-phy.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;

typedef NetDeviceContainer NDC;
typedef Ipv4InterfaceContainer Ipv4IC;

// Function to record when a packet is transmitted by the application
void TxCallback (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet) {
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << packet->GetSize() << std::endl;
}

// Function to record when a packet is received by the sink application
void RxCallback (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &address) {
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << packet->GetSize() << std::endl;
}

int main(int argc, char* argv[]) {

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));

    std::string animFile = "throughput_delay_forward.xml"; 
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Define nodes;
    NodeContainer EndNodes;
    NodeContainer Routers;
    NodeContainer TrafficNodes;

    // Create Nodes
    EndNodes.Create(2);
    Routers.Create(2);
    TrafficNodes.Create(4);

    // helper for p2p links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("3Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));


    // ROUTER and TRAFFIC NODE LINKS
    NDC linkRT00 = p2p.Install(Routers.Get(0), TrafficNodes.Get(0));
    NDC linkRT02 = p2p.Install(Routers.Get(0), TrafficNodes.Get(2));
    NDC linkRT11 = p2p.Install(Routers.Get(1), TrafficNodes.Get(1));
    NDC linkRT13 = p2p.Install(Routers.Get(1), TrafficNodes.Get(3));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;
    //wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");

    NDC ConfiguredRouters;
    mac.SetType("ns3::AdhocWifiMac", "Ssid", SsidValue(ssid));
    ConfiguredRouters = wifi.Install(phy, mac, Routers);
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(Routers);
    mobility.Install(TrafficNodes);
    mobility.Install(EndNodes);
    
    // ENDNODES ROUTER LINK
    NDC linkRE0 = p2p.Install(Routers.Get(0), EndNodes.Get(0));
    NDC linkRE1 = p2p.Install(Routers.Get(1), EndNodes.Get(1));

    InternetStackHelper stack;
    stack.Install(Routers);
    stack.Install(TrafficNodes);
    stack.Install(EndNodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4IC IfaceRT00 = address.Assign(linkRT00);
    address.NewNetwork();

    Ipv4IC IfaceRT02 = address.Assign(linkRT02);
    address.NewNetwork();

    Ipv4IC IfaceRE0 = address.Assign(linkRE0);
    address.NewNetwork();

    address.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4IC IfaceRT11 = address.Assign(linkRT11);
    address.NewNetwork();

    Ipv4IC IfaceRT13 = address.Assign(linkRT13);
    address.NewNetwork();

    Ipv4IC IfaceRE1 = address.Assign(linkRE1);
    address.NewNetwork();

    // Router Interface
    address.SetBase("10.3.1.0", "255.255.255.0");

    Ipv4IC interR0 = address.Assign(ConfiguredRouters);
    address.NewNetwork();

    // TCP Sender
    OnOffHelper tcpSourceHelper("ns3::TcpSocketFactory", InetSocketAddress(IfaceRE1.GetAddress(1), 8080));
    tcpSourceHelper.SetAttribute("DataRate", DataRateValue(DataRate("3Mbps")));
    tcpSourceHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    tcpSourceHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer tcpSources;
    tcpSources.Add(tcpSourceHelper.Install(EndNodes.Get(0)));
    tcpSources.Start(Seconds(1.0));  
    tcpSources.Stop(Seconds(10.0));
     
    // TCP Reciever
    Address tcpSinkAddr(InetSocketAddress(IfaceRE1.GetAddress(1), 8080));
    PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory",tcpSinkAddr);

    ApplicationContainer tcpSinks;
    tcpSinks.Add(tcpSinkHelper.Install(EndNodes.Get(1)));
    tcpSinks.Start(Seconds(1.0));
    tcpSinks.Stop(Seconds(10.0));

    InetSocketAddress udpSinkAddr(IfaceRT13.GetAddress(1), 4000);

    OnOffHelper udpSourceHelper("ns3::UdpSocketFactory", Address());
    udpSourceHelper.SetAttribute("Remote", AddressValue(udpSinkAddr));
    udpSourceHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSourceHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpSources;
    udpSources.Add(udpSourceHelper.Install(TrafficNodes.Get(0)));
    udpSources.Start(Seconds(1.0));
    udpSources.Stop(Seconds(10.0));

    PacketSinkHelper udpSinkHelper("ns3::UdpSocketFactory", udpSinkAddr);

    ApplicationContainer udpSinks;
    udpSinks.Add(udpSinkHelper.Install(TrafficNodes.Get(3)));
    
    udpSinks.Start(Seconds(1.0));
    udpSinks.Stop(Seconds(10.0));

    AnimationInterface anim(animFile);
    anim.EnablePacketMetadata();                                // Optional
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional

    Ptr<ConstantPositionMobilityModel> r0 = Routers.Get(0) -> GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> r1 = Routers.Get(1) -> GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> e0 = EndNodes.Get(0) -> GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> e1 = EndNodes.Get(1) -> GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> t0 = TrafficNodes.Get(0) -> GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> t1 = TrafficNodes.Get(1) -> GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> t2 = TrafficNodes.Get(2) -> GetObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> t3 = TrafficNodes.Get(3) -> GetObject<ConstantPositionMobilityModel>();


    r0 ->SetPosition(Vector(25.0, 44.0, 0.0));
    r1 ->SetPosition(Vector(44.0, 44.0, 0.0));

    e0 ->SetPosition(Vector(0.0, 44.0, 0.0));
    e1 ->SetPosition(Vector(75.0, 44.0, 0.0));

    t0 ->SetPosition(Vector(10.0, 24.0, 0.0));
    t1 ->SetPosition(Vector(65.0, 24.0, 0.0));
    t2 ->SetPosition(Vector(10.0, 64.0, 0.0));
    t3 ->SetPosition(Vector(65.0, 64.0, 0.0));

    // Set up the actual simulation
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create a file stream to record sent and received packets
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> streamSent = ascii.CreateFileStream("tcp-sent-forward.txt");
    Ptr<OutputStreamWrapper> streamReceived = ascii.CreateFileStream("tcp-received-forward.txt");

    // Attach the trace sources
    tcpSources.Get(0)->TraceConnectWithoutContext("Tx", MakeBoundCallback(&TxCallback, streamSent));
    Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeBoundCallback(&RxCallback, streamReceived));

    double firstSentTime = -1.0;

    Simulator::Run();

    // Calculate TCP Throughput
    Ptr<PacketSink> tcpSink = DynamicCast<PacketSink>(tcpSinks.Get(0));
    uint64_t tcpTotalBytes = tcpSink->GetTotalRx();
    double simulationDuration = 9.0; // Duration over which the throughput is calculated (10s - 1s)
    double tcpThroughput = (tcpTotalBytes * 8.0) / (simulationDuration * 1024.0); // Convert to Kbps

    // Calculate Average Delay
    std::ifstream sentFile("tcp-sent-forward.txt");
    std::ifstream receivedFile("tcp-received-forward.txt");
    std::string lineSent, lineReceived;
    double totalDelay = 0.0;
    int packetCount = 0;

    // Read the first sent time
    if (getline(sentFile, lineSent)) {
        std::istringstream sentStream(lineSent);
        sentStream >> firstSentTime;
    }

    // Accumulate delays and count packets
    while (getline(receivedFile, lineReceived)) {
        std::istringstream receivedStream(lineReceived);
        double receivedTime, packetSize;
        receivedStream >> receivedTime >> packetSize;
        totalDelay += receivedTime - firstSentTime;
        packetCount++;
    }  

    double averageDelay = totalDelay / packetCount;

    // Open a file for output and write all the calculated values
    std::ofstream outFile;
    outFile.open("tcp-performance-forward.txt");
    if (outFile.is_open()) {
        outFile << "TCP Throughput: " << tcpThroughput << " Kbps\n";
        outFile << "Average Delay: " << averageDelay << " seconds\n";
        outFile.close(); // Close the file after writing
        std::cout << "TCP performance metrics have been written to 'tcp-performance-forward.txt'" << std::endl;
    } else {
        std::cerr << "Unable to open file 'tcp-performance-forward.txt' for writing." << std::endl;
    }

    // Clean-up
    sentFile.close();
    receivedFile.close();

    std::cout << "Animation Trace file created:" << animFile << std::endl;
    Simulator::Destroy();

    // Remove temporary files
    // std::remove("tcp-sent-forward.txt");
    // std::remove("tcp-received-forward.txt");
    return 0;
}