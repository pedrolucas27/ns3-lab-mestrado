#ifndef ROUTER_TAG_H
#define ROUTER_TAG_H

#include "ns3/tag.h"

#include <cstdint>
#include <ostream>

namespace ns3 {

class RouteTag : public Tag {
  public:
    static TypeId GetTypeId() {
        static TypeId tid = TypeId("ns3::RouteTag")
                                .SetParent<Tag>()
                                .AddConstructor<RouteTag>();
        return tid;
    }

    TypeId GetInstanceTypeId() const override {
        return GetTypeId();
    }

    uint32_t GetSerializedSize() const override {
        return sizeof(m_hops) + sizeof(m_max_hops) + sizeof(m_visited);
    }

    void Serialize(TagBuffer buffer) const override {
        buffer.WriteU8(m_hops);
        buffer.WriteU8(m_max_hops);
        buffer.WriteU64(m_visited);
    }

    void Deserialize(TagBuffer buffer) override {
        m_hops = buffer.ReadU8();
        m_max_hops = buffer.ReadU8();
        m_visited = buffer.ReadU64();
    }

    void Print(std::ostream& os) const override {
        os << "hops=" << static_cast<unsigned>(m_hops);
    }

    uint8_t m_hops = 0;
    uint8_t m_max_hops = 0;
    uint64_t m_visited = 0;
};

} // namespace ns3

#endif // ROUTER_TAG_H