#ifndef SRC_NODE2_HPP_
#define SRC_NODE2_HPP_

#include <ch.hpp>
#include <uavcan_stm32/uavcan_stm32.hpp>

class UavcanNode {
    static constexpr unsigned MemPoolSize = 10752; ///< Refer to the libuavcan manual to learn why
    static constexpr unsigned RxQueueLenPerIface = 64;
    static constexpr unsigned StackSize = 3000;

public:
    typedef uavcan::Node<MemPoolSize> Node;
    typedef uavcan_stm32::CanInitHelper<RxQueueLenPerIface> CanInitHelper;

    UavcanNode(uavcan::ICanDriver &can_driver,
            uavcan::ISystemClock &system_clock);

    virtual ~UavcanNode();

    static int start(uavcan::NodeID node_id, uint32_t bitrate);
    Node &get_node() {
        return _node;
    }

private:
    void fill_node_info();
    int init(uavcan::NodeID node_id);
    void node_spin_once();
    int run();
    bool _task_should_exit = false; ///< flag to indicate to tear down the CAN driver

    static UavcanNode *_instance;
    Node _node;

    class NodeThread: public chibios_rt::BaseStaticThread<UavcanNode::StackSize> {
    protected:
        virtual msg_t main(void) {
            setName("node");
            //UavcanNode::start(2, 1000000);
            _instance->run();
            return 0; //should never be reached
        }
    public:
        NodeThread(void) : BaseStaticThread<UavcanNode::StackSize>() {
        }
    };
    static NodeThread _thread;
};


#endif /* SRC_NODE2_HPP_ */
