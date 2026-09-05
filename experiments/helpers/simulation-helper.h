#ifndef SIMULATION_HELPER_H
#define SIMULATION_HELPER_H

#include "ns3/ipv4-interface-container.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/core-module.h"

#include <cstdint>
#include <utility>
#include <vector>
#include <iostream>
#include <random>

const int SERVER_PORT = 4000;
const double SERVER_SELECTION_THRESHOLD = 0.7;

namespace ns3 {

double generateRandomProbability(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen);
}

Ipv4Address getTargetServer(Ipv4InterfaceContainer interfaces) {
    double selection_probability = generateRandomProbability();
    if (selection_probability < SERVER_SELECTION_THRESHOLD) {
        return interfaces.GetAddress(1);
    }else{
        return interfaces.GetAddress(2);
    }
}

void sendPacket(Ptr<Socket> socket, Ipv4InterfaceContainer interfaces, Time interval) {
    Ipv4Address target_server = getTargetServer(interfaces);

    socket->Connect(InetSocketAddress(target_server, SERVER_PORT));

    Ptr<Packet> packet = Create<Packet>(64);
    socket->Send(packet);
    Simulator::Schedule(interval, &sendPacket, socket, interfaces, interval);
}

} // namespace ns3

#endif // SIMULATION_HELPER_H