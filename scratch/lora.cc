/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * This is an example script for lora protocol. 
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 *          To Thanh Hai <tthhai@gmail.com>
 *
 */

#include "ns3/lora-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"

#include "ns3/lora-net-device.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-phy-gen.h"
#include "ns3/lora-transducer-hd.h"
#include "ns3/lora-prop-model-ideal.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/pointer.h"
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/mac-lora-gw.h"
#include "ns3/header.h"

#include <iostream>
#include <cmath>

using namespace ns3;

class my_lora_header : public Header{ 

  private:
    
    uint64_t source;
    uint64_t destination;
    uint64_t original_source;
    uint64_t original_destination;

    uint64_t id;
    uint64_t original_id;
  
  public:

    my_lora_header()
      : source (0),
        destination (0),
        original_source (0),
        original_destination (0),
        id (0),
        original_id (0) 
        {}


    void setSource(uint64_t s){
      source = s;
    }
    void setDestination(uint64_t d){
      destination = d;
    }
    void setOriginalSource(uint64_t os){
      original_source = os;
    }
    void setOriginalDestination(uint64_t od){
      original_destination = od;
    }
    void setId(uint64_t i){
      id = i;
    }
    void setOriginalId(uint64_t oi){
      original_id = oi;
    }

    uint64_t getSource(){
      return source;
    }
    uint64_t getDestination(){
      return destination;
    }
    uint64_t getOriginalSource(){
      return original_source;
    }
    uint64_t getOriginalDestination(){
      return original_destination;
    }
    uint64_t getId(){
      return id;
    }
    uint64_t getOriginalId(){
      return original_id;
    }


    uint32_t GetSerializedSize (void) const{
      return 48;
    }


  void Serialize (Buffer::Iterator start) const{
    Buffer::Iterator i = start;

    i.WriteHtonU64 (source);
    i.WriteHtonU64 (destination);
    i.WriteHtonU64 (original_source);
    i.WriteHtonU64 (original_destination);
    i.WriteHtonU64 (id);
    i.WriteHtonU64 (original_id);

  }

  uint32_t Deserialize (Buffer::Iterator start) {
    Buffer::Iterator i = start;
    
    source = i.ReadNtohU64 ();
    destination = i.ReadNtohU64 ();
    original_source = i.ReadNtohU64 ();
    original_destination = i.ReadNtohU64 ();
    id = i.ReadNtohU64 ();
    original_id = i.ReadNtohU64 ();
    
    return GetSerializedSize ();
  } 

  TypeId GetTypeId () {
    static TypeId tid = TypeId ("ns3::header::my_lora_header")
    .SetParent<Header> ()
    .AddConstructor<my_lora_header> ();

    return tid;
  }

  TypeId GetInstanceTypeId (void)  const{
    static TypeId tid = TypeId ("ns3::header::my_lora_header")
    .SetParent<Header> ()
    .AddConstructor<my_lora_header> ();
    return tid;
  }

  void Print (std::ostream &os)const {
    os << "source: " << source << std::endl
    << " destination: " << destination << std::endl
    << " original_source: " << original_source << std::endl
    << " original_destination: " << original_destination << std::endl
    << " id: " << id << std::endl
    << " original_id: " << original_id<< std::endl;

    return;
  }



};

/**
 * \brief Test script.
 * 
 * This script 10 lora end-devices, one gateway. Lora end-devices send packets on three channels.
 * 
 */
class LoraExample 
{
public:
  LoraExample ();
  /**
   * \brief Configure script parameters
   * \param argc is the command line argument count
   * \param argv is the command line arguments
   * \return true on successful configuration
  */
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /**
   * Report results
   * \param os the output stream
   */
  void Report (std::ostream & os);

private:

  // parameters
  /// Number of nodes
  uint32_t size;
  /// Number of channels
  double totalChannel;
  /// Simulation time, seconds
  double totalTime;

  ObjectFactory m_phyFac;
  uint32_t m_bytesRx;

  uint32_t packetPerNode;

private:
  /// Create the nodes
  Ptr<LoraNetDevice> CreateNode (Vector pos, Ptr<LoraChannel> chan);
  Ptr<LoraNetDevice> CreateGateway (Vector pos, Ptr<LoraChannel> chan);

