/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
 *
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
 * Authors: Ankit Deepak <adadeepak8@gmail.com>
 *          Shravya K. S. <shravya.ks0@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include "eval-topology.h"
#include "eval-ts.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvaluationTopology");

NS_OBJECT_ENSURE_REGISTERED (EvaluationTopology);

TypeId
EvaluationTopology::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EvaluationTopology")
    .SetGroupName ("AqmEvaluationSuite")
  ;
  return tid;
}

EvaluationTopology::EvaluationTopology (std::string ScenarioName, uint32_t numFlows,
                                        PointToPointHelper p2pHelper, std::string queueDisc, uint32_t pktSize, bool isBql)
  : m_dumbbell (numFlows, p2pHelper, numFlows, p2pHelper, p2pHelper)
{
  m_numFlows = numFlows;
  m_flowsAdded = 0;
  m_packetSize = pktSize;
  bool m_isBql = isBql;

  InternetStackHelper stack;
  m_dumbbell.InstallStack (stack);

  TrafficControlHelper tch;
  m_dumbbell.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                                  Ipv4AddressHelper ("10.10.1.0", "255.255.255.0"),
                                  Ipv4AddressHelper ("10.100.1.0", "255.255.255.0"));
  tch.Uninstall (m_dumbbell.GetLeft ()->GetDevice (0));

  if (m_isBql)
    {
      tch.SetQueueLimits ("ns3::DynamicQueueLimits");
    }

  m_currentAQM = queueDisc;
  if (queueDisc == "ns3::AdaptiveRedQueueDisc" || queueDisc == "ns3::FengAdaptiveRedQueueDisc" || queueDisc == "ns3::NonLinearRedQueueDisc")
    {
      queueDisc = "ns3::RedQueueDisc";
      tch.SetRootQueueDisc (queueDisc.c_str ());
      m_queue = tch.Install (m_dumbbell.GetLeft ()->GetDevice (0)).Get (0);
      if (m_currentAQM == "ns3::AdaptiveRedQueueDisc")
        {
          m_queue->SetAttribute ("ARED", BooleanValue (true));
        }
      else if (m_currentAQM == "ns3::FengAdaptiveRedQueueDisc")
        {
          m_queue->SetAttribute ("FengAdaptive", BooleanValue (true));
        }
      else if (m_currentAQM == "ns3::NonLinearRedQueueDisc")
        {
          m_queue->SetAttribute ("NLRED", BooleanValue (true));
        }
    }
  else
    {
      tch.SetRootQueueDisc (queueDisc.c_str ());
      m_queue = tch.Install (m_dumbbell.GetLeft ()->GetDevice (0)).Get (0);
    }

  if (queueDisc == "ns3::RedQueueDisc")
    {
      StaticCast<RedQueueDisc> (m_queue)->AssignStreams (0);
    }
  else if (queueDisc == "ns3::PieQueueDisc")
    {
      StaticCast<PieQueueDisc> (m_queue)->AssignStreams (0);
    }
  m_queue->TraceConnectWithoutContext ("Enqueue", MakeCallback (&EvaluationTopology::PacketEnqueue, this));
  m_queue->TraceConnectWithoutContext ("Dequeue", MakeCallback (&EvaluationTopology::PacketDequeue, this));
  m_queue->TraceConnectWithoutContext ("Drop", MakeCallback (&EvaluationTopology::PacketDrop, this));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  m_QDrecord = 0;
  m_numQDrecord = 0;
  m_lastQDrecord = Time::Min ();
  m_currentAQM.replace (m_currentAQM.begin (), m_currentAQM.begin () + 5, "/");
  AsciiTraceHelper asciiQD;
  std::string default_directory = "aqm-eval-output/";
  std::string data = "/data";
  m_QDfile = asciiQD.CreateFileStream (std::string (default_directory + ScenarioName + data + m_currentAQM + "-qdel.dat").c_str ());
  m_TPrecord = 0;
  m_lastTPrecord = Time::Min ();
  AsciiTraceHelper asciiTP;
  m_TPfile = asciiTP.CreateFileStream (std::string (default_directory + ScenarioName + data + m_currentAQM + "-throughput.dat").c_str ());
  AsciiTraceHelper asciiGP;
  m_GPfile = asciiGP.CreateFileStream (std::string (default_directory + ScenarioName + data + m_currentAQM + "-goodput.dat").c_str ());
  AsciiTraceHelper asciiMD;
  m_metaData = asciiMD.CreateFileStream (std::string (default_directory + ScenarioName + data + m_currentAQM + "-metadata.dat").c_str ());
  AsciiTraceHelper asciiDT;
  m_dropTime = asciiDT.CreateFileStream (std::string (default_directory + ScenarioName + data + m_currentAQM + "-drop.dat").c_str ());
  AsciiTraceHelper asciiET;
  m_enqueueTime = asciiET.CreateFileStream (std::string (default_directory + ScenarioName + data + m_currentAQM + "-enqueue.dat").c_str ());
}

