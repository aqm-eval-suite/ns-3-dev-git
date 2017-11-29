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

#ifndef EVAL_TOPOLOGY_H
#define EVAL_TOPOLOGY_H

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <vector>
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/assert.h"
#include "ns3/data-rate.h"
#include "eval-app.h"

namespace ns3 {

/**
 * \brief Configures the topology and traffic in the suite
 *
 * This class sets up a point-to-point dumbbell topology, installs the
 * applications on nodes and fetches the performance metrics of interest
 */
class EvaluationTopology
{

public:
  /**
   * \brief Get the type ID
   *
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   *
   * \param ScenarioName The name of the scenario
   * \param numFlows The number of flows
   * \param p2pHelper The configured point-to-point channel
   * \param queueDisc TypeID of the AQM algorithm
   * \param pktSize The size of payload
   */
  EvaluationTopology (std::string ScenarioName,
                      uint32_t numFlows,
                      PointToPointHelper p2pHelper,
                      std::string queueDisc,
                      uint32_t pktSize = 512,
                      bool isBql = false);

  /**
   * \brief Destructor
   */
  ~EvaluationTopology ();

  /**
   * \brief Creates a single flow
   *
   * \param senderDelay Link delay from sender to left router
   * \param receiverDelay Link delay from receiver to right router
   * \param senderBW Link bandwidth from sender to left router
   * \param receiverBW Link bandwidth from receiver to right router
   * \param transport_prot The transport protocol
   * \param maxPacket The maximum number of packets
   * \param rate The rate of sending data
   * \param initCwnd The initial congestion window for TCP
   */
  ApplicationContainer CreateFlow (StringValue senderDelay,
                                   StringValue receiverDelay,
                                   StringValue senderBW,
                                   StringValue receiverBW,
                                   std::string transport_prot,
                                   uint64_t maxPacket,
                                   DataRate rate,
                                   uint32_t initCwnd = 3);

  /**
   * \brief Disconnects the trace sources
   */
  void DestroyConnection ();

  /**
   * \brief Stops the flow temporarily
   *
   * \param flow The flow id
   */
  void Pause (uint32_t flow)
    {
      m_sources[flow]->CancelEvents ();
    }

  /**
   * \brief Restarts the paused flow
   *
   * \param flow The flow id
   */
  void Restart (uint32_t flow)
    {
      m_sources[flow]->ScheduleStartEvent ();
    }

private:
  /**
   * \brief Adds a timestamp to the enqueued queue item
   *
   * \param item The queue item enqueued
   */
   void PacketEnqueue (Ptr<const QueueDiscItem> item);

  /**
   * \brief Removes a timestamp from the queue item being dequeued
   *
   * \param item The queue item being dequeued
   */
   void PacketDequeue (Ptr<const QueueDiscItem> item);

  /**
   * \brief Logs the drop time of the queue item
   *
   * \param item The queue item being dropped
   */
   void PacketDrop (Ptr<const QueueDiscItem> item);

  /**
   * \brief Calculates goodput and logs the size of data received at application
   *
   * \param packet The packet received at application
   * \param address The address of sender
   */
   void PayloadSize (Ptr<const Packet> packet, const Address & address);

  std::string m_currentAQM;                       //!< AQM being currently simulated
  uint32_t m_numFlows;                            //!< The total number of flows
  uint32_t m_flowsAdded;                          //!< The number of flows added till now in the simulation
  PointToPointDumbbellHelper m_dumbbell;          //!< Point to point dumbbell helper object
  Ptr<QueueDisc> m_queue;                         //!< The queue discipline installed on the bottleneck link
  uint32_t m_packetSize;                          //!< The size of payload
  uint64_t m_QDrecord;                            //!< The sum of queue delays observed in 10ms
  uint64_t m_numQDrecord;                         //!< The number of samples obtained in 10ms
  Time m_lastQDrecord;                            //!< Last time the average queue delay was calculated
  Ptr<OutputStreamWrapper> m_QDfile;              //!< File to store queue delay values
  uint64_t m_TPrecord;                            //!< The total amount of data received in 10ms
  Time m_lastTPrecord;                            //!< Last time the average goodput was calculated
  Ptr<OutputStreamWrapper> m_TPfile;              //!< File to store goodput values
  std::vector < Ptr < PacketSink >> m_sinks;      //!< List of packet sinks
  Ptr<OutputStreamWrapper> m_GPfile;              //!< File to store per flow statistics
  std::vector < Ptr < EvalApp >> m_sources;       //!< List of application sources
  Ptr<OutputStreamWrapper> m_metaData;            //!< File to store flow completion times
  Ptr<OutputStreamWrapper> m_dropTime;            //!< File to store packet drop times
  Ptr<OutputStreamWrapper> m_enqueueTime;         //!< File to store packet enqueue times
};

}

#endif /* EVAL_TOPOLOGY_H */
