#include "common-device.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/ipv4.h"
#include "ns3/tag.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CommonDevice");

static const double ENERGY_CONSUMPTION_PER_PACKET = 10.0;

/*
 Classe que herda de Tag (classe abstrata) utilizada para marcar os pacotes que foram reencaminhados, ou seja, que passaram por forwarding.
*/
class ForwardedTag : public Tag {
    public:
        static TypeId GetTypeId(){
            static TypeId tid = TypeId("ns3::ForwardedTag").SetParent<Tag>().AddConstructor<ForwardedTag>();
            return tid;
        }
        TypeId GetInstanceTypeId() const override { return GetTypeId(); }
        uint32_t GetSerializedSize() const override { return 0; }
        void Serialize(TagBuffer i) const override {}
        void Deserialize(TagBuffer i) override {}
        void Print(std::ostream& os) const override { os << "forwarded"; }
};

CommonDevice::CommonDevice(Ptr<Node> node, double initialEnergy, double capacity)
    : m_node(node),
      m_energy_buffer(Create<EnergyBuffer>(initialEnergy, capacity)),
      m_neighbors(),
      m_energy_throttling()
{
}

bool CommonDevice::consumeEnergy(double amount){
    return m_energy_buffer->consumeEnergy(amount);
}

void CommonDevice::recharge(double energy){
    m_energy_buffer->recharge(energy);
}

bool CommonDevice::hasEnergy(){
    return m_energy_buffer->hasEnergy();
}

double CommonDevice::getEnergyAmount(){
    return m_energy_buffer->getEnergyAmount();
}

Ptr<Node> CommonDevice::getNode(){
    return m_node;
}

void CommonDevice::addNeighbor(Ptr<CommonDevice> neighbor){
    m_neighbors.push_back(neighbor);
}

std::vector<Ptr<CommonDevice>> CommonDevice::getNeighbors(){
    return m_neighbors;
}

bool checkIfPacketIsForwarded(Ptr<Packet> packet) {
    ForwardedTag tag;
    return packet->PeekPacketTag(tag);
}

void CommonDevice::receivePacket(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))){
        NS_LOG_INFO("Dispositivo " << m_node->GetId() << ": pacote de tamanho " << packet->GetSize() << " bytes recebido");

        if (packet->GetSize() == 0){
            break;
        }

        bool isForwarded = checkIfPacketIsForwarded(packet);
        if (isForwarded) {
            NS_LOG_INFO("Esse pacote já foi reencaminhado por outro dispositivo.");
        }

        bool canProcessLocally = m_energy_throttling.canProcessRequestLocally(m_energy_buffer, ENERGY_CONSUMPTION_PER_PACKET, isForwarded);
        if (canProcessLocally){
            NS_LOG_INFO("Dispositivo " << m_node->GetId() << ": processando pacote localmente.");
            consumeEnergy(ENERGY_CONSUMPTION_PER_PACKET);
        } else {
            NS_LOG_INFO("Dispositivo " << m_node->GetId() << ": não pode processar pacote localmente, buscando nó adjacente.");
            if(m_neighbors.size() > 0){
                Ptr<CommonDevice> next_neighbor = m_neighbors[0];
                NS_LOG_INFO("Enviando pacote para o nó: " << next_neighbor->getNode()->GetId());

                forwardPacket(packet, next_neighbor);
            }
        }
    }
}

void CommonDevice::forwardPacket(Ptr<Packet> packet, Ptr<CommonDevice> neighbor){    
    Ptr<Ipv4> ipv4 = neighbor->getNode()->GetObject<Ipv4>();
    Ipv4Address neighborAddr = ipv4->GetAddress(1, 0).GetLocal();

    if (!m_forward_socket) {
        m_forward_socket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
        m_forward_socket->Bind();
    }

    // Marca o pacote como reencaminhado antes de enviá-lo para o vizinho
    packet->AddPacketTag(ForwardedTag());

    m_energy_throttling.registerPacketForwarded();

    m_forward_socket->SendTo(packet, 0, InetSocketAddress(neighborAddr, 4000));

    NS_LOG_INFO("Pacote encaminhado para dispositivo " << neighbor->getNode()->GetId());
}

void CommonDevice::printStats(){
    NS_LOG_INFO("===== Estatísticas do dispositivo " << m_node->GetId() << " =====");
    NS_LOG_INFO("Energia restante: " << getEnergyAmount());
    m_energy_throttling.printResults();
}

} // namespace ns3
