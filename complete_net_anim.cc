/*
 * this represents a network of a small city.
 * it comprises of 3 local networks
 * 1.)CSMA LAN network inside a hospital
 * 2.)CSMA LAN network inside a bank
 * 3.)WIFI LAN network inside the school
 *
 * all these networks are client system which try to access a single server.
 * this single server represents all the internet services.
 * for simplicity we used single server in order to better understand the packet forwarding mechanism.
 *
 * the protocol used for communication in UDP. we choose UDP instead of TCP, because UDP is simpler and
 * demostrates the packet flow at lower level
 * */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

///modules for netanim
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CompleteNetworkExample");

int 
main (int argc, char *argv[])
{

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

//////////////////////////// construct the P2P network /////////////////////////////////////////
  /// setup nodes and channels
  NodeContainer p2pNodes;
  p2pNodes.Create (10);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  ///path n0-n1-n4-n7
  NodeContainer n0n1 = NodeContainer(p2pNodes.Get(0),p2pNodes.Get(1));
  NodeContainer n1n4 = NodeContainer(p2pNodes.Get(1),p2pNodes.Get(4));
  NodeContainer n4n7 = NodeContainer(p2pNodes.Get(4),p2pNodes.Get(7));

  NetDeviceContainer d0d1 = pointToPoint.Install(n0n1);
  NetDeviceContainer d1d4 = pointToPoint.Install(n1n4);
  NetDeviceContainer d4d7 = pointToPoint.Install(n4n7);

  ///path n0-n2-n5-n8
  NodeContainer n0n2 = NodeContainer(p2pNodes.Get(0),p2pNodes.Get(2));
  NodeContainer n2n5 = NodeContainer(p2pNodes.Get(2),p2pNodes.Get(5));
  NodeContainer n5n8 = NodeContainer(p2pNodes.Get(5),p2pNodes.Get(8));

  NetDeviceContainer d0d2 = pointToPoint.Install(n0n2);
  NetDeviceContainer d2d5 = pointToPoint.Install(n2n5);
  NetDeviceContainer d5d8 = pointToPoint.Install(n5n8);

  ///path n0-n3-n6-n9
  NodeContainer n0n3 = NodeContainer(p2pNodes.Get(0),p2pNodes.Get(3));
  NodeContainer n3n6 = NodeContainer(p2pNodes.Get(3),p2pNodes.Get(6));
  NodeContainer n6n9 = NodeContainer(p2pNodes.Get(6),p2pNodes.Get(9));

  NetDeviceContainer d0d3 = pointToPoint.Install(n0n3);
  NetDeviceContainer d3d6 = pointToPoint.Install(n3n6);
  NetDeviceContainer d6d9 = pointToPoint.Install(n6n9);


  /////////////////// setup csma networks ///////////////////////////
  ////setup csma nodes,channels
  NodeContainer hospital_csma_nodes;
  hospital_csma_nodes.Add(p2pNodes.Get(7));
  hospital_csma_nodes.Create(3);///total 4 nodes in the hospital system

  NodeContainer school_csma_nodes;
  school_csma_nodes.Add(p2pNodes.Get(8));
  school_csma_nodes.Create(3);///total 4 nodes in the school system

  NodeContainer bank_csma_nodes;
  bank_csma_nodes.Add (p2pNodes.Get(9));
  bank_csma_nodes.Create(3);///total 4 nodes in the bank system

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer HospitalDevices = csma.Install (hospital_csma_nodes);
  NetDeviceContainer SchoolDevices = csma.Install (school_csma_nodes);
  NetDeviceContainer BankDevices = csma.Install (bank_csma_nodes);

  ///install internet stack on every node
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (p2pNodes.Get (1));
  stack.Install (p2pNodes.Get (2));
  stack.Install (p2pNodes.Get (3));
  stack.Install (p2pNodes.Get (4));
  stack.Install (p2pNodes.Get (5));
  stack.Install (p2pNodes.Get (6));

  stack.Install (hospital_csma_nodes);
  stack.Install (school_csma_nodes);
  stack.Install (bank_csma_nodes);

  /// assign ip address to P2P nodes
  Ipv4AddressHelper address;
  /// n0-n1-n4-n7
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = address.Assign(d0d1);
  Ipv4InterfaceContainer i1i4 = address.Assign(d1d4);
  Ipv4InterfaceContainer i4i7 = address.Assign(d4d7);

  /// n0-n2-n5-n8
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = address.Assign(d0d2);
  Ipv4InterfaceContainer i2i5 = address.Assign(d2d5);
  Ipv4InterfaceContainer i5i8 = address.Assign(d5d8);

  /// n0-n3-n6-n9
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i3 = address.Assign(d0d3);
  Ipv4InterfaceContainer i3i6 = address.Assign(d3d6);
  Ipv4InterfaceContainer i6i9 = address.Assign(d6d9);

  /// assign ip address to CSMA nodes
  address.SetBase ("10.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer HospitalInterfaces = address.Assign(HospitalDevices);
  address.SetBase ("10.2.2.0", "255.255.255.0");
  Ipv4InterfaceContainer SchoolInterfaces = address.Assign(SchoolDevices);
  address.SetBase ("10.2.3.0", "255.255.255.0");
  Ipv4InterfaceContainer BankInterfaces = address.Assign(BankDevices);

  //////////////////////////////// install applications on the nodes ////////////////////////////////////
  ///install server on n0
  UdpEchoServerHelper echoServer (9);///run server on port 9
  ApplicationContainer serverApps = echoServer.Install (hospital_csma_nodes.Get(2));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  /// install client app on 2nd node in hospital local network
  UdpEchoClientHelper echoClient (HospitalInterfaces.GetAddress(2), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  ///configure the netanim file
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
              "MinX", DoubleValue (0.0), "MinY", DoubleValue (0.0),"DeltaX", DoubleValue (5.0), "DeltaY", DoubleValue (10.0),
               "GridWidth", UintegerValue (5), "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (p2pNodes);
  mobility.Install (hospital_csma_nodes);
  mobility.Install (school_csma_nodes);
  mobility.Install (bank_csma_nodes);

  AnimationInterface anim ("complete_routing.xml");
  anim.SetConstantPosition (p2pNodes.Get(0), 3, 8);

  anim.SetConstantPosition (p2pNodes.Get(1), 1, 6);
  anim.SetConstantPosition (p2pNodes.Get(2), 3, 6);
  anim.SetConstantPosition (p2pNodes.Get(3), 5, 6);

  anim.SetConstantPosition (p2pNodes.Get(1), 1, 4);
  anim.SetConstantPosition (p2pNodes.Get(2), 3, 4);
  anim.SetConstantPosition (p2pNodes.Get(3), 5, 4);

  anim.SetConstantPosition (p2pNodes.Get(1), 1, 2);
  anim.SetConstantPosition (p2pNodes.Get(2), 3, 2);
  anim.SetConstantPosition (p2pNodes.Get(3), 5, 2);

  anim.SetConstantPosition (hospital_csma_nodes.Get(1), 1, 1);
  anim.SetConstantPosition (hospital_csma_nodes.Get(2), 2, 1);
  anim.SetConstantPosition (hospital_csma_nodes.Get(3), 3, 1);

  anim.SetConstantPosition (school_csma_nodes.Get(1), 4, 1);
  anim.SetConstantPosition (school_csma_nodes.Get(2), 5, 1);
  anim.SetConstantPosition (school_csma_nodes.Get(3), 6, 1);

  anim.SetConstantPosition (bank_csma_nodes.Get(1), 7, 1);
  anim.SetConstantPosition (bank_csma_nodes.Get(2), 8, 1);
  anim.SetConstantPosition (bank_csma_nodes.Get(3), 9, 1);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
