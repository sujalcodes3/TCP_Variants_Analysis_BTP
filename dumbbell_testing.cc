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
 */

#include "ns3/applications-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"

#include <iostream>

using namespace ns3;

int
main(int argc, char* argv[])
{
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(512));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("500kb/s"));

    uint32_t nLeftLeaf = 3;
    uint32_t nRightLeaf = 3;
    uint32_t nLeaf = 0;                                // If non-zero, number of both left and right
    std::string animFile = "dumbbell-animation_2.xml"; // Name of file for animation output

    CommandLine cmd(__FILE__);
    cmd.AddValue("nLeftLeaf", "Number of left side leaf nodes", nLeftLeaf);
    cmd.AddValue("nRightLeaf", "Number of right side leaf nodes", nRightLeaf);
    cmd.AddValue("nLeaf", "Number of left and right side leaf nodes", nLeaf);
    cmd.AddValue("animFile", "File Name for Animation Output", animFile);

    cmd.Parse(argc, argv);

    if (nLeaf > 0)
    {
        nLeftLeaf = nLeaf;
        nRightLeaf = nLeaf;
    }

    // Create the point-to-point link helpers
    PointToPointHelper pointToPointRouter;
    pointToPointRouter.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPointRouter.SetChannelAttribute("Delay", StringValue("1ms"));

    PointToPointHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPointLeaf.SetChannelAttribute("Delay", StringValue("1ms"));

    PointToPointDumbbellHelper d1(nLeftLeaf,
                                  pointToPointLeaf,
                                  nRightLeaf,
                                  pointToPointLeaf,
                                  pointToPointRouter);

    // // Create the point-to-point link helpers
    // PointToPointHelper pointToPointRouter2;
    // pointToPointRouter2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    // pointToPointRouter2.SetChannelAttribute("Delay", StringValue("1ms"));
    // PointToPointHelper pointToPointLeaf2;
    // pointToPointLeaf2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    // pointToPointLeaf2.SetChannelAttribute("Delay", StringValue("1ms"));

    // PointToPointDumbbellHelper d2(1, pointToPointLeaf2, 1, pointToPointLeaf2,
    // pointToPointRouter2);

    // Install Stack
    InternetStackHelper stack;
    d1.InstallStack(stack);

    // Assign IP Addresses
    d1.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                           Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                           Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

    // InternetStackHelper stack2;
    // d2.InstallStack(stack2);

    // // Assign IP Addresses
    // d2.AssignIpv4Addresses(Ipv4AddressHelper("20.1.1.0", "255.255.255.0"),
    //                        Ipv4AddressHelper("20.2.1.0", "255.255.255.0"),
    //                        Ipv4AddressHelper("20.3.1.0", "255.255.255.0"));

    // Install on/off app on all right side nodes

    OnOffHelper upHelper("ns3::UdpSocketFactory", Address());
    // clientHelper.SetAttribute("OnTime", StringValue("ns3::UniformRandomVariable"));
    // clientHelper.SetAttribute("OffTime", StringValue("ns3::UniformRandomVariable"));

    ApplicationContainer udpUpperApp;

    AddressValue remoteAddress1(InetSocketAddress(d1.GetRightIpv4Address(0), 1000));
    upHelper.SetAttribute("Remote", remoteAddress1);
    udpUpperApp.Add(upHelper.Install(d1.GetLeft(0)));

    udpUpperApp.Start(Seconds(1.0));
    udpUpperApp.Stop(Seconds(10.0));

    // Create a packet sink on the receiver
    Address UpperSinkAddress(InetSocketAddress(d1.GetRightIpv4Address(0), 1000));
    PacketSinkHelper upperUdpSinkHelper("ns3::UdpSocketFactory", UpperSinkAddress);
    ApplicationContainer upperUdpSinkApp = upperUdpSinkHelper.Install(d1.GetRight(0));
    upperUdpSinkApp.Start(Seconds(1.0));
    upperUdpSinkApp.Stop(Seconds(10.0));

    // lower udp on off hepleres
    OnOffHelper downHelper("ns3::UdpSocketFactory", Address());

    ApplicationContainer udpLowerApp;

    AddressValue remoteAddress2(InetSocketAddress(d1.GetRightIpv4Address(2), 2000));
    downHelper.SetAttribute("Remote", remoteAddress2);
    udpLowerApp.Add(downHelper.Install(d1.GetLeft(2)));

    udpLowerApp.Start(Seconds(1.0));
    udpLowerApp.Stop(Seconds(10.0));

    // create a udp sink on lower right side
    Address LowerSinkAddress(InetSocketAddress(d1.GetRightIpv4Address(2), 2000));
    PacketSinkHelper lowerUdpSinkHelper("ns3::UdpSocketFactory", LowerSinkAddress);

    ApplicationContainer lowerUdpSinkApp = lowerUdpSinkHelper.Install(d1.GetRight(2));

    lowerUdpSinkApp.Start(Seconds(1.0));
    lowerUdpSinkApp.Stop(Seconds(10.0));

    // TCP Sender
    uint16_t port = 50000;

    OnOffHelper tcpHelper("ns3::TcpSocketFactory",
                          InetSocketAddress(d1.GetRightIpv4Address(1), port));

    ApplicationContainer TcpNode;
    TcpNode.Add(tcpHelper.Install(d1.GetLeft(1)));

    TcpNode.Start(Seconds(1.0));
    TcpNode.Stop(Seconds(10.0));

    //  TCP Reciever sink

    Address sinkLocalAddress2(InetSocketAddress(d1.GetRightIpv4Address(1), port));
    PacketSinkHelper sinkHelper2("ns3::TcpSocketFactory", sinkLocalAddress2);

    ApplicationContainer serverNode = sinkHelper2.Install(d1.GetRight(1));
    serverNode.Start(Seconds(1.0));
    serverNode.Stop(Seconds(10.0));

    // Set the bounding box for animation
    d1.BoundingBox(10, 10, 90, 90);

    // Install on/off app on all right side nodes
    // OnOffHelper clientHelper2("ns3::UdpSocketFactory", Address());
    // clientHelper2.SetAttribute("OnTime", StringValue("ns3::UniformRandomVariable"));
    // clientHelper2.SetAttribute("OffTime", StringValue("ns3::UniformRandomVariable"));

    // AddressValue remoteAddressTcp(InetSocketAddress(d1.GetLeftIpv4Address(1), port));
    // clientHelper2.SetAttribute("Remote", remoteAddressTcp);
    // ApplicationContainer clientApps2;

    // for (uint32_t i = 0;
    //      i < ((d2.RightCount() < d2.LeftCount()) ? d2.RightCount() : d2.LeftCount());
    //      ++i)
    // {
    //     // Create an on/off app sending packets to the same leaf right side
    //     AddressValue remoteAddress(InetSocketAddress(d2.GetLeftIpv4Address(i), 1000));
    //     clientHelper2.SetAttribute("Remote", remoteAddress);
    //     clientApps2.Add(clientHelper2.Install(d2.GetRight(i)));
    // }

    // clientApps2.Start(Seconds(0.0));
    // clientApps2.Stop(Seconds(10.0));

    // // Set the bounding box for animation
    // d2.BoundingBox(1, 60, 90, 90);

    // NodeContainer nodes;
    // nodes.Create(4);

    // PointToPointHelper pointToPoint;
    // pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    // pointToPoint.SetChannelAttribute("Delay", StringValue("1ms"));

    // NetDeviceContainer devices, c1, c2, c3, c4, c5;

    // c1 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    // c2 = pointToPoint.Install(nodes.Get(0), nodes.Get(2));

    // c3 = pointToPoint.Install(nodes.Get(1), nodes.Get(3));

    // c4 = pointToPoint.Install(nodes.Get(0), d1.GetLeft(1));
    // c5 = pointToPoint.Install(nodes.Get(1), d1.GetRight(1));

    // InternetStackHelper stack3;
    // stack3.Install(nodes);

    // Ipv4AddressHelper address;
    // address.SetBase("30.1.1.0", "255.255.255.0");
    // Ipv4InterfaceContainer interfaceC1 = address.Assign(c1);
    // Ipv4InterfaceContainer interfaceC2 = address.Assign(c2);
    // Ipv4InterfaceContainer interfaceC3 = address.Assign(c3);
    // Ipv4InterfaceContainer interfaceC4 = address.Assign(c4);
    // Ipv4InterfaceContainer interfaceC5 = address.Assign(c5);

    // // UDP traffic
    // OnOffHelper clientHelperRight("ns3::UdpSocketFactory", Address());

    // ApplicationContainer clientAppsDown;
    // AddressValue remoteAddressRight1(InetSocketAddress(d1.GetRightIpv4Address(1), 2000));
    // clientHelperRight.SetAttribute("Remote", remoteAddressRight1);
    // clientAppsDown.Add(clientHelperRight.Install(nodes.Get(2)));
    // clientAppsDown.Add(clientHelperRight.Install(nodes.Get(3)));

    // clientAppsDown.Start(Seconds(1.0));
    // clientAppsDown.Stop(Seconds(10.0));

    // // UDP Traffic Sink
    // Address UpdRightsinkAddress(InetSocketAddress(d1.GetRightIpv4Address(1), 2000));
    // PacketSinkHelper UdpRightSinkHelper("ns3::UdpSocketFactory", UpdRightsinkAddress);
    // ApplicationContainer udpRightSink = UdpRightSinkHelper.Install(d1.GetRight(1));
    // udpRightSink.Start(Seconds(1.0));
    // udpRightSink.Stop(Seconds(10.0));

    // MobilityHelper mobility;
    // mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    // mobility.Install(nodes);

    // Create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    // Ptr<ConstantPositionMobilityModel> s1 =
    //     nodes.Get(0)->GetObject<ConstantPositionMobilityModel>();

    // Ptr<ConstantPositionMobilityModel> s2 =
    //     nodes.Get(1)->GetObject<ConstantPositionMobilityModel>();

    // Ptr<ConstantPositionMobilityModel> s3 =
    //     nodes.Get(2)->GetObject<ConstantPositionMobilityModel>();

    // Ptr<ConstantPositionMobilityModel> s4 =
    //     nodes.Get(3)->GetObject<ConstantPositionMobilityModel>();

    // s1->SetPosition(Vector(20.0, 51.0, 0));
    // s2->SetPosition(Vector(80.0, 51.0, 0));
    // s3->SetPosition(Vector(5.0, 64.0, 0));
    // s4->SetPosition(Vector(95.0, 64.0, 0));

    anim.EnablePacketMetadata();                                // Optional
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional

    // Set up the actual simulation
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    std::cout << "Animation Trace file created:" << animFile << std::endl;
    Simulator::Destroy();
    return 0;
}
