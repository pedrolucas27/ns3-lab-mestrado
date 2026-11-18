#ifndef SIMULATION_HELPER_H
#define SIMULATION_HELPER_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace sim {

struct ServerConfig
{
  double   initial_energy;     // energia inicial do dispositivo
  double   capacity;           // capacidade energetica maxima
  double   active_cost;        // custo de processamento de uma request
  double   idle_cost;          // custo por idle_interval em modo ocioso
  double   idle_interval;      // intervalo de desconto de energia ociosa (s)
  double   harvest_interval;   // intervalo entre coletas de energia (s)
  double   harvest_energy;     // energia coletada a cada intervalo
};

struct BreakerConfig
{
  double    open_state_duration;           // tempo em OPEN antes do HALF_OPEN (s)
  double    window_duration;               // janela de contagem de falhas (s)
  double    failure_rate_threshold;        // fracao: 0.5 == 50%
  int       minimum_number_of_calls;       // minimo de chamadas para avaliar
  int       permitted_calls_in_half_open;  // sondagens permitidas em HALF_OPEN
};

struct ClientConfig
{
  double    request_interval;   // intervalo entre requests (s)
  double    response_timeout;   // timeout de resposta (s)
  int       total_requests;     // numero total de requests
};

struct RunConfig
{
  int      run_id = 0;   // posicao da linha no arquivo (1-based)
  ServerConfig  server {};
  BreakerConfig breaker {};
  ClientConfig  client {};
};

// Assume CSV bem formado: sem espacos em volta dos campos e sem linhas vazias.
std::vector<std::string> split_line (const std::string &line){
  std::vector<std::string> out;
  std::string field;
  std::istringstream is (line);
  while (std::getline (is, field, ',')) { out.push_back (field); }
  return out;
}

// Carrega os experimentos, o acesso e por nome de coluna.
std::vector<RunConfig> load_experiments (const std::string &path){
  std::ifstream in (path);
  if (!in) { throw std::runtime_error ("nao foi possivel abrir: " + path); }

  std::map<std::string, size_t> header;
  std::vector<RunConfig> runs;
  std::string line;

  while (std::getline (in, line)){
      const auto c = split_line (line);

      if (header.empty ()){
        for (size_t i = 0; i < c.size (); ++i) { header[c[i]] = i; }
        continue;
      }

      auto cell = [&] (const std::string &name) -> const std::string &
      {
        const auto it = header.find (name);
        if (it == header.end ())
          { throw std::runtime_error (path + ": coluna ausente: " + name); }
        return c.at (it->second);
      };
      auto num = [&] (const std::string &n) { return std::stod (cell (n)); };
      auto u32 = [&] (const std::string &n)
      { return static_cast<int> (std::stoul (cell (n))); };

      RunConfig r;
      r.run_id  = u32 ("run_id");
      r.server  = { num ("initial_energy"),   num ("capacity"),
                    num ("active_cost"),      num ("idle_cost"),
                    num ("idle_interval"),    num ("harvest_interval"),
                    num ("harvest_energy") };
      r.breaker = { num ("open_state_duration"),
                    num ("window_duration"),
                    num ("failure_rate_threshold"),
                    u32 ("minimum_number_of_calls"),
                    u32 ("permitted_calls_in_half_open") };
      r.client  = { num ("request_interval"),
                    num ("response_timeout"),
                    u32 ("total_requests") };
      runs.push_back(r);
    }

  if (header.empty ()){
    throw std::runtime_error (path + ": arquivo vazio");
  }

  return runs;
}

// Escreve a configuracao de um experimento, na mesma ordem do cabecalho
// abaixo. Usado para montar o CSV de resultados.
inline const char *kConfigHeader =
  "run_id,"
  "initial_energy,capacity,active_cost,idle_cost,idle_interval,"
  "harvest_interval,harvest_energy,"
  "open_state_duration,window_duration,failure_rate_threshold,"
  "minimum_number_of_calls,permitted_calls_in_half_open,"
  "request_interval,response_timeout,total_requests";

inline void write_config_row (std::ostream &out, const RunConfig &r)
{
  out << r.run_id << ','
      << r.server.initial_energy << ',' << r.server.capacity << ','
      << r.server.active_cost << ',' << r.server.idle_cost << ','
      << r.server.idle_interval << ',' << r.server.harvest_interval << ','
      << r.server.harvest_energy << ','
      << r.breaker.open_state_duration << ',' << r.breaker.window_duration << ','
      << r.breaker.failure_rate_threshold << ','
      << r.breaker.minimum_number_of_calls << ','
      << r.breaker.permitted_calls_in_half_open << ','
      << r.client.request_interval << ',' << r.client.response_timeout << ','
      << r.client.total_requests;
}

} // namespace sim

#endif // SIMULATION_HELPER_H