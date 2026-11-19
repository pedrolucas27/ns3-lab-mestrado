#include "models/circuit-breaker.h"
#include "models/client-device.h"
#include "models/common-device.h"

#include "helpers/simulation-helper.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace ns3;

namespace {

constexpr int kPort = 9;

constexpr const char *kResultHeader =
  "run_id,"
  "success_count,failure_count,rejected_by_breaker,"
  "energy_final,energy_min";

struct RunMetrics {
  int    success_count       = 0;
  int    failure_count       = 0;
  int    rejected_by_breaker = 0;
  double energy_final        = 0.0;
  double energy_min          = 0.0;
};

// Instancia o cenário e executa o experimento, retornando as métricas.
RunMetrics run_one(const sim::RunConfig &run){
  RunMetrics m;

  RngSeedManager::SetSeed(run.run_id);   // reproduzivel pelo run_id

  {
    NodeContainer nodes;
    nodes.Create (2); // 0 = cliente, 1 = servidor

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
    NetDeviceContainer devices = p2p.Install (nodes);

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    Ptr<CommonDevice> server = Create<CommonDevice>(
        nodes.Get (1),
        kPort,
        run.server.initial_energy,
        run.server.capacity,
        run.server.active_cost,
        run.server.idle_cost,
        Seconds(run.server.harvest_interval),
        run.server.harvest_energy,
        Seconds(run.server.idle_interval));

    Ptr<CircuitBreaker> breaker = Create<CircuitBreaker>(
        Seconds(run.breaker.open_state_duration),
        Seconds(run.breaker.window_duration),
        run.breaker.failure_rate_threshold,
        run.breaker.minimum_number_of_calls,
        run.breaker.permitted_calls_in_half_open);

    Ptr<ClientDevice> client = Create<ClientDevice>(
        nodes.Get (0),
        InetSocketAddress(interfaces.GetAddress (1), kPort),
        Seconds(run.client.request_interval),
        breaker);
    client->set_total_requests(run.client.total_requests);
    client->set_server(server);

    Simulator::Schedule(Seconds (0.0), &CommonDevice::start, server);
    Simulator::Schedule(Seconds (1.0), &ClientDevice::start, client);

    Simulator::Run();

    m.success_count       = client->get_success_count();
    m.failure_count       = client->get_failure_count();
    m.rejected_by_breaker = client->get_rejected_count();
    m.energy_final        = server->get_energy_amount();
    m.energy_min          = server->get_min_energy();

    client->stop();
    server->stop();
  } 

  Simulator::Destroy();

  return m;
}

void append_result(const std::string &path, const sim::RunConfig &r, const RunMetrics &m){
  std::ofstream out (path, std::ios::app);
  out << r.run_id
      << ',' << m.success_count
      << ',' << m.failure_count
      << ',' << m.rejected_by_breaker
      << ',' << m.energy_final
      << ',' << m.energy_min << '\n';
}

void write_header(const std::string &path){
  std::ofstream head (path, std::ios::trunc);
  head << kResultHeader << '\n';
}

} // namespace

int main (int argc, char *argv[]){
  std::string config_csv = "../experiments/circuit-breaker/datasets/experiment.csv";
  std::string out_file   = "../experiments/circuit-breaker/datasets/resultado.csv";

  CommandLine cmd;
  cmd.AddValue ("config", "CSV com um experimento por linha", config_csv);
  cmd.AddValue ("out", "CSV unico de resultados", out_file);
  cmd.Parse (argc, argv);

  // Carrega as configurações dos experimentos
  std::vector<sim::RunConfig> runs;
  try{
    runs = sim::load_experiments(config_csv);
  }catch (const std::exception &e){
    std::cerr << "ERRO ao ler " << config_csv << ": " << e.what () << '\n';
    return 1;
  }

  std::cout << config_csv << ": " << runs.size () << " experimentos\n";

  // Cria um csv inicial para armazenar os resultados
  write_header(out_file);

  // Executa cada experimento e anexa o resultado no CSV
  size_t done = 0;
  for (const auto &run : runs){
    std::cout << '[' << ++done << '/' << runs.size () << "] run "<< run.run_id << std::endl;
    append_result(out_file, run, run_one(run));
  }

  std::cout << "concluido: " << out_file << '\n';
  return 0;
}
