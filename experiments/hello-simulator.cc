/*
 * Template de experimento ns-3.
 * Renomeie ou copie este arquivo para criar novos experimentos.
 */

#include "ns3/core-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HelloSimulator");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    std::cout << "Experimento hello-simulator executado com sucesso." << std::endl;

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
