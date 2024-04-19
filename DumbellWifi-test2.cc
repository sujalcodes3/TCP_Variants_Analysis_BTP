//Dumbbell topology with 2 routers, 4 traffic nodes, 1 server and 1 client
//with a wireless connection between the routers.
//without any taffic created by UDP nodes.


/* TOPOLOGY 
 
           T0                  T1
             \                / 
              \              /
       E0----- R0---------- R1-------- E1
(clientNode)   /             \         (serverNode)
              /               \
            T2                 T3
 */

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
using namespace ns3;

int main(int argc, char* argv[]) {
    std::string animFile = "dumbbell-animation-test2.xml"; 
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
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));


    // ROUTER and TRAFFIC NODE LINKS
    NetDeviceContainer linkRT00 = p2p.Install(Routers.Get(0), TrafficNodes.Get(0));
    NetDeviceContainer linkRT02 = p2p.Install(Routers.Get(0), TrafficNodes.Get(2));
    NetDeviceContainer linkRT11 = p2p.Install(Routers.Get(1), TrafficNodes.Get(1));
    NetDeviceContainer linkRT13 = p2p.Install(Routers.Get(1), TrafficNodes.Get(3));
    
    // ROUTER LINKS
    // if there is a p2p link between the routers.
    //NetDeviceContainer linkRR01 = p2p.Install(Routers.Get(0), Routers.Get(1));
     
    // if there is a wireless link between the routers.    
    // set up a wireless connection between the two router
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;

    NetDeviceContainer routers;
    mac.SetType("ns3::AdhocWifiMac", "Ssid", SsidValue(ssid));
    routers = wifi.Install(phy, mac, Routers);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(Routers);

    // ENDNODES ROUTER LINK
    NetDeviceContainer linkRE0 = p2p.Install(Routers.Get(0), EndNodes.Get(0));
    NetDeviceContainer linkRE1 = p2p.Install(Routers.Get(1), EndNodes.Get(1));

    InternetStackHelper stack;
    stack.Install(Routers);
    stack.Install(TrafficNodes);
    stack.Install(EndNodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer IfaceRT00 = address.Assign(linkRT00);
    Ipv4InterfaceContainer IfaceRT02 = address.Assign(linkRT02);
    Ipv4InterfaceContainer IfaceRE0 = address.Assign(linkRE0);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer IfaceRT11 = address.Assign(linkRT11);
    Ipv4InterfaceContainer IfaceRT13 = address.Assign(linkRT13);
    Ipv4InterfaceContainer IfaceRE1 = address.Assign(linkRE1);

    // Router Interface
    address.SetBase("10.1.3.0", "255.255.255.0");
    //Ipv4InterfaceContainer interfaceRR01 = address.Assign(linkRR01);
    Ipv4InterfaceContainer interR0 = address.Assign(routers);
    //Ipv4InterfaceContainer interR1 = address.Assign(staRouter);

    uint16_t port = 50000;
    // TCP Sender
    OnOffHelper tcpSourceHelper("ns3::TcpSocketFactory", InetSocketAddress(IfaceRE1.GetAddress(1), port)); 
    tcpSourceHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    tcpSourceHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    //tcpSourceHelper.SetAttribute("DataRate", DataRateValue(DataRate("10Mbps")));  

    ApplicationContainer tcpClientSource = tcpSourceHelper.Install(EndNodes.Get(0));
    tcpClientSource.Start(Seconds(1.0));  
    tcpClientSource.Stop(Seconds(10.0));
    
    // TCP Reciever
    Address sinkLocalAddress(InetSocketAddress(IfaceRE1.GetAddress(1), port));
    PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer tcpServerSink = tcpSinkHelper.Install(EndNodes.Get(1));
    tcpServerSink.Start(Seconds(1.0));
    tcpServerSink.Stop(Seconds(10.0));
    
    
    
    

    AnimationInterface anim(animFile);
    anim.EnablePacketMetadata();                                // Optional
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional
    anim.SetConstantPosition(Routers.Get(0), 25, 44);
    
    Routers.Get(0);
    anim.SetConstantPosition(Routers.Get(1), 44, 44);

    // ENDNODES
    anim.SetConstantPosition(EndNodes.Get(0), 0, 44);
    anim.SetConstantPosition(EndNodes.Get(1), 75, 44);

    // TRAFFIC NODES
    anim.SetConstantPosition(TrafficNodes.Get(0), 10, 24);
    anim.SetConstantPosition(TrafficNodes.Get(1), 65, 24);
    anim.SetConstantPosition(TrafficNodes.Get(2), 10, 64);
    anim.SetConstantPosition(TrafficNodes.Get(3), 65, 64);
    
    // Set up the actual simulation
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    std::cout << "Animation Trace file created:" << animFile << std::endl;
    Simulator::Destroy();
    return 0;
}

