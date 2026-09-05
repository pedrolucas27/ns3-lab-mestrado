#ifndef COMMON_DEVICE_H
#define COMMON_DEVICE_H

#include "context-based-energy-throttling.h"
#include "energy-buffer.h"

#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "ns3/socket.h"

#include <vector>

namespace ns3 {

/**
 * Dispositivo comum em uma rede, com nó associado e buffer de energia.
 */
class CommonDevice : public SimpleRefCount<CommonDevice> {
  public:
    /**
     * Construtor da classe CommonDevice.
     * @param node Ponteiro para o nó associado ao dispositivo.
     * @param initialEnergy Energia inicial do buffer de energia.
     * @param capacity Capacidade do buffer de energia.
     */
    CommonDevice(Ptr<Node> node, double initialEnergy, double capacity);

    Ptr<Node> getNode();

    void addNeighbor(Ptr<CommonDevice> neighbor);

    std::vector<Ptr<CommonDevice>> getNeighbors();

    void receivePacket(Ptr<Socket> socket);

    void forwardPacket(Ptr<Packet> packet, Ptr<CommonDevice> neighbor);

    void printStats();

    void start();

  private:
    Ptr<Node> m_node;                                 //!< Nó associado em rede
    
    Ptr<EnergyBuffer> m_energy_buffer;                //!< Buffer de energia
    
    std::vector<Ptr<CommonDevice>> m_neighbors;       //!< Vizinhos do dispositivo
    
    Ptr<Socket> m_forward_socket; //!< Socket usado para encaminhar pacotes a vizinhos
    
    ContextBasedEnergyThrottling m_energy_throttling; //!< Estratégia de controle de energia

    void harvestTick(); //!< Método de colheita de energia acionado periodicamente
    
    void idleTick();    //!< Método de consumo de energia
};

} // namespace ns3

#endif // COMMON_DEVICE_H
