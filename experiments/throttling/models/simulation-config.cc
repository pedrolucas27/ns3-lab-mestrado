#include "simulation-config.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;

    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

constexpr int EXPECTED_COLUMNS = 13;

} // namespace

std::vector<SimulationConfig> load_simulation_configs(const std::string& csvPath) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + csvPath);
    }

    std::vector<SimulationConfig> configs;

    std::string line;
    std::getline(file, line); // header row, e.g.
    
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> tokens = split_csv_line(line);
        if (static_cast<int>(tokens.size()) != EXPECTED_COLUMNS) {
            throw std::runtime_error("Malformed CSV line (expected " +
                                      std::to_string(EXPECTED_COLUMNS) +
                                      " columns, got " +
                                      std::to_string(tokens.size()) +
                                      "): " + line);
        }

        SimulationConfig cfg;
        cfg.simulation_id          = tokens[0];
        cfg.num_nodes              = std::stoi(tokens[1]);
        cfg.energy_buffer_capacity = std::stod(tokens[2]);
        cfg.initial_energy         = std::stod(tokens[3]);
        cfg.node_active_cost       = std::stod(tokens[4]);
        cfg.transmission_cost      = std::stod(tokens[5]);
        cfg.node_idle_cost         = std::stod(tokens[6]);
        cfg.harvest_interval_s     = std::stod(tokens[7]);
        cfg.harvest_energy         = std::stod(tokens[8]);
        cfg.num_clients            = std::stoi(tokens[9]);
        cfg.max_hops               = std::stoi(tokens[10]);
        cfg.request_interval_s     = std::stod(tokens[11]);
        cfg.requests_per_client    = std::stoi(tokens[12]);

        configs.push_back(cfg);
    }

    return configs;
}