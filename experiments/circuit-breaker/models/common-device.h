#ifndef COMMON_DEVICE_H
#define COMMON_DEVICE_H

#include "energy-buffer.h"

#include "ns3/event-id.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "ns3/socket.h"

namespace ns3 {

/**
 * Dispositivo comum em uma rede, com nó associado e buffer de energia.
 */
class CommonDevice : public SimpleRefCount<CommonDevice>
{
  public:
    CommonDevice(Ptr<Node> node,
                 uint16_t port,
                 double initial_energy,
                 double capacity,
                 double active_cost,
                 double idle_cost,
                 Time harvest_interval,
                 double harvest_energy,
                 Time idle_interval);

    void start();

    /** Cancela harvest/idle para a simulacao poder terminar. */
    void stop();

    /** Disponivel = tem energia para atender e nao esta em crash. */
    bool is_available() const;

    /** Modos de falha programaveis pelo cenario. */
    void crash();
    void recover();

    double get_energy_amount() const;
    double get_min_energy() const;
    Ptr<Node> get_node() const;

  private:
    void receive_packet(Ptr<Socket> socket);
    void process_request();          
    void harvest_tick();
    void idle_tick();

    Ptr<Node> m_node;
    Ptr<Socket> m_socket;
    uint16_t m_port;

    Ptr<EnergyBuffer> m_energy_buffer;
    double m_active_cost;
    double m_idle_cost;              
    Time m_harvest_interval;
    double m_harvest_energy;
    Time m_idle_interval;

    EventId m_harvest_event;
    EventId m_idle_event;

    bool m_crashed;
};

} // namespace ns3

#endif // COMMON_DEVICE_H