  bool DoExamples ();

  uint32_t DoOneExample (Ptr<LoraPropModel> prop);

  bool RxPacket (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender);
  void SendOnePacket (Ptr<LoraNetDevice> dev, uint32_t mode);

  void SendOnePacket2GW (Ptr<LoraNetDevice> dev, uint32_t mode, Address &sender);

  uint32_t RanTxTime(uint32_t fMin, uint32_t fMax);
  uint32_t random_number(uint32_t min_num, uint32_t max_num);
};

int main (int argc, char **argv)
{
  LoraExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
LoraExample::LoraExample () :
  size (10),
  totalChannel(3),
  totalTime (100)
{
}

bool
LoraExample::Configure (int argc, char **argv)
{
  CommandLine cmd;

  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("size", "Number of nodes.", totalChannel);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);

  cmd.Parse (argc, argv);
  return true;
}

void
LoraExample::Run ()
{
  std::cout << "Starting simulation for " << totalTime << " s ...\n";
  std::cout << "Creating " << size << " nodes ...\n";
  std::cout << "Transmission on " << totalChannel << " channels ...\n";

  DoExamples ();
}

void
LoraExample::Report (std::ostream &)
{ 
}


bool
LoraExample::RxPacket (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender)
{
  std::cout << "Received packet at "<< Simulator::Now().GetSeconds() << std::endl;
  
  // std::cout<< "position"<< dev->GetNode()->GetObject("MobilityModel")->GetPosition()<<std::endl;
  m_bytesRx += 1;
  return true;
}
void
LoraExample::SendOnePacket (Ptr<LoraNetDevice> dev, uint32_t mode)
{
  Ptr<Packet> pkt = Create<Packet> (13);
  dev->Send (pkt, dev->GetBroadcast (), mode);
}


void
LoraExample::SendOnePacket2GW (Ptr<LoraNetDevice> dev, uint32_t mode, Address &sender)
{
  Ptr<Packet> pkt = Create<Packet> (100);
  my_lora_header header;

  header.setSource(0);
  header.setDestination(1);
  header.setOriginalSource(0);
  header.setOriginalDestination(1);
  header.setId(0);
  header.setOriginalId(0);

  pkt->AddHeader(header);
  std::cout << "Sending packet from " << header.getSource() << " to " << header.getDestination() << std::endl;
  dev->Send (pkt, sender, mode);

}


Ptr<LoraNetDevice>
LoraExample::CreateNode (Vector pos, Ptr<LoraChannel> chan)
{

  Ptr<LoraPhy> phy = m_phyFac.Create<LoraPhy> ();
  Ptr<Node> node = CreateObject<Node> ();
  Ptr<LoraNetDevice> dev = CreateObject<LoraNetDevice> ();
  Ptr<MacLoraAca> mac = CreateObject<MacLoraAca> ();
  Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();

  Ptr<LoraTransducerHd> trans = CreateObject<LoraTransducerHd> ();

  mobility->SetPosition (pos);
  node->AggregateObject (mobility);
  mac->SetAddress (LoraAddress::Allocate ());

  dev->SetPhy (phy);
  dev->SetMac (mac);
  dev->SetChannel (chan);
  dev->SetTransducer (trans);
  node->AddDevice (dev);

  return dev;
}

Ptr<LoraNetDevice>
LoraExample::CreateGateway (Vector pos, Ptr<LoraChannel> chan)
{

  Ptr<LoraPhy> phy = m_phyFac.Create<LoraPhy> ();
  Ptr<Node> node = CreateObject<Node> ();
  Ptr<LoraNetDevice> dev = CreateObject<LoraNetDevice> ();

  Ptr<MacLoraAca> mac = CreateObject<MacLoraAca> ();

  Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();

  Ptr<LoraTransducerHd> trans = CreateObject<LoraTransducerHd> ();

  mobility->SetPosition (pos);
  node->AggregateObject (mobility);
  mac->SetAddress (LoraAddress::Allocate ());

  dev->SetPhy (phy);
  dev->SetMac (mac);
  dev->SetChannel (chan);
  dev->SetTransducer (trans);
  node->AddDevice (dev);

  return dev;
}

