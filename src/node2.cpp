#include <node2.hpp>
#include <ch.hpp>
#include <uavcan_stm32/uavcan_stm32.hpp>
#include <uavcan/uavcan.hpp>

UavcanNode *UavcanNode::_instance; //TODO: Maybe remove this?

UavcanNode::UavcanNode(uavcan::ICanDriver &can_driver, uavcan::ISystemClock &system_clock) :
    _node(can_driver, system_clock)
{
    printf("Created node");
}

int UavcanNode::start(uavcan::NodeID node_id, uint32_t bitrate)
{
    if (_instance != nullptr) {
        printf("UAVCAN already started!");
        return -1;
    }

    /*
     * CAN driver init
     */
    static CanInitHelper can;
    static bool can_initialized = false;

    if (!can_initialized) {
        const int can_init_res = can.init(bitrate);

        if (can_init_res < 0) {
            printf("CAN driver init failed %i", can_init_res);
            return can_init_res;
        }

        can_initialized = true;
    }

    /*
     * Node init
     */
    _instance = new UavcanNode(can.driver, uavcan_stm32::SystemClock::instance());

    if (_instance == nullptr) {
        printf("Out of memory");
        return -1;
    }

    const int node_init_res = _instance->init(node_id);

    if (node_init_res < 0) {
        delete _instance;
        _instance = nullptr;
        printf("Node init failed %i", node_init_res);
        return node_init_res;
    }

    /*
     * Start the task. Normally it should never exit.
     */
    _thread.start(NORMALPRIO);

    return 1;
}

void UavcanNode::fill_node_info()
{
    /* software version */
    uavcan::protocol::SoftwareVersion swver;

    //TODO: fill node info

    //_node.setSoftwareVersion(swver);

    /* hardware version */
    uavcan::protocol::HardwareVersion hwver;

    hwver.major = 1;
    hwver.minor = 0;

    _node.setHardwareVersion(hwver);
}

int UavcanNode::init(uavcan::NodeID node_id)
{
    _node.setName("org.kmti.elload");

    _node.setNodeID(node_id);

    fill_node_info();
    return _node.start();
}

void UavcanNode::node_spin_once()
{
    const int spin_res = _node.spin(uavcan::MonotonicTime());
    if (spin_res < 0) {
        printf("node spin error %i", spin_res);
    }
}

int UavcanNode::run()
{

    _node.setModeOperational();

    while (!_task_should_exit) {
        node_spin_once();  // Non-blocking
    }
    return 0;
}
