#include "energy-buffer.h"

#include <algorithm>

namespace ns3{

EnergyBuffer::EnergyBuffer(double initial_energy, double capacity)
    : energy_amount(std::min(initial_energy, capacity)),
      max_energy(capacity),
      min_energy(energy_amount)
{
}

void EnergyBuffer::update_min()
{
    if (energy_amount < min_energy)
    {
        min_energy = energy_amount;
    }
}

bool EnergyBuffer::consume_energy(double amount)
{
    if (energy_amount >= amount)
    {
        energy_amount -= amount;
        update_min();
        return true;
    }
    return false;
}

void EnergyBuffer::recharge(double amount)
{
    energy_amount += amount;
    if (energy_amount > max_energy)
    {
        energy_amount = max_energy;
    }
}

bool EnergyBuffer::has_energy() const
{
    return energy_amount > 0;
}

double EnergyBuffer::get_energy_amount() const
{
    return energy_amount;
}

double EnergyBuffer::get_min_energy() const
{
    return min_energy;
}

void EnergyBuffer::set_max_energy(double capacity)
{
    max_energy = capacity;
    if (energy_amount > max_energy)
    {
        energy_amount = max_energy;
    }
}

} // namespace ns3
