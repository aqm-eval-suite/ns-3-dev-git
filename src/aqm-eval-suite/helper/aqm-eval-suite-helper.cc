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

#include "aqm-eval-suite-helper.h"

namespace ns3 {

ScenarioImpl::ScenarioImpl ()
{
  m_AQM = {
    "ns3::PfifoFastQueueDisc",
    "ns3::RedQueueDisc",
    "ns3::AdaptiveRedQueueDisc",
    "ns3::CoDelQueueDisc",
    "ns3::PieQueueDisc"
  };
  m_nAQM = 5;
}

ScenarioImpl::~ScenarioImpl ()
{
}

void
ScenarioImpl::addAQM (std::string aqm)
{
  m_AQM.push_back (aqm);
  m_nAQM++;
}

void
ScenarioImpl::DestroyTrace (EvaluationTopology et)
{
  et.DestroyConnection ();
}

void
ScenarioImpl::ConfigureQueueDisc (uint32_t limit, uint32_t pktsize, std::string linkbw, std::string linkdel)
{
  Config::SetDefault ("ns3::PfifoFastQueueDisc::Limit", UintegerValue (limit));

  Config::SetDefault ("ns3::RedQueueDisc::QueueLimit", UintegerValue (limit));
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (pktsize));
  Config::SetDefault ("ns3::RedQueueDisc::LinkBandwidth", StringValue (linkbw));
  Config::SetDefault ("ns3::RedQueueDisc::LinkDelay", StringValue (linkdel));

  Config::SetDefault ("ns3::CoDelQueueDisc::MaxPackets", UintegerValue (limit));
  Config::SetDefault ("ns3::CoDelQueueDisc::MaxBytes", UintegerValue (limit * pktsize));
  Config::SetDefault ("ns3::CoDelQueueDisc::MinBytes", UintegerValue (pktsize));

  Config::SetDefault ("ns3::PieQueueDisc::QueueLimit", UintegerValue (limit));
  Config::SetDefault ("ns3::PieQueueDisc::MeanPktSize", UintegerValue (pktsize));
}

void
ScenarioImpl::RunSimulation (Time simtime)
{
  for (uint32_t i = 0; i < m_nAQM; i++)
    {
      EvaluationTopology et = CreateScenario (m_AQM[i]);
      Simulator::Schedule (simtime, &ScenarioImpl::DestroyTrace, this, et);
      Simulator::Stop (simtime);
      Simulator::Run ();
      Simulator::Destroy ();
    }
}

} //namespace ns3
