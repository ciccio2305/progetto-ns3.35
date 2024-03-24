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
struct MY_msg {
  uint32_t type;
  uint32_t source;
  uint32_t destination;
  uint32_t original_source;
  uint32_t original_destination;
  uint32_t id;
  uint32_t original_id;
};

void set_msg(MY_msg *m,uint32_t t, uint32_t s, uint32_t d, uint32_t os, uint32_t od, uint32_t i, uint32_t oi){
  
  m->type = t;
  m->source = s;
  m->destination = d;
  m->original_source = os;
  m->original_destination = od;
  m->id = i;
  m->original_id = oi;

}

typedef std::vector< MY_msg > my_list;

class my_lora_header : public Header{ 

  private:
    //type 0 classic message, 1 ack
    uint32_t type;
    uint32_t source;
    uint32_t destination;
    uint32_t original_source;
    uint32_t original_destination;

    uint32_t id;
    uint32_t original_id;
  public:

    my_lora_header()
      : type (0),
        source (0),
        destination (0),
        original_source (0),
        original_destination (0),
        id (0),
        original_id (255) 
        {
          
        }
    //set in order: type, source, destination, original_source, original_destination, id, original_id
    void set_all(uint32_t t,uint32_t s, uint32_t d, uint32_t os, uint32_t od, uint32_t i, uint32_t oi){
      
      type = t;
      source = s;
      destination = d;
      original_source = os;
      original_destination = od;
      id = i;
      original_id = oi;


    }
    
    void setSource(uint32_t s){
      source = s;
    }
    
    void setType(uint32_t t){
      type = t;
    }

    void setDestination(uint32_t d){
      destination = d;
    }
    
    void setOriginalSource(uint32_t os){
      original_source = os;
    }
    
    void setOriginalDestination(uint32_t od){
      original_destination = od;
    }
    
    void setId(uint32_t i){
      id = i;
    }
    
    void setOriginalId(uint32_t oi){
      original_id = oi;
    }

    uint32_t getSource(){
      return source;
    }
    
    uint32_t getDestination(){
      return destination;
    }
    
    uint32_t getOriginalSource(){
      return original_source;
    }
    
    uint32_t getOriginalDestination(){
      return original_destination;
    }
    
    uint32_t getId(){
      return id;
    }
    
    uint32_t getOriginalId(){
      return original_id;
    }

    uint32_t GetSerializedSize (void) const{
      return 28;
    }

    uint32_t GetType () {
      return type;
    }

  void Serialize (Buffer::Iterator start) const{
    Buffer::Iterator i = start;
    i.WriteHtonU32 (type);
    i.WriteHtonU32 (source);
    i.WriteHtonU32 (destination);
    i.WriteHtonU32 (original_source);
    i.WriteHtonU32 (original_destination);
    i.WriteHtonU32 (id);
    i.WriteHtonU32 (original_id);

  }

  uint32_t Deserialize (Buffer::Iterator start) {
    Buffer::Iterator i = start;
    type = i.ReadNtohU32 ();
    source = i.ReadNtohU32 ();
    destination = i.ReadNtohU32 ();
    original_source = i.ReadNtohU32 ();
    original_destination = i.ReadNtohU32 ();
    id = i.ReadNtohU32 ();
    original_id = i.ReadNtohU32 ();
    
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
    os <<"type: "<< type << std::endl
    << "source: " << source << std::endl
    << " destination: " << destination << std::endl
    << " original_source: " << original_source << std::endl
    << " original_destination: " << original_destination << std::endl
    << " id: " << id << std::endl
    << " original_id: " << original_id<< std::endl;

    return;
  }



};

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
  
  Ptr<LoraNetDevice> *PtrDevice;
  my_list* msgs_queues;

  uint32_t * counterArray;

  uint32_t counter_msg_reinoltro;
  uint32_t counter_msg_ricevuti_dal_oi;
  uint32_t counter_ack;
  uint32_t counter_msg_inviati;
  uint32_t counter_msg_ricevuti_dal_destinatario;


