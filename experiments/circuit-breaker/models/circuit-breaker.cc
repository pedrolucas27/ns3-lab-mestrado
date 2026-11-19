#include "circuit-breaker.h"
#include "ns3/simulator.h"

namespace ns3{

    std::string state_name(CircuitBreakerState state){
        switch (state){
            case CircuitBreakerState::CLOSED:
                return "CLOSED";
            case CircuitBreakerState::OPEN:
                return "OPEN";
            case CircuitBreakerState::HALF_OPEN:
                return "HALF_OPEN";
            default:
                return "UNKNOWN";
        }
    }

    CircuitBreaker::CircuitBreaker(Time open_state_duration,
                                   Time window_duration,
                                   double failure_rate_threshold,
                                   int minimum_number_of_calls,
                                   int permitted_calls_in_half_open)
        : state(CircuitBreakerState::CLOSED),
          open_state_duration(open_state_duration),
          window_duration(window_duration),
          open_at(Seconds(0)),
          total_calls(0),
          failed_calls(0),
          failure_rate_threshold(failure_rate_threshold),
          minimum_number_of_calls(minimum_number_of_calls),
          permitted_calls_in_half_open(permitted_calls_in_half_open)
    {
        window_start_time = Simulator::Now();
    }

    CircuitBreakerState CircuitBreaker::get_state() const{
        return state;
    }

    // Zera total_calls e failed_calls para a próxima amostragem (CLOSED ou HALF_OPEN).
    void CircuitBreaker::reset_window(){
        total_calls = 0;
        failed_calls = 0;
        window_start_time = Simulator::Now();
    }

    bool CircuitBreaker::is_window_expired(){
        return (Simulator::Now() - window_start_time) >= window_duration;
    }

    bool CircuitBreaker::is_open_state_expired(){
        return (Simulator::Now() - open_at) >= open_state_duration;
    }

    // Abre o circuito e marca o instante; os contadores só são zerados na próxima entrada em HALF_OPEN ou CLOSED.
    void CircuitBreaker::open(){
        state = CircuitBreakerState::OPEN;
        open_at = Simulator::Now();
    }

    // Volta a operar normalmente e inicia uma janela limpa.
    void CircuitBreaker::close(){
        state = CircuitBreakerState::CLOSED;
        reset_window();
    }

    // Sondagem: mesma base de contagem (total_calls / failed_calls), zerada para não misturar falhas da janela CLOSED anterior.
    void CircuitBreaker::half_open(){
        state = CircuitBreakerState::HALF_OPEN;
        reset_window();
    }

    // Contabiliza uma chamada já concluída (sucesso ou falha) e aplica a transição.
    void CircuitBreaker::register_result(bool failed){
        if (state == CircuitBreakerState::OPEN)
            return;
        
        // Evita a contabilização de chamadas em janelas antigas caso a janela tenha expirado.
        if (state == CircuitBreakerState::CLOSED && is_window_expired()){
            reset_window();
        }

        total_calls++;
        if (failed)
            failed_calls++;

        double failure_rate = static_cast<double>(failed_calls) / total_calls; // Taxa atual de falha.

        if (state == CircuitBreakerState::HALF_OPEN){
            if (total_calls >= permitted_calls_in_half_open){
                if (failure_rate > failure_rate_threshold)
                    open();
                else
                    close();
            }
            return;
        }

        if (total_calls >= minimum_number_of_calls && failure_rate > failure_rate_threshold){
            open();
        }
    }

    // Decide se esta tentativa pode ir a rede. Não registra sucesso/falha.
    bool CircuitBreaker::allow_request(){
        switch (state){
            case CircuitBreakerState::CLOSED:
                return true;
            case CircuitBreakerState::OPEN:
                if (is_open_state_expired()){
                    half_open();
                    return true;
                }
                return false;
            case CircuitBreakerState::HALF_OPEN:
                return total_calls < permitted_calls_in_half_open;
            default:
                return false;
        }
    }

} // namespace ns3
