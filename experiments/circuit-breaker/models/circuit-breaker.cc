#include "circuit-breaker.h"
#include "ns3/simulator.h"

namespace ns3
{

    std::string state_name(CircuitBreakerState state)
    {
        switch (state)
        {
        case CircuitBreakerState::CLOSED:
            return "CLOSED";
        case CircuitBreakerState::OPEN:
            return "OPEN";
        case CircuitBreakerState::HALF_OPEN:
            return "HALF_OPEN";
        }
        return "UNKNOWN";
    }

    CircuitBreaker::CircuitBreaker(Time _open_state_duration,
                                   Time _window_duration,
                                   double _failure_rate_threshold,
                                   int _minimum_number_of_calls,
                                   int _permitted_calls_in_half_open)
        : state(CircuitBreakerState::CLOSED),
          open_state_duration(_open_state_duration),
          window_duration(_window_duration),
          open_at(Seconds(0)),
          total_calls(0),
          failed_calls(0),
          failure_rate_threshold(_failure_rate_threshold),
          minimum_number_of_calls(_minimum_number_of_calls),
          permitted_calls_in_half_open(_permitted_calls_in_half_open)
    {
        window_start_time = Simulator::Now();
    }

    CircuitBreakerState CircuitBreaker::get_state()
    {
        return state;
    }

    void CircuitBreaker::reset_window()
    {
        total_calls = 0;
        failed_calls = 0;
        window_start_time = Simulator::Now();
    }

    bool CircuitBreaker::is_window_expired()
    {
        return (Simulator::Now() - window_start_time) >= window_duration;
    }

    bool CircuitBreaker::is_open_state_expired()
    {
        return (Simulator::Now() - open_at) >= open_state_duration;
    }

    void CircuitBreaker::open()
    {
        state = CircuitBreakerState::OPEN;
        open_at = Simulator::Now();
    }

    void CircuitBreaker::close()
    {
        state = CircuitBreakerState::CLOSED;
        reset_window();
    }

    void CircuitBreaker::half_open()
    {
        state = CircuitBreakerState::HALF_OPEN;
        total_calls = 0;
        failed_calls = 0;
    }

    void CircuitBreaker::register_result(bool failed)
    {
        if (state == CircuitBreakerState::CLOSED && is_window_expired())
        {
            reset_window();
        }

        total_calls++;
        if (failed)
            failed_calls++;

        double failure_rate = static_cast<double>(failed_calls) / total_calls;

        if (state == CircuitBreakerState::HALF_OPEN)
        {
            if (total_calls >= permitted_calls_in_half_open)
            {
                if (failure_rate > failure_rate_threshold)
                    open();
                else
                    close();
            }
            return;
        }

        if (state == CircuitBreakerState::OPEN)
            return;

        if (total_calls >= minimum_number_of_calls && failure_rate > failure_rate_threshold)
        {
            open();
        }
    }

    bool CircuitBreaker::allow_request()
    {
        switch (state)
        {
        case CircuitBreakerState::CLOSED:
            return true;
        case CircuitBreakerState::OPEN:
            if (is_open_state_expired())
            {
                half_open();
                return true;
            }
            return false;
        case CircuitBreakerState::HALF_OPEN:
            return total_calls < permitted_calls_in_half_open;
        }
        return false;
    }

    bool CircuitBreaker::intercept()
    {
        bool request_allowed = allow_request();
        if (!request_allowed)
            return false;

        try
        {
            // request(); - método que repassa a request para o server processar
        }
        catch (...)
        {
            register_result(true); // chama o registrador de resultado com falha = true
            throw;
        }

        register_result(false); // chama o registrador de resultado com falha = false
        return true;
    }

} // namespace ns3