private:
  /// Create the nodes
  Ptr<LoraNetDevice> CreateNode (Vector pos, Ptr<LoraChannel> chan);
  Ptr<LoraNetDevice> CreateGateway (Vector pos, Ptr<LoraChannel> chan);

  bool DoExamples ();

  uint32_t DoOneExample (Ptr<LoraPropModel> prop);

  bool RxPacket (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender);
  void SendOnePacket (Ptr<LoraNetDevice> dev, uint32_t mode);

  void SendOnePacket2GW (Ptr<LoraNetDevice> dev, MY_msg pkt, uint32_t mode, Address &sender);

  uint32_t FindIndex(Ptr<NetDevice> dev);

  uint32_t RanTxTime(uint32_t fMin, uint32_t fMax);
  uint32_t random_number(uint32_t min_num, uint32_t max_num);

  void add_msg_in_queue(MY_msg m,uint32_t node_index){
    msgs_queues[node_index].push_back(m);
  }

  void remove_msg_from_queue(uint32_t id, uint32_t node_index){
    my_list::iterator it; 
     for (it = msgs_queues[node_index].begin(); it != msgs_queues[node_index].end(); it++ ) {
      if (it->id == id) {
        msgs_queues[node_index].erase(it);
        break;
      }
    }
  }

  void check_queue(MY_msg m, uint32_t node_index){
    my_list::iterator it; 
    for (it = msgs_queues[node_index].begin(); it != msgs_queues[node_index].end(); it++ ) {
      
      if (it->id == m.id) {
        std::cout << "messaggio non ricevuto dal destinatario finale"<<std::endl<<"al tempo :"<<Simulator::Now().GetSeconds()<<std::endl;
        msgs_queues[node_index].erase(it);
        m.id = counterArray[node_index]++;
        Simulator::Schedule(Seconds(1), &LoraExample::SendOnePacket2GW, this, PtrDevice[node_index], m, 0, PtrDevice[node_index]->GetMac()->GetBroadcast());
        counter_msg_reinoltro++;
        return;
      }
    }
  }

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
  counterArray = new uint32_t [size];
  for (size_t i = 0; i < size; i++) counterArray[i] = 0;
  msgs_queues =new my_list[size];
  
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

uint32_t
LoraExample::FindIndex(Ptr<NetDevice> dev)
{
  for (size_t i = 0; i < size; i++) {
    if (PtrDevice[i] == dev) return i;
  }
  return size;
}