EvaluationTopology::~EvaluationTopology (void)
{
}

void
EvaluationTopology::DestroyConnection ()
{
  m_queue->TraceDisconnectWithoutContext ("Enqueue", MakeCallback (&EvaluationTopology::PacketEnqueue, this));
  m_queue->TraceDisconnectWithoutContext ("Dequeue", MakeCallback (&EvaluationTopology::PacketDequeue, this));
  m_queue->TraceDisconnectWithoutContext ("Drop", MakeCallback (&EvaluationTopology::PacketDrop, this));

  for (uint32_t i = 0; i < m_sinks.size (); i++)
    {
      m_sinks[i]->TraceDisconnectWithoutContext ("Rx", MakeCallback (&EvaluationTopology::PayloadSize, this));
    }
  for (uint32_t i = 0; i < m_sources.size (); i++)
    {
      *m_metaData->GetStream () << "The flow completion time of flow " << (i + 1)
                                << " = "
                                << (m_sources[i]->GetFlowCompletionTime ()).GetSeconds ()
                                << "\n";
    }
}

void
EvaluationTopology::PacketEnqueue (Ptr<const QueueDiscItem> item)
{
  Ptr<Packet> p = item->GetPacket ();
  EvalTimestampTag tag;
  p->AddPacketTag (tag);
  Ptr<const Ipv4QueueDiscItem> iqdi = Ptr<const Ipv4QueueDiscItem> (dynamic_cast<const Ipv4QueueDiscItem *> (PeekPointer (item)));
  *m_enqueueTime->GetStream () << (iqdi->GetHeader ()).GetDestination ()
                               << " "
                               << Simulator::Now ().GetSeconds ()
                               << "\n";
}

void
EvaluationTopology::PacketDequeue (Ptr<const QueueDiscItem> item)
{
  Ptr<Packet> p = item->GetPacket ();
  EvalTimestampTag tag;
  p->RemovePacketTag (tag);
  Time delta = Simulator::Now () - tag.GetTxTime ();
  if (m_lastQDrecord == Time::Min () || Simulator::Now () - m_lastQDrecord > MilliSeconds (10))
    {
      m_lastQDrecord = Simulator::Now ();
      if (m_numQDrecord > 0)
        {
          *m_QDfile->GetStream () << Simulator::Now ().GetSeconds ()
                                  << " "
                                  << (m_QDrecord * 1.0) / (m_numQDrecord * 1.0)
                                  << "\n";
        }
      m_QDrecord = 0;
      m_numQDrecord = 0;
    }
  m_numQDrecord++;
  m_QDrecord += delta.GetMilliSeconds ();
}

void
EvaluationTopology::PacketDrop (Ptr<const QueueDiscItem> item)
{
  Ptr<const Ipv4QueueDiscItem> iqdi = Ptr<const Ipv4QueueDiscItem> (dynamic_cast<const Ipv4QueueDiscItem *> (PeekPointer (item)));
  *m_dropTime->GetStream () << (iqdi->GetHeader ()).GetDestination ()
                            << " "
                            << Simulator::Now ().GetSeconds ()
                            << "\n";
}

void
EvaluationTopology::PayloadSize (Ptr<const Packet> packet, const Address & address)
{
  *m_GPfile->GetStream () << address
                          << " "
                          << Simulator::Now ().GetSeconds ()
                          << " "
                          << packet->GetSize ()
                          <<  "\n";
  if (m_lastTPrecord == Time::Min () || Simulator::Now () - m_lastTPrecord > MilliSeconds (10))
    {
      if (m_TPrecord > 0)
        {
          *m_TPfile->GetStream () << Simulator::Now ().GetSeconds ()
                                  << " "
                                  << (m_TPrecord * 1.0) / (Simulator::Now () - m_lastTPrecord).GetSeconds ()
                                  << "\n";
        }
      m_lastTPrecord = Simulator::Now ();
      m_TPrecord = 0;
    }
  m_TPrecord += packet->GetSize ();
}

