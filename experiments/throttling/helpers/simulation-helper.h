#ifndef SIMULATION_HELPER_H
#define SIMULATION_HELPER_H

#include "ns3/ipv4-interface-container.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/core-module.h"

#include "../models/common-device.h"
#include "../models/router-tag.h"
#include "../models/simulation-config.h"

#include <cstdint>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include <string>

const int SERVER_PORT = 4000;
const double SERVER_SELECTION_THRESHOLD = 0.7;

namespace ns3 {

struct SimulationTopology {
    NodeContainer nodes;
    NodeContainer clients;
    NodeContainer devices;
    Ipv4InterfaceContainer interfaces;
};

struct AggregatedMetrics {
    uint64_t received = 0;
    uint64_t accepted = 0;
    uint64_t rejected = 0;
    uint64_t received_forwarded = 0;
    uint64_t accepted_forwarded = 0;
    uint64_t rejected_forwarded = 0;
    uint64_t sent_forwarded = 0;
    double total_energy_remaining = 0.0;
    double minimum_energy_remaining = 0.0;
};

inline Time getSimulationStopTime(const SimulationConfig& config) {
    const double firstRequestTime = 1.0;
    const double lastRequestTime = firstRequestTime +
        (config.requests_per_client - 1) * config.request_interval_s;
    const double jitterMargin = config.request_interval_s;   // pior caso do sorteio
    const double forwardingMargin = (config.max_hops + 1) * 0.01;
    return Seconds(lastRequestTime + jitterMargin + forwardingMargin);
}

inline AggregatedMetrics aggregateMetrics(
    const std::vector<Ptr<CommonDevice>>& devices) {
    AggregatedMetrics total;
    if (devices.empty()) {
        return total;
    }

    total.minimum_energy_remaining = devices.front()->getEnergyAmount();
    for (const Ptr<CommonDevice>& device : devices) {
        EnergyMetrics metrics = device->getMetrics();
        total.received += metrics.received;
        total.accepted += metrics.accepted;
        total.rejected += metrics.rejected;
        total.received_forwarded += metrics.received_forwarded;
        total.accepted_forwarded += metrics.accepted_forwarded;
        total.rejected_forwarded += metrics.rejected_forwarded;
        total.sent_forwarded += metrics.sent_forwarded;

        double remainingEnergy = device->getEnergyAmount();
        total.total_energy_remaining += remainingEnergy;
        total.minimum_energy_remaining = std::min(
            total.minimum_energy_remaining, remainingEnergy);
    }
    return total;
}

inline void writeMetricsHeader(std::ofstream& output) {
    output << "simulation_id,num_nodes,num_clients,max_hops,"
           << "total_received,total_accepted,total_rejected,"
           << "received_forwarded,accepted_forwarded,rejected_forwarded,"
           << "sent_forwarded,total_energy_remaining,minimum_energy_remaining\n";
}

inline void writeMetricsRow(std::ofstream& output,
                            const SimulationConfig& config,
                            const AggregatedMetrics& metrics) {
    output << config.simulation_id << ','
           << config.num_nodes << ','
           << config.num_clients << ','
           << config.max_hops << ','
           << metrics.received << ','
           << metrics.accepted << ','
           << metrics.rejected << ','
           << metrics.received_forwarded << ','
           << metrics.accepted_forwarded << ','
           << metrics.rejected_forwarded << ','
           << metrics.sent_forwarded << ','
           << metrics.total_energy_remaining << ','
           << metrics.minimum_energy_remaining << '\n';
}

// Cria a topologia parametrizada pelo CSV e separa clientes dos dispositivos.
inline SimulationTopology createTopology(const SimulationConfig& config) {
    SimulationTopology topology;
    topology.nodes.Create(config.num_clients + config.num_nodes);

    for (int index = 0; index < config.num_clients; ++index) {
        topology.clients.Add(topology.nodes.Get(index));
    }
    for (int index = 0; index < config.num_nodes; ++index) {
        topology.devices.Add(topology.nodes.Get(config.num_clients + index));
    }

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));
    NetDeviceContainer netDevices = csma.Install(topology.nodes);

    InternetStackHelper stack;
    stack.Install(topology.nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    topology.interfaces = address.Assign(netDevices);
    return topology;
}

