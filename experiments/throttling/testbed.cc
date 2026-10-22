#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "models/common-device.h"
#include "helpers/simulation-helper.h"
#include "models/simulation-config.h"

#include <iostream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EnergyAwareTestbed");

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
    LogComponentEnable("EnergyAwareTestbed", LOG_LEVEL_INFO);
    LogComponentEnable("CommonDevice", LOG_LEVEL_INFO);
    LogComponentEnable("ContextBasedEnergyThrottling", LOG_LEVEL_INFO);

    std::string csvPath = (argc > 1)
        ? argv[1]
        : "../experiments/throttling/datasets/energy_aware_network_simulation_dataset.csv";
    std::vector<SimulationConfig> simulations = load_simulation_configs(csvPath);

    std::ofstream metricsOutput("../experiments/throttling/testbed-metrics.csv");
    if (!metricsOutput.is_open()) {
        std::cerr << "Error: could not open ../experiments/throttling/testbed-metrics.csv\n";
        return 1;
    }
    writeMetricsHeader(metricsOutput);

    for (const SimulationConfig& config : simulations) {
        NS_LOG_INFO("Iniciando " << config.simulation_id
                    << " com " << config.num_nodes << " dispositivos e "
                    << config.num_clients << " clientes");
        runSimulation(config, metricsOutput);
    }

    return 0;
}
