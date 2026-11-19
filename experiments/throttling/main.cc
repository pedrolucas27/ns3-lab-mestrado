#include "helpers/simulation-helper.h"
#include "models/common-device.h"
#include "models/simulation-config.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EnergyAwareThrottling");

namespace {

void runSimulation(const SimulationConfig& config, std::ofstream& metricsOutput) {
    // Cada linha do CSV gera uma topologia e uma configuração energética própria.
    SimulationTopology topology = createTopology(config);
    std::vector<Ptr<CommonDevice>> energyDevices = createEnergyDevices(topology.devices, config);

    createDeviceSockets(topology.devices, energyDevices);
    scheduleClientTraffic(topology.clients, topology.interfaces, config);

    // Executa até a última requisição, com margem para o failover terminar.
    Simulator::Stop(getSimulationStopTime(config));
    Simulator::Run();

    AggregatedMetrics metrics = aggregateMetrics(energyDevices);
    writeMetricsRow(metricsOutput, config, metrics);

    Simulator::Destroy();
}

} // namespace

int main(int argc, char* argv[]) {
    LogComponentEnable("EnergyAwareThrottling", LOG_LEVEL_INFO);
    LogComponentEnable("CommonDevice", LOG_LEVEL_INFO);
    LogComponentEnable("ContextBasedEnergyThrottling", LOG_LEVEL_INFO);

    std::string csvPath = "../experiments/throttling/datasets/energy_aware_network_simulation_dataset.csv";
    std::string outFile = "../experiments/throttling/main-metrics.csv";

    CommandLine cmd;
    cmd.AddValue("config", "CSV com um experimento por linha", csvPath);
    cmd.AddValue("out", "CSV de resultados", outFile);
    cmd.Parse(argc, argv);

    std::vector<SimulationConfig> simulations;
    try {
        simulations = load_simulation_configs(csvPath);
    } catch (const std::exception& e) {
        std::cerr << "ERRO ao ler " << csvPath << ": " << e.what() << '\n';
        return 1;
    }

    std::ofstream metricsOutput(outFile);
    if (!metricsOutput.is_open()) {
        std::cerr << "ERRO: nao foi possivel abrir " << outFile << '\n';
        return 1;
    }
    writeMetricsHeader(metricsOutput);

    std::cout << csvPath << ": " << simulations.size() << " experimentos\n";

    for (const SimulationConfig& config : simulations) {
        NS_LOG_INFO("Iniciando " << config.simulation_id
                    << " com " << config.num_nodes << " dispositivos e "
                    << config.num_clients << " clientes");
        runSimulation(config, metricsOutput);
    }

    std::cout << "concluido: " << outFile << '\n';
    return 0;
}
