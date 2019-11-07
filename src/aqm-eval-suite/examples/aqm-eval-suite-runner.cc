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

#include <stdlib.h>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "ns3/core-module.h"
#include <sys/stat.h>

using namespace ns3;

std::vector<std::string> AQM = {
"PfifoFast",
"CoDel",
"Pie",
"Red",
"AdaptiveRed",
"FengAdaptiveRed",
"NonLinearRed",
"Cobalt"
};
std::string queueDisc = "QueueDisc";
uint32_t nAQM = 8;
std::string AggressiveTcp = "";
std::string QueueDiscMode = "QUEUE_DISC_MODE_PACKETS";
std::string isBql = "false"; 

void RemoveAqm (std::string aqm)
{
  AQM.erase (std::remove (AQM.begin (), AQM.end (), aqm), AQM.end ());
  nAQM--;	
}

void RunOneScenario (std::string scenarioName)
{
  mkdir ((std::string ("aqm-eval-output/") + scenarioName).c_str (), 0700);
  mkdir ((std::string ("aqm-eval-output/") + scenarioName + (std::string ("/data"))).c_str (), 0700);
  mkdir ((std::string ("aqm-eval-output/") + scenarioName + (std::string ("/graph"))).c_str (), 0700);
  std::string commandToRun;
  if (AggressiveTcp != "" && scenarioName == "AggressiveTransportSender")
    {
      commandToRun = std::string ("./waf --run \"") + scenarioName + std::string (" --TcpVariant=ns3::") + AggressiveTcp + std::string (" --QueueDiscMode=") + QueueDiscMode + std::string (" --isBql=") + isBql + std::string ("\"");
    }
  else
    {
      commandToRun = std::string ("./waf --run \"") + scenarioName + std::string (" --QueueDiscMode=") + QueueDiscMode + std::string (" --isBql=") + isBql + std::string ("\"");
    }
  system (commandToRun.c_str ());
  std::ofstream outfile;
  outfile.open ((std::string ("aqm-eval-output/") + scenarioName + std::string ("/data/plot-shell")).c_str (), std::ios::out | std::ios::trunc);
  outfile << "set terminal png size 600, 350\n";
  outfile << "set size .9, 1\n";
  outfile << "set output \"aqm-eval-output/" << scenarioName.c_str () << "/graph/qdel-goodput.png\"\n set xlabel \"Queue Delay (ms)\" font \"Verdana\"\nset ylabel \"Goodput (Mbps)\" font \"Verdana\"\n";
  outfile << "set xrange[] reverse\nset grid\nshow grid\n";
  outfile.close ();

  std::string gnuPlot = "plot ";

  for (uint32_t i = 0; i < nAQM; i++)
    {
      std::string proQdelThr = std::string ("python src/aqm-eval-suite/utils/generate-ellipseinput.py ") + scenarioName + " " + AQM[i] + queueDisc;
      std::string proEllipse = std::string ("python src/aqm-eval-suite/utils/ellipsemaker ") + scenarioName + " " + AQM[i] + queueDisc + " " + std::to_string (i + 1);
      std::string plotGoodput = std::string ("python src/aqm-eval-suite/utils/goodput_process.py ") + scenarioName + " " + AQM[i] + queueDisc;
      std::string plotDelay = std::string ("python src/aqm-eval-suite/utils/delay_process.py ") + scenarioName + " " + AQM[i] + queueDisc;
      system (proQdelThr.c_str ());
      system (proEllipse.c_str ());
      system (plotGoodput.c_str ());
      system (plotDelay.c_str ());
      std::string graphName = std::string ("\"aqm-eval-output/") + scenarioName + std::string ("/data/") + AQM[i] + queueDisc + std::string ("-ellipse.dat\" notitle with lines");
      if (i != nAQM - 1)
        {
          gnuPlot = gnuPlot + graphName + std::string (",");
        }
      else
        {
          gnuPlot = gnuPlot + graphName;
        }
    }
  outfile.open ((std::string ("aqm-eval-output/") + scenarioName + std::string ("/data/plot-shell")).c_str (), std::ios::out | std::ios::app);
  outfile << gnuPlot.c_str ();
  outfile.close ();

  system ((std::string ("gnuplot aqm-eval-output/") + scenarioName + std::string ("/data/plot-shell")).c_str ());
  system ((std::string ("convert aqm-eval-output/") + scenarioName + std::string ("/graph/qdel-goodput.png aqm-eval-output/") + scenarioName + std::string ("/graph/qdel-goodput.eps")).c_str ());
}

