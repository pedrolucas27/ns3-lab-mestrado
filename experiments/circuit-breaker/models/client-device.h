#ifndef CLIENT_DEVICE_H
#define CLIENT_DEVICE_H

#include "circuit-breaker.h"
#include "common-device.h"

#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "ns3/socket.h"

namespace ns3 {

/**
 * Cliente UDP periodico protegido por um circuit breaker.
 *
 * Cada tentativa prevista no experimento e disparada. Falha so ocorre quando o
 * servidor responde que nao tem energia para processar (custo ativo maior que
 * o buffer). Rejeicoes do breaker nao chegam ao servidor.
 *
 * A simulacao nao usa tempo fixo: ao esgotar as tentativas e as respostas em
 * voo, o cliente encerra o harvest/idle do servidor para Run() retornar.
 */
class ClientDevice : public SimpleRefCount<ClientDevice>
{
  public:
    ClientDevice(Ptr<Node> node,
                 Address server_address,
                 Time request_interval,
                 Ptr<CircuitBreaker> breaker);

    void start();
    void stop();

    void set_total_requests(int tot);
    void set_server(Ptr<CommonDevice> server);

    int get_success_count() const;
    int get_failure_count() const;
    int get_rejected_count() const;

  private:
    void send_request();
    void handle_response(Ptr<Socket> socket);
    void try_finish();
    bool budget_exhausted() const;

    Ptr<Node> m_node;
    Ptr<Socket> m_socket;
    Address m_server_address;
    Time m_request_interval;
    Ptr<CircuitBreaker> m_breaker;
    Ptr<CommonDevice> m_server;

    int m_total_requests;
    int m_inflight;

    EventId m_send_event;

    int m_sent;
    int m_ok;
    int m_failed;
    int m_rejected;
    bool m_finished;
};

} // namespace ns3

#endif // CLIENT_DEVICE_H
