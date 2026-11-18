#ifndef CIRCUIT_BREAKER_H
#define CIRCUIT_BREAKER_H

#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"

#include <string>

namespace ns3 {

// Estados do circuito:
// CLOSED    - operação normal, requisições passam e falhas são contadas
// OPEN      - circuito aberto, requisições são rejeitadas sem chegar ao servidor
// HALF_OPEN - sondagem, libera uma cota limitada para testar se o serviço voltou
enum class CircuitBreakerState {
    CLOSED,
    OPEN,
    HALF_OPEN
};

std::string state_name(CircuitBreakerState state);

class CircuitBreaker : public SimpleRefCount<CircuitBreaker> {
public:
    CircuitBreaker(Time _open_state_duration, Time _window_duration, double _failure_rate_threshold, int _minimum_number_of_calls, int _permitted_calls_in_half_open);

    // Retorna o estado atual do circuito
    CircuitBreakerState get_state();

    // Contabiliza o resultado de uma requisição e checa se é necessário mudar de estado
    void register_result(bool failed);

    // Informa se a requisição pode passar. Em OPEN com o tempo cumprido, transiciona para HALF_OPEN e libera a sondagem
    bool allow_request();

    // Ponto de entrada: consulta allow_request, executa a requisição e registra o resultado. Retorna false se o circuito rejeitou
    bool intercept();

    // Zera os contadores e reinicia a contagem da janela de tempo
    void reset_window();

    // Verifica se a janela de contagem de falhas já venceu
    bool is_window_expired();

    // Verifica se o tempo de espera em OPEN já foi cumprido
    bool is_open_state_expired();

    // Transições de estado. open() grava o momento da abertura,
    // close() e half_open() preparam os contadores do estado de destino
    void open();
    void close();
    void half_open();

private:
    CircuitBreakerState state;              // Estado do circuito

    Time open_state_duration;               // Quanto tempo o circuito permanece aberto
    Time window_duration;                   // Janela de tempo para contar falhas
    Time window_start_time;                 // Momento em que a janela de tempo começou
    Time open_at;                           // Momento em que o circuito foi aberto

    int total_calls;                        // Contador de chamadas totais
    int failed_calls;                       // Contador de chamadas com falha
    double failure_rate_threshold;          // Taxa de falha maxima que manterá o circuito fechado (fração: 0.5 == 50%)
    int minimum_number_of_calls;            // Número mínimo de chamadas para que a taxa de falha seja considerada
    int permitted_calls_in_half_open;       // Número de chamadas permitidas no estado HALF_OPEN

};

} // namespace ns3

#endif // CIRCUIT_BREAKER_H