void RunRttFairness (std::string scenarioName)
{
  std::string orig = "RttFairness";
  for (uint32_t i = 1; i <= 15; i++)
    {
      char sce[20];
      sprintf (sce, "%d", i);
      scenarioName = orig + std::string (sce);
      mkdir ((std::string ("aqm-eval-output/") + scenarioName).c_str (), 0700);
      mkdir ((std::string ("aqm-eval-output/") + scenarioName + std::string ("/data")).c_str (), 0700);
      mkdir ((std::string ("aqm-eval-output/") + scenarioName + std::string ("/graph")).c_str (), 0700);
    }
  std::string commandToRun = std::string ("./waf --run \"RttFairness") + std::string (" --QueueDiscMode=") + QueueDiscMode + std::string (" --isBql=") + isBql + std::string ("\"");
  system (commandToRun.c_str ());
  for (uint32_t i = 1; i <= 15; i++)
    {
      char sce[20];
      sprintf (sce, "%d", i);
      scenarioName = orig + std::string (sce);
      std::ofstream outfile;
      outfile.open ((std::string ("aqm-eval-output/") + scenarioName + std::string ("/data/plot-shell")).c_str (), std::ios::out | std::ios::trunc);
      outfile << "set terminal png size 600, 350\n";
      outfile << "set size .9, 1\n";
      outfile << "set output \"aqm-eval-output/" << scenarioName.c_str () << "/graph/qdel-goodput.png\"\n set xlabel \"Queue Delay (ms)\" font \"Verdana\"\nset ylabel \"Goodput (Mbps)\" font \"Verdana\"\n";
      outfile << "set xrange[] reverse\nset grid\nshow grid\n";
      outfile.close ();

      std::string gnuPlot = "plot ";

      for (uint32_t i = 0; i < nAQM; i++)
        {
          std::string proQdelThr = std::string ("python src/aqm-eval-suite/utils/generate-ellipseinput.py ") + scenarioName + " " + AQM[i] + queueDisc;
          std::string proEllipse = std::string ("python src/aqm-eval-suite/utils/ellipsemaker ") + scenarioName + " " + AQM[i] + queueDisc + " " + std::to_string (i + 1);
          std::string plotGoodput = std::string ("python src/aqm-eval-suite/utils/goodput_process.py ") + scenarioName + " " + AQM[i] + queueDisc;
          std::string plotDelay = std::string ("python src/aqm-eval-suite/utils/delay_process.py ") + scenarioName + " " + AQM[i] + queueDisc;
          std::string plotDrop = std::string ("python src/aqm-eval-suite/utils/drop_process.py ") + scenarioName + " " + AQM[i] + queueDisc;
          system (proQdelThr.c_str ());
          system (proEllipse.c_str ());
          system (plotGoodput.c_str ());
          system (plotDelay.c_str ());
          system (plotDrop.c_str ());
          std::string graphName = std::string ("\"aqm-eval-output/") + scenarioName + std::string ("/data/") + AQM[i] + queueDisc + std::string ("-ellipse.dat\" notitle with lines");
          if (i != nAQM - 1)
            {
              gnuPlot = gnuPlot + graphName + std::string (",");
            }
          else
            {
              gnuPlot = gnuPlot + graphName;
            }
        }

      outfile.open ((std::string ("aqm-eval-output/") + scenarioName + std::string ("/data/plot-shell")).c_str (), std::ios::out | std::ios::app);
      outfile << gnuPlot.c_str ();
      outfile.close ();

      system ((std::string ("gnuplot aqm-eval-output/") + scenarioName + std::string ("/data/plot-shell")).c_str ());
      system ((std::string ("convert aqm-eval-output/") + scenarioName + std::string ("/graph/qdel-goodput.png aqm-eval-output/") + scenarioName + std::string ("/graph/qdel-goodput.eps")).c_str ());
    }
}

int main (int argc, char *argv[])
{
  mkdir ("aqm-eval-output", 0700);
  std::map<std::string, std::string> ScenarioNumberMapping;
  ScenarioNumberMapping["5.1.1"] = "TCPFriendlySameInitCwnd";
  ScenarioNumberMapping["5.1.2"] = "TCPFriendlyDifferentInitCwnd";
  ScenarioNumberMapping["5.2"] = "AggressiveTransportSender";
  ScenarioNumberMapping["5.3.1"] = "UnresponsiveTransport";
  ScenarioNumberMapping["5.3.2"] = "UnresponsiveWithFriendly";
  ScenarioNumberMapping["5.4"] = "LbeTransportSender";
  ScenarioNumberMapping["8.2.2"] = "MildCongestion";
  ScenarioNumberMapping["8.2.3"] = "MediumCongestion";
  ScenarioNumberMapping["8.2.4"] = "HeavyCongestion";
  ScenarioNumberMapping["8.2.5"] = "VaryingCongestion";
  ScenarioNumberMapping["8.2.6.1"] = "VaryingBandwidthUno";
  ScenarioNumberMapping["8.2.6.2"] = "VaryingBandwidthDuo";
  ScenarioNumberMapping["6"] = "RttFairness";

  std::string scenarioName = "";
  std::string scenarioNumber = "";

  CommandLine cmd;
  cmd.AddValue ("number", "Scenario number from RFC", scenarioNumber);
  cmd.AddValue ("name", "Name of the scenario (eg: TCPFriendlySameInitCwnd)", scenarioName);
  cmd.AddValue ("AggressiveTcp", "Variant of the Aggressive TCP", AggressiveTcp);
  cmd.AddValue ("QueueDiscMode", "Determines the unit for QueueLimit", QueueDiscMode);
  cmd.AddValue ("isBql", "Enables/Disables Byte Queue Limits", isBql);

  cmd.Parse (argc, argv);

  if (scenarioName == "" && scenarioNumber == "")
    {
      std::cout << "No value given";
      exit (-1);
    }

  if (scenarioName == "")
    {
      scenarioName = ScenarioNumberMapping[scenarioNumber];
    }

  if (scenarioName != "All" && scenarioName != "RttFairness")
    {
      RunOneScenario (scenarioName);
    }
  else if (scenarioName != "All" && scenarioName == "RttFairness")
    {
      RunRttFairness (scenarioName);
    }
  else
    {
      RunRttFairness (scenarioName);
      for (std::map<std::string, std::string>::iterator it = ScenarioNumberMapping.begin (); it != ScenarioNumberMapping.end (); ++it)
        {
          if (it->second != "RttFairness")
            {
              RunOneScenario (it->second);
            }
        }
    }
}