// Cria os dispositivos energéticos; cada um conhece todos os outros dispositivos,
// mas nenhum cliente é inserido na lista de vizinhos.
inline std::vector<Ptr<CommonDevice>> createEnergyDevices(
    const NodeContainer& deviceNodes,
    const SimulationConfig& config) {

    std::vector<Ptr<CommonDevice>> devices;
    devices.reserve(deviceNodes.GetN());

    for (uint32_t index = 0; index < deviceNodes.GetN(); ++index) {
        devices.push_back(Create<CommonDevice>(
            deviceNodes.Get(index),

            config.initial_energy,
            config.energy_buffer_capacity,
            config.node_active_cost,
            config.transmission_cost,
            config.node_idle_cost,
            config.harvest_interval_s,
            config.harvest_energy));
    }

    for (const Ptr<CommonDevice>& device : devices) {
        device->setNeighbors(devices);
    }
    
    return devices;
}

// Instala o socket receptor de cada dispositivo e inicia seus ciclos de energia.
inline std::vector<Ptr<Socket>> createDeviceSockets(
    const NodeContainer& deviceNodes,
    const std::vector<Ptr<CommonDevice>>& devices) {
    std::vector<Ptr<Socket>> sockets;
    sockets.reserve(deviceNodes.GetN());

    for (uint32_t index = 0; index < deviceNodes.GetN(); ++index) {
        Ptr<Socket> socket = Socket::CreateSocket(deviceNodes.Get(index), UdpSocketFactory::GetTypeId());
        socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), SERVER_PORT));
        socket->SetRecvCallback(MakeCallback(&CommonDevice::receivePacket, PeekPointer(devices[index])));
        sockets.push_back(socket);
        devices[index]->start();
    }
    return sockets;
}

inline Ptr<UniformRandomVariable> getRng() {
    static Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    return rng;
}

inline Ipv4Address getTargetServer(const Ipv4InterfaceContainer& interfaces,
                                   uint32_t firstDeviceInterface,
                                   uint32_t deviceCount) {
    uint32_t selected = getRng()->GetInteger(0, deviceCount - 1);
    return interfaces.GetAddress(firstDeviceInterface + selected);
}

// Envia uma quantidade limitada de pacotes, conforme requests_per_client.
inline void sendPacket(Ptr<Socket> socket,
                       Ipv4InterfaceContainer interfaces,
                       uint32_t firstDeviceInterface,
                       uint32_t deviceCount,
                       Time interval,
                       uint32_t remainingPackets,
                       uint8_t maxHops) {
    if (remainingPackets == 0 || deviceCount == 0) {
        return;
    }

    Ipv4Address target_server = getTargetServer(interfaces, firstDeviceInterface, deviceCount);

    socket->Connect(InetSocketAddress(target_server, SERVER_PORT));

    Ptr<Packet> packet = Create<Packet>(64);
    RouteTag route;
    route.m_max_hops = maxHops;
    packet->AddPacketTag(route);
    socket->Send(packet);

    if (remainingPackets > 1) {
        Simulator::Schedule(interval,
                            &sendPacket,
                            socket,
                            interfaces,
                            firstDeviceInterface,
                            deviceCount,
                            interval,
                            remainingPackets - 1,
                            maxHops);
    }
}

// Agenda o tráfego de todos os clientes usando os parâmetros da simulação atual.
inline void scheduleClientTraffic(const NodeContainer& clients,
                                  const Ipv4InterfaceContainer& interfaces,
                                  const SimulationConfig& config) {
    uint32_t firstDeviceInterface = config.num_clients;

    Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable>();
    jitter->SetAttribute("Min", DoubleValue(0.0));
    jitter->SetAttribute("Max", DoubleValue(config.request_interval_s));

    for (uint32_t index = 0; index < clients.GetN(); ++index) {
        Ptr<Socket> socket = Socket::CreateSocket(
            clients.Get(index), UdpSocketFactory::GetTypeId());
        socket->Bind();
        Simulator::Schedule(
            Seconds(1.0 + jitter->GetValue()),
            &sendPacket,
            socket,
            interfaces,
            firstDeviceInterface,
            config.num_nodes,
            Seconds(config.request_interval_s),
            config.requests_per_client,
            static_cast<uint8_t>(std::min(config.max_hops, 255)));
    }
}

} // namespace ns3

#endif // SIMULATION_HELPER_H