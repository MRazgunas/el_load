#ifndef SRC_NODE_HPP_
#define SRC_NODE_HPP_

#include <uavcan/uavcan.hpp>

namespace node{

constexpr unsigned NodeMemoryPoolSize = 4096;
typedef uavcan::Node<NodeMemoryPoolSize> Node;

uavcan::ISystemClock& getSystemClock();
uavcan::ICanDriver& getCanDriver();
int init();
void run();
}

#endif /* SRC_NODE_HPP_ */
