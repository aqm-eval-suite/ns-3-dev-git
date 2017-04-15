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

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "eval-app.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvalApp");

NS_OBJECT_ENSURE_REGISTERED (EvalApp);

EvalApp::EvalApp ()
  : m_socket (0),
    m_peer (),
    m_connected (false),
    m_residualBits (0),
    m_lastStartTime (Seconds (0)),
    m_maxBytes (0),
    m_totBytes (0)
{
  m_onTime = CreateObject<ConstantRandomVariable> ();
  m_onTime->SetAttribute ("Constant", DoubleValue (1));
  m_offTime = CreateObject<ConstantRandomVariable> ();
  m_offTime->SetAttribute ("Constant", DoubleValue (0));
  m_flowStop = Time::Min ();
  m_flowStart = Time::Min ();
}

EvalApp::~EvalApp ()
{
  m_socket = 0;
}

void
EvalApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint64_t maxByte, uint64_t stream, DataRate rate)
{
  m_socket = socket;
  m_peer = address;
  m_pktSize = packetSize;
  m_maxBytes = maxByte;
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  m_cbrRate = rate;
}

void
EvalApp::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Called at time specified by Start
void EvalApp::StartApplication ()
{
  NS_LOG_FUNCTION (this);

  m_socket->Bind ();
  m_socket->Connect (m_peer);
  m_socket->SetAllowBroadcast (true);
  m_socket->ShutdownRecv ();

  if (m_flowStart == Time::Min ())
    {
      m_flowStart = Simulator::Now (); 
    }

  m_socket->SetConnectCallback (
    MakeCallback (&EvalApp::ConnectionSucceeded, this),
    MakeCallback (&EvalApp::ConnectionFailed, this));

  m_cbrRateFailSafe = m_cbrRate;

  // Ensure no pending event
  CancelEvents ();

  ScheduleStartEvent ();
}

// Called at time specified by Stop
void EvalApp::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if (m_socket != 0)
    {
      if (m_flowStop == Time::Min ())
        {
          m_flowStop = Simulator::Now ();
        }
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("EvalApp found null socket to close in StopApplication");
    }
}

Time EvalApp::GetFlowCompletionTime ()
{
  return m_flowStop - m_flowStart;
}

void EvalApp::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning () && m_cbrRateFailSafe == m_cbrRate )
    {
      // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * m_cbrRate.GetBitRate ();
      m_residualBits += bits.GetHigh ();
    }
  m_cbrRateFailSafe = m_cbrRate;
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
}

void EvalApp::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();

  // Schedule the send packet event
  ScheduleNextTx ();
  ScheduleStopEvent ();
}

void EvalApp::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

void EvalApp::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      NS_LOG_LOGIC ("bits = " << bits);
      Time nextTime (Seconds (bits /
                              static_cast<double> (m_cbrRate.GetBitRate ())));
      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime,
                                         &EvalApp::SendPacket, this);
    }
  else
    {
      // All done, cancel any pending events
      StopApplication ();
    }
}

void EvalApp::ScheduleStartEvent ()
{
  NS_LOG_FUNCTION (this);

  // Schedules the event to start sending data (switch to the "On" state)
  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &EvalApp::StartSending, this);
}

void EvalApp::ScheduleStopEvent ()
{
  NS_LOG_FUNCTION (this);

  // Schedules the event to stop sending data (switch to "Off" state)
  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &EvalApp::StopSending, this);
}

void EvalApp::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet = Create<Packet> (m_pktSize);
  m_socket->Send (packet);
  m_totBytes += m_pktSize;
  m_lastStartTime = Simulator::Now ();
  m_residualBits = 0;
  ScheduleNextTx ();
}

void EvalApp::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void EvalApp::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

} //namespace ns3
