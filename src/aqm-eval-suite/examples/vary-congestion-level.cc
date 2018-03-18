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
 * This example is equivalent to the scenario described in Section 8.2.5
 * of RFC 7928 (https://tools.ietf.org/html/rfc7928#section-8.2.5).
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/aqm-eval-suite-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("VaryCongestionLevel");

class VaryingCongestion : public ScenarioImpl
{
public:
  VaryingCongestion ();
  ~VaryingCongestion ();

protected:
  virtual EvaluationTopology CreateScenario (std::string aqm, bool isBql);

private:
  void PauseApp (EvaluationTopology et, uint32_t flow)
  {
    et.Pause (flow);
  }

  void restartApp (EvaluationTopology et, uint32_t flow)
  {
    et.Restart (flow);
  }
};

VaryingCongestion::VaryingCongestion ()
{
}

VaryingCongestion::~VaryingCongestion ()
{
}

EvaluationTopology
VaryingCongestion::CreateScenario (std::string aqm, bool isBql)
{
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute  ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("48ms"));
  uint32_t nflow = 0.114 * 62;

  EvaluationTopology et ("VaryingCongestion", nflow, pointToPoint, aqm, 698, isBql);
  for (uint32_t i = 0; i < nflow; i++)
    {
      ApplicationContainer ac = et.CreateFlow (StringValue ("1ms"),
                                               StringValue ("1ms"),
                                               StringValue ("10Mbps"),
                                               StringValue ("10Mbps"),
                                               "ns3::TcpNewReno", 0, DataRate ("10Mb/s"), 3);

      ac.Start (Seconds (0));
      ac.Stop (Seconds (300));
      if (i > 0.036 * 62 && i < 0.081 * 62)
        {
          Simulator::Schedule (Seconds (40), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (60), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (100), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (120), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (160), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (180), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (220), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (240), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (280), &VaryingCongestion::PauseApp, this, et, i);
        }
      else if (i > 0.036 * 62 && i < 0.114 * 62)
        {
          Simulator::Schedule (Seconds (20), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (60), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (80), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (120), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (140), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (180), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (200), &VaryingCongestion::PauseApp, this, et, i);
          Simulator::Schedule (Seconds (240), &VaryingCongestion::restartApp, this, et, i);
          Simulator::Schedule (Seconds (260), &VaryingCongestion::PauseApp, this, et, i);
        }
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

  VaryingCongestion sce;
  sce.ConfigureQueueDisc (45, 750, "1Mbps", "48ms", QueueDiscMode);
  sce.RunSimulation (Seconds (310), isBql == "true");
}
