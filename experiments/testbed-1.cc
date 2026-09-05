#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

#include "models/common-device.h"

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimulatorBase");

void sendPacket(Ptr<Socket> socket, Time interval) {
    Ptr<Packet> packet = Create<Packet>(64);
    socket->Send(packet);
    Simulator::Schedule(interval, &sendPacket, socket, interval);
}

int main(int argc, char* argv[]){

    LogComponentEnable("SimulatorBase", LOG_LEVEL_INFO);
    LogComponentEnable("CommonDevice", LOG_LEVEL_INFO);
    LogComponentEnable("ContextBasedEnergyThrottling", LOG_LEVEL_INFO);

    NS_LOG_INFO("Iniciando simulação...");
    
    // Cria 3 nós: um cliente e dois dispositivos
    NodeContainer nodes;
    nodes.Create(3);

    // Define os nós
    Ptr<Node> client = nodes.Get(0);
    Ptr<Node> device_1 = nodes.Get(1);
    Ptr<Node> device_2 = nodes.Get(2);

    // Define e vincula os dispositivos orientados a energia aos nós
    Ptr<CommonDevice> common_device_1 = Create<CommonDevice>(device_1, 100.0, 100.0);
    Ptr<CommonDevice> common_device_2 = Create<CommonDevice>(device_2, 20.0, 100.0);

    // Adiciona os vizinhos aos dispositivos
    common_device_1->addNeighbor(common_device_2);

    // Cria, configura e instala o canal CSMA entre os nós
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));
    NetDeviceContainer net_devices = csma.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    // Configura a rede
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(net_devices);

    const int PORT = 4000;

    // Sockets nós dispostivos
    Ptr<Socket> server_socket_1 = Socket::CreateSocket(device_1, UdpSocketFactory::GetTypeId());
    server_socket_1->Bind(InetSocketAddress(Ipv4Address::GetAny(), PORT));
    server_socket_1->SetRecvCallback(MakeCallback(&CommonDevice::receivePacket, PeekPointer(common_device_1)));

    Ptr<Socket> server_socket_2 = Socket::CreateSocket(device_2, UdpSocketFactory::GetTypeId());
    server_socket_2->Bind(InetSocketAddress(Ipv4Address::GetAny(), PORT));
    server_socket_2->SetRecvCallback(MakeCallback(&CommonDevice::receivePacket, PeekPointer(common_device_2)));

    // Socket nó cliente
    Ptr<Socket> clientSocket = Socket::CreateSocket(client, UdpSocketFactory::GetTypeId());
    clientSocket->Bind();
    clientSocket->Connect(InetSocketAddress(interfaces.GetAddress(1), PORT));
    
    common_device_1->start();
    common_device_2->start();

    Simulator::Schedule(Seconds(1), &sendPacket, clientSocket, Seconds(0.5));

    Simulator::Stop(Seconds(2000.0));
    Simulator::Run();

    common_device_1->printStats();
    common_device_2->printStats();

    Simulator::Destroy();

    return 0;
}
