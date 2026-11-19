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

class ClientDevice : public SimpleRefCount<ClientDevice>
{
  public:
    ClientDevice(Ptr<Node> node,
                 Address server_address,
                 Time request_interval,
                 int total_requests,
                 Ptr<CircuitBreaker> breaker,
                 Ptr<CommonDevice> server);

    void start();               // Abre o socket, agenda o envio e dispara a primeira tentativa.
    void stop();                // Cancela o envio periódico e fecha o socket.
    int get_success_count();    // Respostas com status OK.
    int get_failure_count();    // Respostas com status de falha.
    int get_rejected_count();   // Tentativas bloqueadas pelo circuito.

  private:
    void send_request();                      // Trata as tentativas de requisição: envia ou rejeita, e reagenda.
    void handle_response(Ptr<Socket> socket); // Lê respostas e registra sucesso/falha no breaker.
    void try_finish();                        // Encerra cliente e servidor quando o orçamento acaba e nada está em pendente.
    bool budget_exhausted() const;            // True se enviadas + rejeitadas já cobrem o orçamento.

    Ptr<Node> m_node;                 // Nó ns-3 do cliente.
    Ptr<Socket> m_socket;             // Socket do servidor.
    Address m_server_address;         // Endereço do servidor.
    Time m_request_interval;          // Intervalo entre tentativas.
    int m_total_requests;             // Orçamento de tentativas (enviadas + rejeitadas).
    Ptr<CircuitBreaker> m_breaker;    // Mecanismo de segurança do cliente.
    Ptr<CommonDevice> m_server;       // Servidor
    EventId m_send_event;             // Evento da próxima tentativa
    int m_inflight;                   // Pacotes já enviados cuja resposta ainda não chegou (UDP assíncrono)
    int m_sent;                       // Chamadas efetivamente enviadas
    int m_ok;                         // Respostas OK
    int m_failed;                     // Respostas de falha
    int m_rejected;                   // Chamadas barradas pelo circuito
    bool m_finished;                  // True depois de encerrar o cenário
};

} // namespace ns3

#endif // CLIENT_DEVICE_H
