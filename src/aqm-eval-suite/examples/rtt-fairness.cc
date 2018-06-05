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
 * This example is equivalent to the scenario described in Section 6.2
 * of RFC 7928 (https://tools.ietf.org/html/rfc7928#section-6.2).
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/aqm-eval-suite-module.h"
#include "vector"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RttFairness");

class RttFairness : public ScenarioImpl
{
public:
  RttFairness (uint32_t run);
  ~RttFairness ();

protected:
  virtual EvaluationTopology CreateScenario (std::string aqm, bool isBql);

private:
  uint32_t m_run;
  std::vector<uint32_t> delay;
};

RttFairness::RttFairness (uint32_t run)
{
  m_run = run;
  delay = {5, 8, 10, 16, 20, 30, 40, 60, 80, 100, 150, 200, 300, 400, 500};
}

RttFairness::~RttFairness ()
{
}

EvaluationTopology
RttFairness::CreateScenario (std::string aqm, bool isBql)
{
  double bottleneck;
  double reqDelayConstRtt;
  double reqDelay;
  char OWD[20];
  char scenario[20];
  char OWDConst[20];
  char bottleneckStr[20];
  if (delay[m_run] < 100)
    {
      bottleneck = delay[m_run] * 0.8 / 2;
      reqDelay = delay[m_run] * 0.2 / 4;
      reqDelayConstRtt = (50 - bottleneck) / 2;
    }
  else
    {
      bottleneck = 40;
      reqDelay = (delay[m_run] - 80) / 4;
      reqDelayConstRtt = 5;
    }

  sprintf (OWD, "%fms", reqDelay);
  sprintf (OWDConst, "%fms", reqDelayConstRtt);
  sprintf (scenario, "%d", m_run + 1);
  sprintf (bottleneckStr, "%fms", bottleneck);
  std::string scenarioName = std::string ("RttFairness") + std::string (scenario);
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute  ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (bottleneckStr));
  uint32_t nflow = 2;

  EvaluationTopology et (scenarioName, nflow, pointToPoint, aqm, 698, isBql);
  ApplicationContainer ac1 = et.CreateFlow (StringValue (OWDConst),
                                            StringValue (OWDConst),
                                            StringValue ("10Mbps"),
                                            StringValue ("10Mbps"),
                                            "ns3::TcpNewReno", 0, DataRate ("10Mb/s"), 3);

  ApplicationContainer ac2 = et.CreateFlow (StringValue (OWD),
                                            StringValue (OWD),
                                            StringValue ("10Mbps"),
                                            StringValue ("10Mbps"),
                                            "ns3::TcpNewReno", 0, DataRate ("10Mb/s"), 3);

  ac1.Start (Seconds (0));
  ac1.Stop (Seconds (600));

  ac2.Start (Seconds (0.5));
  ac2.Stop (Seconds (600.5));
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

  for (uint32_t i = 0; i < 15; i++)
    {
      RttFairness rf (i);
      rf.ConfigureQueueDisc (45, 750, "1Mbps", "2ms", QueueDiscMode);
      rf.RunSimulation (Seconds (610), isBql == "true");
    }
}
