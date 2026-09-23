#ifndef SIMULATION_CONFIG_HPP
#define SIMULATION_CONFIG_HPP

#include <string>
#include <vector>

struct SimulationConfig {
    std::string simulation_id;

    int    num_nodes;

    double energy_buffer_capacity;   
    double initial_energy;           
    double node_active_cost;         
    double transmission_cost;        
    double node_idle_cost;           

    double harvest_interval_s;       
    double harvest_energy;           

    int    num_clients;
    int    max_hops;

    double request_interval_s;       
    int    requests_per_client;
};

std::vector<SimulationConfig> load_simulation_configs(const std::string& csvPath);

#endif // SIMULATION_CONFIG_HPP