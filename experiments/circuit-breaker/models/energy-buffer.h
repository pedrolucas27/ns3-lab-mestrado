#ifndef ENERGY_BUFFER_H
#define ENERGY_BUFFER_H

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * Armazena e gerencia a energia de um dispositivo com colheita de energia.
 * O buffer satura na capacidade máxima definida na construção.
 */
class EnergyBuffer : public SimpleRefCount<EnergyBuffer> {
  public:
    /**
     * Constrói um buffer de energia.
     * @param initial_energy Energia inicial armazenada (J).
     * @param capacity Capacidade máxima do buffer (J).
     */
    EnergyBuffer(double initial_energy, double capacity);

    /**
     * Consome energia do buffer.
     * Se a energia for insuficiente, o buffer é zerado e retorna false.
     * @param amount Quantidade de energia a consumir (J).
     * @return True se havia energia suficiente; caso contrário, False.
     */
    bool consume_energy(double amount);

    /**
     * Adiciona energia ao buffer, limitando à capacidade máxima.
     * @param amount Quantidade de energia colhida (J).
     */
    void recharge(double amount);

    /**
     * Verifica se o buffer possui energia.
     * @return True se a energia armazenada for maior que zero.
     */
    bool has_energy() const;

    /**
     * Obtém a energia atualmente armazenada.
     * @return Energia atual no buffer (J).
     */
    double get_energy_amount() const;

    /**
     * Menor energia observada desde a construção (após consumos).
     */
    double get_min_energy() const;

    /**
     * Redefine a capacidade máxima do buffer.
     * @param capacity Nova capacidade máxima (J).
     */
    void set_max_energy(double capacity);

  private:
    void update_min();

    double energy_amount; //!< Energia atualmente armazenada (J)
    double max_energy;    //!< Capacidade máxima do buffer (J)
    double min_energy;    //!< Menor energia observada (J)
};

} // namespace ns3

#endif // ENERGY_BUFFER_H
