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

#ifndef AQM_EVAL_SUITE_HELPER_H
#define AQM_EVAL_SUITE_HELPER_H

#include "ns3/eval-topology.h"

namespace ns3 {

/**
 * \brief An abstract class for scenario implementation
 *
 * This class contains the definition of methods commonly used in all
 * scenarios, and virtual declaration of scenario-specific methods.
 */
class ScenarioImpl
{
public:
  /**
   * \brief Constructor
   */
  ScenarioImpl ();

  /**
   * \brief Destructor
   */
  virtual ~ScenarioImpl ();

  /**
   * \brief Run simulation for the specified duration
   *
   * \param simtime The simulation time
   * \param isBql Enable/Disable Byte Queue Limits
   */
  void RunSimulation (Time simtime, bool isBql);

  /**
   * \brief Adds the AQM to m_AQM list
   *
   * \param aqm TypeID of the AQM algorithm
   */
  void addAQM (std::string aqm);

  /**
   * \brief Helper to disconnect trace sources
   *
   * \param et The EvaluationTopology for which trace sources are disconnected
   */
  void DestroyTrace (EvaluationTopology et);

  /**
   * \brief Configure the queue discipline
   *
   * \param limit Maximum capacity of the queue discipline
   * \param pktsize Packet size
   * \param linkbw Bottleneck link bandwidth
   * \param linkdel Bottleneck link delay
   * \param mode Mode of operation for QueueDisc
   */
  virtual void ConfigureQueueDisc (uint32_t limit, uint32_t pktsize, std::string linkbw, std::string linkdel, std::string mode);

protected:
  /**
   * \brief Create simulation scenario
   *
   * \param aqm TypeID of the AQM algorithm
   */
  virtual EvaluationTopology CreateScenario (std::string aqm, bool isBql) = 0;

private:
  std::vector<std::string> m_AQM;               //!< List of AQM algorithms
  uint32_t m_nAQM;                              //!< Number of AQM algorithms
};

}

#endif /* AQM_EVAL_SUITE_HELPER_H */