uint32_t
LoraExample::RanTxTime(uint32_t fMin, uint32_t fMax)
{
    return random_number(fMin, fMax);
}


uint32_t 
LoraExample::random_number(uint32_t min_num, uint32_t max_num)
{
    uint32_t  result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1; 
    } else {
        low_num = max_num + 1; 
        hi_num = min_num;
    }

    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}


uint32_t
LoraExample::DoOneExample (Ptr<LoraPropModel> prop)
{
  Ptr<LoraChannel> channel = CreateObject<LoraChannel> ();
  channel->SetAttribute ("PropagationModel", PointerValue (prop));

  Ptr<LoraNetDevice> gw0 = CreateGateway (Vector (50,50,50), channel);

//Set positions to nodes
  uint32_t x = 50; uint32_t y = 50; uint32_t z = 50; 
  uint32_t n = 10; 

  Ptr<LoraNetDevice> PtrDevice[n];
  for (uint32_t i = 0; i < n; i++)
  {
      PtrDevice[i] = CreateNode (Vector (x,y,z), channel);
      x += 0; y += 1000; z += 0;

      PtrDevice[i]->SetGWAddress(gw0->GetAddress()); 
      PtrDevice[i]->SetReceiveCallback (MakeCallback (&LoraExample::RxPacket, this));             
  }

//Set gateway to receive packets from end devices node.
  gw0->SetReceiveCallback (MakeCallback (&LoraExample::RxPacket, this));

  Simulator::Schedule (Seconds(10), &LoraExample::SendOnePacket2GW, this, PtrDevice[0],0, PtrDevice[1]->GetMac()->GetBroadcast());
  Simulator::Schedule (Seconds(10), &LoraExample::SendOnePacket2GW, this, PtrDevice[4],1, PtrDevice[1]->GetMac()->GetBroadcast());
  m_bytesRx = 0;
  Simulator::Stop (Days(100));
  Simulator::Run ();
  Simulator::Destroy ();

  return m_bytesRx;
}

bool
LoraExample::DoExamples ()
{

  LoraModesList mList;
  LoraTxMode mode = LoraTxModeFactory::CreateMode (LoraTxMode::LORA, 80, 80, 10000, 4000, 2, "TestMode");
  mList.AppendMode (LoraTxMode (mode));

  Ptr<LoraPhyPerGenDefault> perDef = CreateObject<LoraPhyPerGenDefault> ();
  Ptr<LoraPhyCalcSinrDefault> sinrDef = CreateObject<LoraPhyCalcSinrDefault> ();
  m_phyFac.SetTypeId ("ns3::LoraPhyGen");
  m_phyFac.Set ("PerModel", PointerValue (perDef));
  m_phyFac.Set ("SinrModel", PointerValue (sinrDef));
  m_phyFac.Set ("SupportedModes", LoraModesListValue (mList));

  Ptr<LoraPropModelThorp> prop = CreateObject<LoraPropModelThorp> ();

  LoraTxMode mode00 = LoraTxModeFactory::CreateMode (LoraTxMode::LORA, 100, 120, 10000, 125, 2, "TestMode00");
  LoraTxMode mode01 = LoraTxModeFactory::CreateMode (LoraTxMode::LORA, 100, 120, 11000, 100, 2, "TestMode01");
  LoraTxMode mode02 = LoraTxModeFactory::CreateMode (LoraTxMode::LORA, 300, 120, 12000, 125, 2, "TestMode02");

  LoraModesList m0;
  m0.AppendMode (mode00);
  LoraModesList m1;
  m1.AppendMode (mode01);
  LoraModesList m2;
  m2.AppendMode (mode02);

  m_phyFac = ObjectFactory ();
  m_phyFac.SetTypeId ("ns3::LoraPhyDual");

  m_phyFac.Set ("SupportedModesPhy1", LoraModesListValue (m0));
  m_phyFac.Set ("SupportedModesPhy2", LoraModesListValue (m1));
  m_phyFac.Set ("SupportedModesPhy3", LoraModesListValue (m2));

  uint32_t n_ReceivedPacket = DoOneExample(prop);
  
  std::cout << "Received " << n_ReceivedPacket << " packets\n";

  return false;
}