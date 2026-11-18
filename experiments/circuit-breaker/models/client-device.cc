#include "client-device.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE("ClientDevice");

ClientDevice::ClientDevice(Ptr<Node> node,
                           Address server_address,
                           Time request_interval,
                           Time response_timeout,
                           Ptr<CircuitBreaker> breaker)
    : m_node(node),
      m_server_address(server_address),
      m_request_interval(request_interval),
      m_response_timeout(response_timeout),
      m_breaker(breaker),
      m_total_requests(0),
      m_waiting(false),
      m_sent(0),
      m_ok(0),
      m_failed(0),
      m_rejected(0)
{
}

void ClientDevice::start(){
    m_socket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
    m_socket->Bind();
    m_socket->Connect(m_server_address);
    m_socket->SetRecvCallback(MakeCallback(&ClientDevice::handle_response, this));
    send_request();
}

void ClientDevice::stop(){
    Simulator::Cancel(m_send_event);
    Simulator::Cancel(m_timeout_event);
    if (m_socket){
        m_socket->Close();
    }
}

void ClientDevice::send_request(){
    if (m_total_requests > 0 && (m_sent + m_rejected) >= m_total_requests) {
        Simulator::Cancel(m_send_event);
        return;
    }

    // O dataset tem request_interval < response_timeout. Sem este guarda, a
    // proxima sonda sobrescreve m_waiting/m_timeout_event e corrompe as metricas.
    if (m_waiting) {
        m_send_event = Simulator::Schedule(m_request_interval, &ClientDevice::send_request, this);
        return;
    }

    // Bloqueado aqui, o pacote nem chega ao radio: economiza energia nas duas pontas.
    if (m_breaker->allow_request()){
        m_socket->Send(Create<Packet>(4));
        m_sent++;
        m_waiting = true;
        m_timeout_event =
            Simulator::Schedule(m_response_timeout, &ClientDevice::on_timeout, this);

        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                        << " enviada (breaker=" << state_name(m_breaker->get_state()) << ")");
    }else{
        m_rejected++;
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                        << " REJEITADA (breaker=" << state_name(m_breaker->get_state()) << ")");
    }

    m_send_event = Simulator::Schedule(m_request_interval, &ClientDevice::send_request, this);
}

void ClientDevice::handle_response(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))){
        if (!m_waiting){
            continue; // chegou depois do prazo: ja contabilizada como falha
        }

        m_waiting = false;
        Simulator::Cancel(m_timeout_event);
        m_ok++;
        m_breaker->register_result(false);

        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                        << " resposta OK (breaker=" << state_name(m_breaker->get_state()) << ")");
    }
}

void ClientDevice::on_timeout(){
    m_waiting = false;
    m_failed++;
    m_breaker->register_result(true);

    NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                    << " TIMEOUT (breaker=" << state_name(m_breaker->get_state()) << ")");
}

void ClientDevice::set_total_requests(int total_requests){
    m_total_requests = total_requests;
}

int ClientDevice::get_success_count() const{
    return m_ok;
}

int ClientDevice::get_timeout_count() const{
    return m_failed;
}

int ClientDevice::get_rejected_count() const{
    return m_rejected;
}

} // namespace ns3