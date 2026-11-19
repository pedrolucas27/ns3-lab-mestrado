#include "common-device.h"
#include "service-unavailable-exception.h"

#include "ns3/core-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/udp-socket-factory.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CommonDevice");

CommonDevice::CommonDevice(Ptr<Node> node,
                           uint16_t port,
                           double initial_energy,
                           double capacity,
                           double active_cost,
                           double idle_cost,
                           Time harvest_interval,
                           double harvest_energy,
                           Time idle_interval)
    : m_node(node),
      m_port(port),
      m_energy_buffer(Create<EnergyBuffer>(initial_energy, capacity)),
      m_active_cost(active_cost),
      m_idle_cost(idle_cost),
      m_harvest_interval(harvest_interval),
      m_harvest_energy(harvest_energy),
      m_idle_interval(idle_interval)
{
}

void CommonDevice::start()
{
    m_socket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
    m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));
    m_socket->SetRecvCallback(MakeCallback(&CommonDevice::receive_packet, this));

    m_harvest_event = Simulator::Schedule(m_harvest_interval, &CommonDevice::harvest_tick, this);
    m_idle_event = Simulator::Schedule(m_idle_interval, &CommonDevice::idle_tick, this);
}

void CommonDevice::stop()
{
    Simulator::Cancel(m_harvest_event);
    Simulator::Cancel(m_idle_event);
}

double CommonDevice::get_energy_amount() const
{
    return m_energy_buffer->get_energy_amount();
}

double CommonDevice::get_min_energy() const
{
    return m_energy_buffer->get_min_energy();
}

Ptr<Node> CommonDevice::get_node() const
{
    return m_node;
}

void CommonDevice::process_request()
{
    if (m_energy_buffer->get_energy_amount() < m_active_cost){
        throw OutOfEnergyException();
    }

    m_energy_buffer->consume_energy(m_active_cost);
}

void CommonDevice::receive_packet(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0){
            continue;
        }

        uint8_t status = 1;
        try{
            process_request();
        } catch (const ServiceUnavailableException&){
            status = 0; // intermitencia: custo ativo maior que a energia disponivel
        }

        socket->SendTo(Create<Packet>(&status, 1), 0, from);
    }
}

void CommonDevice::harvest_tick()
{
    m_energy_buffer->recharge(m_harvest_energy);
    m_harvest_event = Simulator::Schedule(m_harvest_interval, &CommonDevice::harvest_tick, this);
}

void CommonDevice::idle_tick()
{
    m_energy_buffer->consume_energy(m_idle_cost * m_idle_interval.GetSeconds());
    m_idle_event = Simulator::Schedule(m_idle_interval, &CommonDevice::idle_tick, this);
}

} // namespace ns3