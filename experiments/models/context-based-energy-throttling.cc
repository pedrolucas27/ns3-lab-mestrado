#include "context-based-energy-throttling.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ContextBasedEnergyThrottling");

ContextBasedEnergyThrottling::ContextBasedEnergyThrottling()
    : m_received(0),
      m_accepted(0),
      m_rejected(0),
      m_received_forwarded(0),
      m_accepted_forwarded(0),
      m_rejected_forwarded(0),
      m_sent_forwarded(0)
{
}

void ContextBasedEnergyThrottling::printResults(){
    NS_LOG_INFO("Solicitações avaliadas por este dispositivo (total): " << m_received);
    NS_LOG_INFO("  - Recebidas já reencaminhadas por outro nó: " << m_received_forwarded);
    NS_LOG_INFO("Solicitações processadas localmente/aceitas (total): " << m_accepted);
    NS_LOG_INFO("  - Entre as aceitas, recebidas de nós vizinhos: " << m_accepted_forwarded);
    NS_LOG_INFO("Solicitações rejeitadas localmente por falta de energia (total): " << m_rejected);
    NS_LOG_INFO("  - Entre as rejeitadas localmente, recebidas de nós vizinhos: " << m_rejected_forwarded);
    NS_LOG_INFO("Solicitações rejeitadas localmente e encaminhadas a um vizinho: " << m_sent_forwarded);
}

bool ContextBasedEnergyThrottling::canProcessRequestLocally(Ptr<EnergyBuffer> deviceEnergyBuffer, double energyConsumptionPerRequest, bool isForwarded){
    NS_LOG_INFO("Verificando se o dispositivo pode processar a solicitação localmente...");

    m_received++;
    if (isForwarded) {
        m_received_forwarded++;
    }

    bool result = deviceEnergyBuffer->hasEnergy() && deviceEnergyBuffer->getEnergyAmount() >= energyConsumptionPerRequest;

    if (result){
        m_accepted++;
        if (isForwarded) {
            m_accepted_forwarded++;
        }
    } else {
        m_rejected++;
        if (isForwarded) {
            m_rejected_forwarded++;
        }
    }

    return result;
}

void ContextBasedEnergyThrottling::registerPacketForwarded(){
    m_sent_forwarded++;
}

} // namespace ns3