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
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/ipv4-flow-classifier.h"


using namespace ns3;
using namespace std;

typedef NetDeviceContainer NDC;
typedef Ipv4InterfaceContainer Ipv4IC;

int main(int argc, char* argv[]) {

    std::string animFile = "test.xml"; 
    string drate; 
    CommandLine cmd(__FILE__);
    cmd.AddValue("rate", "rate define", drate);
    cmd.Parse(argc, argv);
    
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    
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
    p2p.SetDeviceAttribute("DataRate", StringValue(drate));
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
    tcpSourceHelper.SetAttribute("DataRate", DataRateValue(DataRate(drate)));
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

    // Udp taffic from T0 TO T3
    InetSocketAddress udpSinkAddr1(IfaceRT13.GetAddress(1), 4001);

    OnOffHelper udpSourceHelper1("ns3::UdpSocketFactory", Address());
    udpSourceHelper1.SetAttribute("DataRate", DataRateValue(DataRate("512Kbps")));
    udpSourceHelper1.SetAttribute("Remote", AddressValue(udpSinkAddr1));
    udpSourceHelper1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSourceHelper1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpSources1;
    udpSources1.Add(udpSourceHelper1.Install(TrafficNodes.Get(0)));
    udpSources1.Start(Seconds(1.0));
    udpSources1.Stop(Seconds(10.0));

    PacketSinkHelper udpSinkHelper1("ns3::UdpSocketFactory", udpSinkAddr1);

    ApplicationContainer udpSinks1;
    udpSinks1.Add(udpSinkHelper1.Install(TrafficNodes.Get(3)));
    udpSinks1.Start(Seconds(1.0));
    udpSinks1.Stop(Seconds(10.0));


    //udp taffic from T2 TO T1
    InetSocketAddress udpSinkAddr2(IfaceRT11.GetAddress(1), 4002);

    OnOffHelper udpSourceHelper2("ns3::UdpSocketFactory", Address());
    udpSourceHelper2.SetAttribute("Remote", AddressValue(udpSinkAddr2));
    udpSourceHelper2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSourceHelper2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpSources2;
    udpSources2.Add(udpSourceHelper2.Install(TrafficNodes.Get(2)));
    udpSources2.Start(Seconds(1.0));
    udpSources2.Stop(Seconds(10.0));

    PacketSinkHelper udpSinkHelper2("ns3::UdpSocketFactory", udpSinkAddr2);

    ApplicationContainer udpSinks2;
    udpSinks2.Add(udpSinkHelper2.Install(TrafficNodes.Get(1)));

    udpSinks2.Start(Seconds(1.0));
    udpSinks2.Stop(Seconds(10.0));


    // Flow monitor
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();


    AnimationInterface anim(animFile);
    anim.EnablePacketMetadata();                                // Optional
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional
    //anim.EnableIpv4RouteTracking("RoutingTracking.xml", Seconds(1), Seconds(10), MilliSeconds(100));
   

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

    Simulator::Stop(Seconds(10));
    Simulator::Run();

    flowMonitor->CheckForLostPackets (); 
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

        if(iter -> first == 1) {
            //NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
            //NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
            //NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
            cout << "Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress << endl;
            cout << "Tx Packets = " << iter->second.txPackets << endl;
            cout << "Rx Packets = " << iter->second.rxPackets << endl;
            
            // Calculate Mean Delay
            double meanDelay = 0;
            if (iter->second.rxPackets > 0)
            {
                meanDelay = (iter->second.delaySum.GetSeconds() / iter->second.rxPackets);
            }
            cout << "Mean Delay: " << meanDelay * 1000 << "ms" << endl;

            // Calculate Packet Loss
            uint32_t lostPackets = iter->second.txPackets - iter->second.rxPackets;
            double packetLossRatio = 0;
            if (iter->second.txPackets > 0)
            {
                packetLossRatio = static_cast<double>(lostPackets) / iter->second.txPackets;
            }
            cout << "Lost Packets = " << lostPackets << endl;
            cout << "Packet Loss Ratio = " << packetLossRatio * 100 << endl;
        }
    }
    // Calculate TCP Throughput
    Ptr<PacketSink> tcpSink = DynamicCast<PacketSink>(tcpSinks.Get(0));
    uint64_t tcpTotalBytes = tcpSink->GetTotalRx();
    double simulationDuration = 9.0; // Duration over which the throughput is calculated (10s - 1s)
    double tcpThroughput = (tcpTotalBytes * 8.0) / (simulationDuration * 1024.0); // Convert to Kbps
    cout << "TCP Throughput: " << tcpThroughput << " Kbps" << endl;
    Simulator::Destroy();

    std::cout << std::endl;
    return 0;
}
