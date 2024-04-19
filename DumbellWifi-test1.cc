//Dumbbell topology with 2 routers, 4 traffic nodes, 1 server and 1 client
//with a p2p connection between the routers.
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

#include <cstdlib>
#include <iostream>

using namespace ns3;

int main(int argc, char* argv[])
{
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(512));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("440kb/s"));

    std::string animFile = "dumbbell-animation-test1.xml"; // Name of file for animation output

    CommandLine cmd(__FILE__);
    cmd.AddValue("animFile", "File Name for Animation Output", animFile);

    cmd.Parse(argc, argv);

    // Create the point-to-point link helpers
    PointToPointHelper pointToPointRouter;
    pointToPointRouter.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPointRouter.SetChannelAttribute("Delay", StringValue("1ms"));
    
    PointToPointHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPointLeaf.SetChannelAttribute("Delay", StringValue("1ms"));

    PointToPointHelper pointToPointNodeRouter;
    pointToPointNodeRouter.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPointNodeRouter.SetChannelAttribute("Delay", StringValue("1ms"));

    // 4 routers
    NodeContainer Routers;
    Routers.Create(2);

    // 4 traffic nodes
    NodeContainer TrafficNodes;
    TrafficNodes.Create(4);

    // 1 server and 1 client 
    NodeContainer EndNodes;
    EndNodes.Create(2);


    InternetStackHelper stack;
    stack.Install(Routers);
    stack.Install(TrafficNodes);
    stack.Install(EndNodes);
    
    // ROUTER and TRAFFIC NODE LINKS
    NetDeviceContainer linkRT00 = pointToPointNodeRouter.Install(Routers.Get(0), TrafficNodes.Get(0));
    NetDeviceContainer linkRT02 = pointToPointNodeRouter.Install(Routers.Get(0), TrafficNodes.Get(2));
    NetDeviceContainer linkRT11 = pointToPointNodeRouter.Install(Routers.Get(1), TrafficNodes.Get(1));
    NetDeviceContainer linkRT13 = pointToPointNodeRouter.Install(Routers.Get(1), TrafficNodes.Get(3));
    
    // ROUTER LINKS
    NetDeviceContainer linkRR01 = pointToPointRouter.Install(Routers.Get(0), Routers.Get(1));

    // ENDNODES ROUTER LINK
    NetDeviceContainer linkRE0 = pointToPointLeaf.Install(Routers.Get(0), EndNodes.Get(0));
    NetDeviceContainer linkRE1 = pointToPointLeaf.Install(Routers.Get(1), EndNodes.Get(1));


    
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaceRT00 = address.Assign(linkRT00);
    Ipv4InterfaceContainer interfaceRT02 = address.Assign(linkRT02);
    Ipv4InterfaceContainer interfaceRE0 = address.Assign(linkRE0);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaceRT11 = address.Assign(linkRT11);
    Ipv4InterfaceContainer interfaceRT13 = address.Assign(linkRT13);
    Ipv4InterfaceContainer interfaceRE1 = address.Assign(linkRE1);

    // Router Interface
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaceRR01 = address.Assign(linkRR01);




    // TCP Sender
    OnOffHelper clientHelper("ns3::TcpSocketFactory", InetSocketAddress(interfaceRE1.GetAddress(1), 50000)); 
    ApplicationContainer clientEndNode = clientHelper.Install(EndNodes.Get(0));
    clientEndNode.Start(Seconds(1.0));  
    clientEndNode.Stop(Seconds(10.0));
    
    // TCP Reciever
    uint16_t port = 50000;
    Address sinkLocalAddress(InetSocketAddress(interfaceRE1.GetAddress(1), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer serverEndNode = sinkHelper.Install(EndNodes.Get(1));
    serverEndNode.Start(Seconds(1.0));
    serverEndNode.Stop(Seconds(10.0));

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
