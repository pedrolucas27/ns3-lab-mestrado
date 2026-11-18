#ifndef SERVICE_UNAVAILABLE_EXCEPTION_H
#define SERVICE_UNAVAILABLE_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace ns3 {

/** Base: o dispositivo nao pode atender, qualquer que seja a causa. */
class ServiceUnavailableException : public std::runtime_error
{
  public:
    explicit ServiceUnavailableException(const std::string& reason)
        : std::runtime_error(reason)
    {
    }
};

/** Falha por esgotamento do buffer de energia; recupera com o harvest. */
class OutOfEnergyException : public ServiceUnavailableException
{
  public:
    OutOfEnergyException()
        : ServiceUnavailableException("Dispositivo sem energia para processar a requisicao")
    {
    }
};

/** Falha abrupta programada pelo cenario; recupera por recover(). */
class CrashedException : public ServiceUnavailableException
{
  public:
    CrashedException()
        : ServiceUnavailableException("Dispositivo em crash")
    {
    }
};

} // namespace ns3

#endif // SERVICE_UNAVAILABLE_EXCEPTION_H