ApplicationContainer
EvaluationTopology::CreateFlow (StringValue senderDelay, StringValue receiverDelay,
                                StringValue senderBW, StringValue receiverBW,
                                std::string transport_prot, uint64_t maxPacket,
                                DataRate rate, uint32_t initCwnd)
{
  NS_ASSERT_MSG (m_flowsAdded < m_numFlows, "Trying to create more flows than permitted");
  m_flowsAdded++;

  char tempstr[20];

  sprintf (tempstr, "%d", m_flowsAdded);
  std::string sdelayAddress = std::string ("/ChannelList/") + tempstr + std::string ("/Delay");
  std::string rrBWAddress = std::string ("/NodeList/1/DeviceList/") + tempstr + std::string ("/DataRate");
  std::string srBWAddress = std::string ("/NodeList/0/DeviceList/") + tempstr + std::string ("/DataRate");

  sprintf (tempstr, "%d", m_flowsAdded + 1);
  std::string slBWAddress = std::string ("/NodeList/") + tempstr + std::string ("/DeviceList/0/DataRate");
  std::string socketTypeAddress = std::string ("/NodeList/") + tempstr + std::string ("/$ns3::TcpL4Protocol/SocketType");

  sprintf (tempstr, "%d", m_numFlows + m_flowsAdded);
  std::string rdelayAddress = std::string ("/ChannelList/") + tempstr + std::string ("/Delay");

  sprintf (tempstr, "%d", m_numFlows + m_flowsAdded + 1);
  std::string rlBWAddress = std::string ("/NodeList/") + tempstr + std::string ("/DeviceList/0/DataRate");

  Config::Set (sdelayAddress.c_str (), senderDelay);
  Config::Set (rdelayAddress.c_str (), receiverDelay);
  Config::Set (slBWAddress.c_str (), senderBW);
  Config::Set (srBWAddress.c_str (), senderBW);
  Config::Set (rlBWAddress.c_str (), receiverBW);
  Config::Set (rrBWAddress.c_str (), receiverBW);

  if (transport_prot == "udp")
    {
      uint32_t port = 50000;
      AddressValue remoteAddress (InetSocketAddress (m_dumbbell.GetRightIpv4Address (m_flowsAdded - 1), port));
      ApplicationContainer sourceAndSinkApp;
      PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (m_dumbbell.GetRightIpv4Address (m_flowsAdded - 1), port));
      sinkHelper.SetAttribute ("Protocol", TypeIdValue (UdpSocketFactory::GetTypeId ()));

      Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (m_dumbbell.GetLeft (m_flowsAdded - 1), UdpSocketFactory::GetTypeId ());

      Ptr<EvalApp> app = CreateObject<EvalApp> ();
      app->Setup (ns3UdpSocket, remoteAddress.Get (), m_packetSize, maxPacket * m_packetSize, m_flowsAdded * 2, rate);
      m_dumbbell.GetLeft (m_flowsAdded - 1)->AddApplication (app);

      sourceAndSinkApp.Add (app);
      sourceAndSinkApp.Add (sinkHelper.Install (m_dumbbell.GetRight (m_flowsAdded - 1)));
      Ptr<Application> appSink = sourceAndSinkApp.Get (1);
      Ptr<PacketSink> psink = Ptr<PacketSink> (dynamic_cast<PacketSink *> (PeekPointer (appSink)));
      psink->TraceConnectWithoutContext ("Rx", MakeCallback (&EvaluationTopology::PayloadSize, this));
      return sourceAndSinkApp;
    }
  else if (transport_prot.compare ("ns3::TcpWestwood") == 0)
    {
      // the default protocol type in ns3::TcpWestwood is WESTWOOD
      Config::Set (socketTypeAddress.c_str (), TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::Set ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else if (transport_prot.compare ("ns3::TcpWestwoodPlus") == 0)
    {
      Config::Set (socketTypeAddress.c_str (), TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::Set ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
      Config::Set ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else
    {
      Config::Set (socketTypeAddress.c_str (), TypeIdValue (TypeId::LookupByName (transport_prot)));
    }

  uint32_t port = 50000;
  AddressValue remoteAddress (InetSocketAddress (m_dumbbell.GetRightIpv4Address (m_flowsAdded - 1), port));
  ApplicationContainer sourceAndSinkApp;
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (m_dumbbell.GetRightIpv4Address (m_flowsAdded - 1), port));
  sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (m_dumbbell.GetLeft (m_flowsAdded - 1), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket->SetAttribute ("InitialCwnd", UintegerValue (initCwnd));
  ns3TcpSocket->SetAttribute ("SegmentSize", UintegerValue (m_packetSize));

  if (transport_prot.compare ("ns3::TcpNewReno") == 0)
    {
      ns3TcpSocket->SetAttribute ("Sack", BooleanValue (true));
    }
  else
    {
      ns3TcpSocket->SetAttribute ("Sack", BooleanValue (false));
    }

  Ptr<EvalApp> app = CreateObject<EvalApp> ();
  app->Setup (ns3TcpSocket, remoteAddress.Get (), m_packetSize, maxPacket * m_packetSize, m_flowsAdded * 2, rate);
  m_dumbbell.GetLeft (m_flowsAdded - 1)->AddApplication (app);

  sourceAndSinkApp.Add (app);
  sourceAndSinkApp.Add (sinkHelper.Install (m_dumbbell.GetRight (m_flowsAdded - 1)));
  Ptr<Application> appSink = sourceAndSinkApp.Get (1);
  Ptr<PacketSink> psink = Ptr<PacketSink> (dynamic_cast<PacketSink *> (PeekPointer (appSink)));
  psink->TraceConnectWithoutContext ("Rx", MakeCallback (&EvaluationTopology::PayloadSize, this));
  m_sinks.push_back (psink);
  m_sources.push_back (app);
  return sourceAndSinkApp;
}

} //namespace ns3
