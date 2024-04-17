/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: George F. Riley<riley@ece.gatech.edu>
 *
 *
 *
 */

/* TOPOLOGY 
 
           T0                  T1
             \                / 
              \              /
       E0----- R0---------- R1-------- E1
(clientNode)   /             \         (serverNode)
              /               \
            T2                 T3
 */
/*
// set up a wireless connection between the two router
YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
YansWifiPhyHelper wifiPhy = YansWifiPhyHelper();
wifiPhy.SetChannel(wifiChannel.Create());

WifiHelper wifi;
//wifi.SetRemoteStationManager("ns3::AarfWifiManager");

WifiMacHelper wifiMac;
Ssid ssid = Ssid("InterRouterNetwork");
wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
NetDeviceContainer apRouter = wifi.Install(wifiPhy, wifiMac, Routers.Get(0));

wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
NetDeviceContainer staRouter = wifi.Install(wifiPhy, wifiMac, Routers.Get(1));

// Set up mobility
MobilityHelper mobility;
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
mobility.Install(Routers.Get(1));
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
using namespace ns3;

int main(int argc, char* argv[]) {
    
        Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(512));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("440kb/s"));

    std::string animFile = "dumbell-second.xml"; 
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Create nodes;
    NodeContainer EndNodes;
    NodeContainer Routers;
    NodeContainer TrafficNodes;

    EndNodes.Create(2);
    Routers.Create(2);
    TrafficNodes.Create(4);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));


    // Set up the links between the routers and the end nodes  
    NetDeviceContainer erl = p2p.Install(EndNodes.Get(0), Routers.Get(0));
    NetDeviceContainer t0rl= p2p.Install(TrafficNodes.Get(0), Routers.Get(0));
    NetDeviceContainer t1rl= p2p.Install(TrafficNodes.Get(1), Routers.Get(0));
    NetDeviceContainer err = p2p.Install(Routers.Get(1), EndNodes.Get(1)); 
    NetDeviceContainer t2rr = p2p.Install(TrafficNodes.Get(2), Routers.Get(1)); 
    NetDeviceContainer t3rr = p2p.Install(TrafficNodes.Get(3), Routers.Get(1)); 
   
    NetDeviceContainer rr = p2p.Install(Routers.Get(0), Routers.Get(1)); 

    InternetStackHelper stack;
    stack.Install(Routers);
    stack.Install(TrafficNodes);
    stack.Install(EndNodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer erlIface = address.Assign(erl);
    Ipv4InterfaceContainer t0rlIface = address.Assign(t0rl);
    Ipv4InterfaceContainer t1rlIface = address.Assign(t1rl);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer errIface = address.Assign(err);
    Ipv4InterfaceContainer t2rrIface = address.Assign(t2rr);
    Ipv4InterfaceContainer t3rrIface = address.Assign(t3rr);
    
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer routerInter = address.Assign(rr);
    //Ipv4InterfaceContainer wifiIface = address.Assign(apRouter);

    OnOffHelper tcpSender("ns3::TcpSocketFactory", InetSocketAddress(errIface.GetAddress(1), 8080));
    ApplicationContainer clientNode = tcpSender.Install(EndNodes.Get(0));
    clientNode.Start(Seconds(1.0));
    clientNode.Start(Seconds(10.0));

    Address serverSinkAddress(InetSocketAddress(errIface.GetAddress(1), 8080));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", serverSinkAddress);
    ApplicationContainer serverNode = sinkHelper.Install(EndNodes.Get(1));

    serverNode.Start(Seconds(1.0));
    serverNode.Start(Seconds(10.0));

    AnimationInterface anim(animFile);
    anim.EnablePacketMetadata();                                // Optional
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional
    anim.SetConstantPosition(Routers.Get(0), 25, 44);
    
    Routers.Get(0);
    anim.SetConstantPosition(Routers.Get(1), 50, 44);

    // ENDNODES
    anim.SetConstantPosition(EndNodes.Get(0), 0, 44);
    anim.SetConstantPosition(EndNodes.Get(1), 75, 44);

    // TRAFFIC NODES
    anim.SetConstantPosition(TrafficNodes.Get(0), 12, 24);
    anim.SetConstantPosition(TrafficNodes.Get(1), 12, 64);
    anim.SetConstantPosition(TrafficNodes.Get(2), 62, 24);
    anim.SetConstantPosition(TrafficNodes.Get(3), 62, 64);

    // Set up the actual simulation
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    std::cout << "Animation Trace file created:" << animFile << std::endl;
    Simulator::Destroy();
    return 0;
}

