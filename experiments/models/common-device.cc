#include "common-device.h"
#include "router-tag.h"

#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/ipv4.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"

#include <algorithm>
#include <cstdint>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CommonDevice");

CommonDevice::CommonDevice(Ptr<Node> node,
                           double initialEnergy,
                           double capacity,
                           double activeCost,
                           double transmissionCost,
                           double idleCost,
                           double harvestInterval,
                           double harvestEnergy)
    : m_node(node),
      m_energy_buffer(Create<EnergyBuffer>(initialEnergy, capacity)),
      m_active_cost(activeCost),
      m_transmission_cost(transmissionCost),
      m_idle_cost(idleCost),
      m_harvest_interval(Seconds(harvestInterval).GetSeconds()),
      m_harvest_energy(harvestEnergy),
      m_energy_throttling()
{
}

Ptr<Node> CommonDevice::getNode(){
    return m_node;
}

std::vector<Ptr<CommonDevice>> CommonDevice::getNeighbors() {
    return m_neighbors;
}

void CommonDevice::setNeighbors(const std::vector<Ptr<CommonDevice>>& neighbors) {
    m_neighbors = neighbors;
}

bool getRouteTag(Ptr<Packet> packet, RouteTag& tag) {
    return packet->PeekPacketTag(tag);
}

void CommonDevice::receivePacket(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))) {
        if (packet->GetSize() == 0) {
            continue;
        }

        // Verifica o número de hops do pacote que possui uma tag para evitar que o failover forme ciclos.
        RouteTag route;
        bool isForwarded = getRouteTag(packet, route) && route.m_hops > 0;
        if (isForwarded && route.m_hops >= route.m_max_hops) {
            // O limite pertence ao pacote; o dispositivo apenas o respeita.
            continue;
        }

        if (m_energy_throttling.canProcessRequestLocally(m_energy_buffer, m_active_cost, isForwarded)) {
            m_energy_buffer->consumeEnergy(m_active_cost);
            NS_LOG_DEBUG("Dispositivo " << m_node->GetId() << ": pacote processado localmente.");
            continue;
        }

        // Seleciona o vizinho disponível com mais energia para o failover.
        Ptr<CommonDevice> nextNeighbor;
        double highestEnergy = -1.0;

        for (const Ptr<CommonDevice>& candidate : m_neighbors) {
            // Cria uma máscara de bits correspondente ao ID do candidato.
            uint64_t candidateBit = 1ULL << candidate->getNode()->GetId();
            // Ignora o próprio nó e nós já visitados por pacotes encaminhados.
            if (candidate == this || (isForwarded && (route.m_visited & candidateBit))) {
                continue; // Pula
            }

            // Obtém a quantidade atual de energia do candidato.
            double candidateEnergy = candidate->getEnergyAmount();
            // Aceita somente candidatos capazes de processar a solicitação e com mais energia que o anterior.
            if (candidateEnergy >= candidate->m_active_cost && candidateEnergy > highestEnergy) {
                // Atualiza o maior nível de energia encontrado.
                highestEnergy = candidateEnergy;
                // Define este candidato como o próximo nó do failover.
                nextNeighbor = candidate;
            }
        }

        if (nextNeighbor == nullptr) {
            NS_LOG_WARN("Dispositivo " << m_node->GetId()<< ": nenhum dispositivo disponível para failover.");
            continue;
        }

        // Failover: realiza a transmissão do pacote ao próximo nó.
        if (m_energy_buffer->consumeEnergy(m_transmission_cost)) {
            // O nó atual paga a transmissão e acrescenta um salto à rota.
            route.m_hops++;
            // Marca o nó atual para impedir ciclos no failover.
            route.m_visited |= 1ULL << m_node->GetId();
            // Atualiza a rota armazenada no pacote.
            packet->ReplacePacketTag(route);
            // Encaminha o pacote ao vizinho selecionado.
            forwardPacket(packet, nextNeighbor);
        }
    }
}

double CommonDevice::getEnergyAmount() const {
    return m_energy_buffer->getEnergyAmount();
}

EnergyMetrics CommonDevice::getMetrics() const {
    return m_energy_throttling.getMetrics();
}

void CommonDevice::forwardPacket(Ptr<Packet> packet, Ptr<CommonDevice> neighbor){
    Ptr<Ipv4> ipv4 = neighbor->getNode()->GetObject<Ipv4>();
    Ipv4Address neighborAddr = ipv4->GetAddress(1, 0).GetLocal();

    if (!m_forward_socket) {
        m_forward_socket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
        m_forward_socket->Bind();
    }

    m_energy_throttling.registerPacketForwarded();

    m_forward_socket->SendTo(packet, 0, InetSocketAddress(neighborAddr, 4000));

    NS_LOG_DEBUG("Pacote encaminhado para dispositivo " << neighbor->getNode()->GetId());
}

void CommonDevice::harvestTick(){
    // Simula a colheita de energia a cada segundo
    bool wasOff = !m_energy_buffer->hasEnergy();
    m_energy_buffer->recharge(m_harvest_energy);

    if(wasOff && m_energy_buffer->hasEnergy()){
        NS_LOG_INFO("Dispositivo " << m_node->GetId() << ": energia colhida, dispositivo voltou a funcionar.");
    }

    Simulator::Schedule(Seconds(m_harvest_interval), &CommonDevice::harvestTick, this);
}

void CommonDevice::idleTick(){
    // Simula o consumo de energia se houver energia disponível
    if(m_energy_buffer->hasEnergy()){
        m_energy_buffer->consumeEnergy(m_idle_cost);
    } else {
        NS_LOG_WARN("Dispositivo sem energia para consumo");
    }

    Simulator::Schedule(Seconds(1.0), &CommonDevice::idleTick, this);
}

void CommonDevice::start(){
    Simulator::Schedule(Seconds(1.0), &CommonDevice::harvestTick, this);
    Simulator::Schedule(Seconds(1.0), &CommonDevice::idleTick, this);
}

void CommonDevice::printStats(){
    NS_LOG_INFO("===== Estatísticas do dispositivo " << m_node->GetId() << " =====");
    NS_LOG_INFO("Energia restante: " << m_energy_buffer->getEnergyAmount());
    m_energy_throttling.printResults();
}

} // namespace ns3
