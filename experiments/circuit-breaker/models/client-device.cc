#include "client-device.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE("ClientDevice");

namespace {
constexpr uint8_t kResponseOk = 1;
}

ClientDevice::ClientDevice(Ptr<Node> node,
                           Address server_address,
                           Time request_interval,
                           Ptr<CircuitBreaker> breaker)
    : m_node(node),
      m_server_address(server_address),
      m_request_interval(request_interval),
      m_breaker(breaker),
      m_total_requests(0),
      m_inflight(0),
      m_sent(0),
      m_ok(0),
      m_failed(0),
      m_rejected(0),
      m_finished(false)
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
    if (m_socket){
        m_socket->Close();
        m_socket = nullptr;
    }
}

void ClientDevice::set_server(Ptr<CommonDevice> server){
    m_server = server;
}

bool ClientDevice::budget_exhausted() const{
    return m_total_requests > 0 && (m_sent + m_rejected) >= m_total_requests;
}

void ClientDevice::try_finish(){
    if (m_finished || !budget_exhausted() || m_inflight > 0){
        return;
    }

    m_finished = true;
    Simulator::Cancel(m_send_event);
    if (m_server){
        m_server->stop();
    }
}

void ClientDevice::send_request(){
    if (budget_exhausted()){
        try_finish();
        return;
    }

    if (m_breaker->allow_request()){
        m_socket->Send(Create<Packet>(4));
        m_sent++;
        m_inflight++;

        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                        << " enviada (breaker=" << state_name(m_breaker->get_state()) << ")");
    }else{
        m_rejected++;
        NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                        << " REJEITADA (breaker=" << state_name(m_breaker->get_state()) << ")");
    }

    if (!budget_exhausted()){
        m_send_event = Simulator::Schedule(m_request_interval, &ClientDevice::send_request, this);
    }else{
        try_finish();
    }
}

void ClientDevice::handle_response(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))){
        if (m_inflight <= 0){
            continue;
        }

        m_inflight--;

        uint8_t status = 0;
        packet->CopyData(&status, 1);

        if (status == kResponseOk){
            m_ok++;
            m_breaker->register_result(false);
            NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                            << " resposta OK (breaker=" << state_name(m_breaker->get_state()) << ")");
        }else{
            m_failed++;
            m_breaker->register_result(true);
            NS_LOG_INFO("[" << Simulator::Now().GetSeconds() << "s] no " << m_node->GetId()
                            << " FALHA ENERGIA (breaker=" << state_name(m_breaker->get_state()) << ")");
        }
    }

    try_finish();
}

void ClientDevice::set_total_requests(int total_requests){
    m_total_requests = total_requests;
}

int ClientDevice::get_success_count() const{
    return m_ok;
}

int ClientDevice::get_failure_count() const{
    return m_failed;
}

int ClientDevice::get_rejected_count() const{
    return m_rejected;
}

} // namespace ns3
