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

/*
 * This example is equivalent to the scenario described in Experiment 1 of
 * Section 8.2.6 of RFC 7928 (https://tools.ietf.org/html/rfc7928#section-8.2.6).
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/aqm-eval-suite-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("VaryAvailableCapacityDuo");

class VaryingBandwidthDuo : public ScenarioImpl
{
public:
  VaryingBandwidthDuo ();
  ~VaryingBandwidthDuo ();

protected:
  virtual EvaluationTopology CreateScenario (std::string aqm, bool isBql);

private:
  void ChangeDataRate ();
  bool m_state;
};

VaryingBandwidthDuo::VaryingBandwidthDuo ()
{
  m_state = true;
}

VaryingBandwidthDuo::~VaryingBandwidthDuo ()
{
}

void
VaryingBandwidthDuo::ChangeDataRate ()
{
  if (m_state)
    {
      Config::Set ("/NodeList/0/DeviceList/0/DataRate", StringValue ("10Mbps"));
      Config::Set ("/NodeList/1/DeviceList/0/DataRate", StringValue ("10Mbps"));
      m_state = false;
    }
  else
    {
      Config::Set ("/NodeList/0/DeviceList/0/DataRate", StringValue ("100Mbps"));
      Config::Set ("/NodeList/1/DeviceList/0/DataRate", StringValue ("100Mbps"));
      m_state = true;
    }
}

EvaluationTopology
VaryingBandwidthDuo::CreateScenario (std::string aqm, bool isBql)
{
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute  ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("48ms"));
  uint32_t nflow = 2;

  EvaluationTopology et ("VaryingBandwidthDuo", nflow, pointToPoint, aqm, 698, isBql);
  ApplicationContainer ac1 = et.CreateFlow (StringValue ("1ms"),
                                            StringValue ("1ms"),
                                            StringValue ("500Mbps"),
                                            StringValue ("500Mbps"),
                                            "ns3::TcpNewReno", 0, DataRate ("500Mb/s"), 3);


  ApplicationContainer ac2 = et.CreateFlow (StringValue ("1ms"),
                                            StringValue ("1ms"),
                                            StringValue ("500Mbps"),
                                            StringValue ("500Mbps"),
                                            "ns3::TcpNewReno", 0, DataRate ("500Mb/s"), 3);

  ac1.Start (Seconds (0));
  ac1.Stop (Seconds (300));

  ac2.Start (Seconds (0));
  ac2.Stop (Seconds (300));

  for (double i = 0.1; i < 300; i += 0.1)
    {
      Simulator::Schedule (Seconds (i), &VaryingBandwidthDuo::ChangeDataRate, this);
    }

  return et;
}

int
main (int argc, char *argv[])
{
  std::string QueueDiscMode = "";
  std::string isBql = "";
  CommandLine cmd;
  cmd.AddValue ("QueueDiscMode", "Determines the unit for QueueLimit", QueueDiscMode);
  cmd.AddValue ("isBql", "Enables/Disables Byte Queue Limits", isBql);
  cmd.Parse (argc, argv);

  VaryingBandwidthDuo sce;
  sce.ConfigureQueueDisc (500, 750, "100Mbps", "48ms", QueueDiscMode);
  sce.RunSimulation (Seconds (310), isBql == "true");
}
