#ifndef CONTEXT_BASED_ENERGY_THROTTLING_H
#define CONTEXT_BASED_ENERGY_THROTTLING_H

#include "ns3/ptr.h"
#include "energy-buffer.h"

namespace ns3 {

class ContextBasedEnergyThrottling {
public:
    ContextBasedEnergyThrottling();

    bool canProcessRequestLocally(Ptr<EnergyBuffer> energyBuffer, double energyConsumptionPerRequest, bool isForwarded);

    void registerPacketForwarded(); //!< Chamado quando o dispositivo encaminha um pacote a um vizinho

    void printResults();

private:
    int m_received;            //!< Total de solicitações recebidas (originais + já-reencaminhadas)
    int m_accepted;             //!< Total de solicitações processadas/aceitas
    int m_rejected;             //!< Total de solicitações rejeitadas
    int m_received_forwarded;   //!< Entre as recebidas, quantas já chegaram marcadas como reencaminhadas
    int m_accepted_forwarded;   //!< Entre as já-reencaminhadas recebidas, quantas foram aceitas
    int m_rejected_forwarded;   //!< Entre as já-reencaminhadas recebidas, quantas foram rejeitadas
    int m_sent_forwarded;       //!< Quantos pacotes este dispositivo encaminhou para um vizinho
};

} // namespace ns3

#endif // CONTEXT_BASED_ENERGY_THROTTLING_H