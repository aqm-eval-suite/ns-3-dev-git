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

#include "eval-ts.h"

namespace ns3 {

EvalTimestampTag::EvalTimestampTag ()
  : m_creationTime (Simulator::Now ().GetTimeStep ())
{
}

TypeId
EvalTimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EvalTimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<EvalTimestampTag> ()
    .AddAttribute ("CreationTime",
                   "The time at which the timestamp was created",
                   StringValue ("0.0s"),
                   MakeTimeAccessor (&EvalTimestampTag::GetTxTime),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId
EvalTimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
EvalTimestampTag::GetSerializedSize (void) const
{
  return 8;
}

void
EvalTimestampTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_creationTime);
}

void
EvalTimestampTag::Deserialize (TagBuffer i)
{
  m_creationTime = i.ReadU64 ();
}

void
EvalTimestampTag::Print (std::ostream &os) const
{
  os << "CreationTime=" << m_creationTime;
}

Time
EvalTimestampTag::GetTxTime (void) const
{
  return TimeStep (m_creationTime);
}

} //namespace ns3