bool
LoraExample::RxPacket (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender)
{
  
  uint32_t index = FindIndex(dev);
  my_lora_header header;
  pkt->PeekHeader(header);
  
  uint32_t dest = header.getDestination();
  uint32_t originalDest = header.getOriginalDestination();
  uint32_t src = header.getSource();


  if (dest == index) {
    
    if(header.GetType() == 1&&header.getOriginalDestination() == index){
      counter_ack++;
      remove_msg_from_queue(header.getOriginalId(),index);
    }

    if(header.GetType() == 0){
      MY_msg m_ack;
      
      set_msg(&m_ack,1, 
        index, 
        header.getSource(), 
        index, 
        header.getSource(), 
        counterArray[index]++, 
        header.getId()
      );
      
      Simulator::Schedule(Seconds(0.1), &LoraExample::SendOnePacket2GW, this, PtrDevice[index], m_ack, mode, PtrDevice[index]->GetMac()->GetBroadcast());
    
    }

    //se non sono il destinatario finale reinoltro il messaggio
    if (originalDest != index && header.GetType() == 0) {
        counter_msg_ricevuti_dal_destinatario++;
        uint32_t dest=0;
        if (src == index - 1) dest=index + 1;
          else dest=index - 1;

        MY_msg m_reinoltro;
        
        set_msg(&m_reinoltro,0,
          index, 
          dest, 
          header.getOriginalSource(), 
          header.getOriginalDestination(), 
          counterArray[index]++, 
          header.getOriginalId()
        );
          
        Simulator::Schedule(Seconds(3.7), &LoraExample::SendOnePacket2GW, this, PtrDevice[index], m_reinoltro, mode, PtrDevice[index]->GetMac()->GetBroadcast());
    }

     if (originalDest == index&& header.GetType() == 0){
      std::cout << "messaggio arrivato al gateway"<<std::endl<<"al tempo :"<<Simulator::Now().GetSeconds()<<std::endl;
      counter_msg_ricevuti_dal_oi++;
     }

    //se il messaggio era era mer e non era un ack invio l'ack di risposta
    

  }
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
LoraExample::SendOnePacket2GW (Ptr<LoraNetDevice> dev, MY_msg pkt, uint32_t mode, Address &sender)
{ 
  
  counter_msg_inviati++;
  Ptr<Packet> pkt2 = Create<Packet>(100);
  my_lora_header header;
  header.set_all(
    pkt.type,
    pkt.source, 
    pkt.destination, 
    pkt.original_source, 
    pkt.original_destination, 
    pkt.id, 
    pkt.original_id);
  
  pkt2->AddHeader(header);
  dev->Send (pkt2, sender, mode);
  
  if(pkt.type == 0){
    this->add_msg_in_queue(pkt,pkt.source);
    Simulator::Schedule(Seconds(30), &LoraExample::check_queue, this, pkt,pkt.source);
  }
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

  PtrDevice = new Ptr<LoraNetDevice>[size];

  for (uint32_t i = 0; i < size; i++)
  {
      PtrDevice[i] = CreateNode (Vector (x,y,z), channel);
      x += 0; y += 8; z += 0;

      PtrDevice[i]->SetGWAddress(gw0->GetAddress()); 
      PtrDevice[i]->SetReceiveCallback (MakeCallback (&LoraExample::RxPacket, this));             
  }


  gw0->SetReceiveCallback (MakeCallback (&LoraExample::RxPacket, this));

  uint32_t id=counterArray[6]++;
  MY_msg m;
  
  set_msg(&m,0, 6, 5, 6, 0, id, id);
  Simulator::Schedule (Seconds(10), &LoraExample::SendOnePacket2GW, this, PtrDevice[6], m, 0, PtrDevice[6]->GetMac()->GetBroadcast());
  
  id=counterArray[7]++;
  MY_msg m2;
  set_msg(&m2,0, 7, 6, 7, 0, id, id);
  Simulator::Schedule (Seconds(11), &LoraExample::SendOnePacket2GW, this, PtrDevice[7], m2, 1, PtrDevice[1]->GetMac()->GetBroadcast());
  


  counter_ack = 0;
  counter_msg_reinoltro = 0;
  counter_msg_ricevuti_dal_oi = 0;
  counter_msg_ricevuti_dal_destinatario = 0;
  counter_msg_inviati = 0;

  m_bytesRx = 0;
  Simulator::Stop (Seconds(20000));
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

  LoraTxMode mode00 = LoraTxModeFactory::CreateMode (LoraTxMode::LORA, 300, 120, 8681, 100, 2, "TestMode00");
  LoraTxMode mode01 = LoraTxModeFactory::CreateMode (LoraTxMode::LORA, 300, 120, 8682, 100, 2, "TestMode01");
  LoraTxMode mode02 = LoraTxModeFactory::CreateMode (LoraTxMode::LORA, 300, 120, 8683, 100, 2, "TestMode02");

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
  
  std::cout << " Received packets: " << n_ReceivedPacket <<std::endl<<
    " Ack ricevute: " << counter_ack <<std::endl<<
    " Msg reinoltro: " << counter_msg_reinoltro <<std::endl<<
    " Msg ricevuti dal final: " << counter_msg_ricevuti_dal_oi <<std::endl<<
    " Msg inviati: " << counter_msg_inviati <<std::endl<<
    " Msg ricevuti dal destinatario intermedio per il reinoltro: " << counter_msg_ricevuti_dal_destinatario 
    << std::endl;

  return false;
}