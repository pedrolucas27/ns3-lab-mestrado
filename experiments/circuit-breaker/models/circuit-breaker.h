#ifndef CIRCUIT_BREAKER_H
#define CIRCUIT_BREAKER_H

#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"

#include <string>

namespace ns3 {

enum class CircuitBreakerState
{
    CLOSED,
    OPEN,
    HALF_OPEN
};

std::string state_name(CircuitBreakerState state); // Converte o estado para string

class CircuitBreaker : public SimpleRefCount<CircuitBreaker>
{
  public:
    CircuitBreaker(Time open_state_duration,
                   Time window_duration,
                   double failure_rate_threshold,
                   int minimum_number_of_calls,
                   int permitted_calls_in_half_open);

    CircuitBreakerState get_state() const; // Estado atual.
    bool allow_request();                  // True se a chamada pode ser enviada.
    void register_result(bool failed);     // Contabiliza sucesso/falha e aplica transições.

  private:
    void reset_window();          // Zera contadores e reinicia a janela de amostragem.
    bool is_window_expired();     // True se a janela de amostragem já expirou.
    bool is_open_state_expired(); // True se o tempo mínimo em OPEN já passou.
    void open();                  // Vai para OPEN e marca o instante da abertura.
    void close();                 // Vai para CLOSED e zera a janela.
    void half_open();             // Vai para HALF_OPEN e zera os contadores de sondagem.

    CircuitBreakerState state;        // Estado atual do circuito.
    Time open_state_duration;         // Tempo que o circuito fica em OPEN.
    Time window_duration;             // Tamanho da janela usada para contabilizar falhas.
    Time window_start_time;           // Início da janela corrente.
    Time open_at;                     // Instante em que o circuito entrou em OPEN.
    int total_calls;                  // Chamadas concluídas na janela CLOSED ou na sondagem HALF_OPEN.
    int failed_calls;                 // Falhas na janela CLOSED ou na sondagem HALF_OPEN.
    double failure_rate_threshold;    // Limite de taxa de falha (fração).
    int minimum_number_of_calls;      // Amostras mínimas em CLOSED para abrir.
    int permitted_calls_in_half_open; // Chamadas liberadas em HALF_OPEN.
};

} // namespace ns3

#endif // CIRCUIT_BREAKER_H
