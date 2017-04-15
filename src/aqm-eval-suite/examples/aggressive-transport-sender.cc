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
 * This example is equivalent to the scenario described in Section 5.2
 * of RFC 7928 (https://tools.ietf.org/html/rfc7928#section-5.2).
 *
 * Note: BIC TCP is used as an aggressive sender due to unavailability
 * of CUBIC TCP module in ns-3.
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/aqm-eval-suite-module.h"

using namespace ns3;

class AggressiveTransportSender : public ScenarioImpl
{
public:
  AggressiveTransportSender ();
  ~AggressiveTransportSender ();

protected:
  virtual EvaluationTopology CreateScenario (std::string aqm);
};

AggressiveTransportSender::AggressiveTransportSender ()
{
}

AggressiveTransportSender::~AggressiveTransportSender ()
{
}

EvaluationTopology
AggressiveTransportSender::CreateScenario (std::string aqm)
{
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute  ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("48ms"));
  uint32_t nflow = 1;

  EvaluationTopology et ("AggressiveTransportSender", nflow, pointToPoint, aqm, 1460);
  ApplicationContainer ac = et.CreateFlow (StringValue ("1ms"),
                                           StringValue ("1ms"),
                                           StringValue ("10Mbps"),
                                           StringValue ("10Mbps"),
                                           "ns3::TcpBic", 0, DataRate ("10Mb/s"), 3);

  ac.Start (Seconds (0));
  ac.Stop (Seconds (300));
  return et;
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  AggressiveTransportSender sce;
  sce.ConfigureQueueDisc (45, 1460, "1Mbps", "48ms");
  sce.RunSimulation (Seconds (310));
}
