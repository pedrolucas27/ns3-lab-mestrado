#ifndef CLIENT_DEVICE_H
#define CLIENT_DEVICE_H

#include "circuit-breaker.h"

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
 * Mantem no maximo uma requisicao em voo (exige request_interval >
 * response_timeout). O sinal de falha e o silencio: servidor sem energia nao
 * responde, o timeout vence e vira register_result(true).
 */
class ClientDevice : public SimpleRefCount<ClientDevice>
{
  public:
    ClientDevice(Ptr<Node> node,
                 Address server_address,
                 Time request_interval,
                 Time response_timeout,
                 Ptr<CircuitBreaker> breaker);

    void start();
    
    void stop();

    void set_total_requests(int tot);

    int get_success_count() const;
    int get_timeout_count() const;
    int get_rejected_count() const;

  private:
    void send_request();                      // ciclo periodico; consulta o breaker

    void handle_response(Ptr<Socket> socket); // sucesso

    void on_timeout();                        // falha

    Ptr<Node> m_node;
    Ptr<Socket> m_socket;
    Address m_server_address;
    Time m_request_interval;
    Time m_response_timeout;
    Ptr<CircuitBreaker> m_breaker;

    int m_total_requests;

    EventId m_send_event;    // proximo envio
    EventId m_timeout_event; // prazo da requisicao em voo
    bool m_waiting;          // ha requisicao aguardando resposta?

    int m_sent;
    int m_ok;
    int m_failed;
    int m_rejected;
};

} // namespace ns3

#endif // CLIENT_DEVICE_H