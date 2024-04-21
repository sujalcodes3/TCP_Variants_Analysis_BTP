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
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor.h"

using namespace ns3;

typedef NetDeviceContainer NDC;
typedef Ipv4InterfaceContainer Ipv4IC;

int main(int argc, char* argv[]) {
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
    std::string animFile = "flow_reverse.xml"; 
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
    p2p.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
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
    tcpSourceHelper.SetAttribute("DataRate", DataRateValue(DataRate("2Mbps")));
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

    // Udp taffic from T3 TO T0
    InetSocketAddress udpSinkAddr1(IfaceRT00.GetAddress(1), 4001);

    OnOffHelper udpSourceHelper1("ns3::UdpSocketFactory", Address());
    udpSourceHelper1.SetAttribute("Remote", AddressValue(udpSinkAddr1));
    udpSourceHelper1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSourceHelper1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpSources1;
    udpSources1.Add(udpSourceHelper1.Install(TrafficNodes.Get(3)));
    udpSources1.Start(Seconds(1.0));
    udpSources1.Stop(Seconds(10.0));

    PacketSinkHelper udpSinkHelper1("ns3::UdpSocketFactory", udpSinkAddr1);

    ApplicationContainer udpSinks1;
    udpSinks1.Add(udpSinkHelper1.Install(TrafficNodes.Get(0)));
    udpSinks1.Start(Seconds(1.0));
    udpSinks1.Stop(Seconds(10.0));


    //udp taffic from T1 to T2
    InetSocketAddress udpSinkAddr2(IfaceRT02.GetAddress(1), 4002);

    OnOffHelper udpSourceHelper2("ns3::UdpSocketFactory", Address());
    udpSourceHelper2.SetAttribute("Remote", AddressValue(udpSinkAddr2));
    udpSourceHelper2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSourceHelper2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpSources2;
    udpSources2.Add(udpSourceHelper2.Install(TrafficNodes.Get(1)));
    udpSources2.Start(Seconds(1.0));
    udpSources2.Stop(Seconds(10.0));

    PacketSinkHelper udpSinkHelper2("ns3::UdpSocketFactory", udpSinkAddr2);

    ApplicationContainer udpSinks2;
    udpSinks2.Add(udpSinkHelper2.Install(TrafficNodes.Get(2)));

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

    std::cout << "Animation Trace file created:" << animFile << std::endl;
    Simulator::Destroy();
    flowMonitor->SerializeToXmlFile("flow_reverse.xml", true, true);
    return 0;
}