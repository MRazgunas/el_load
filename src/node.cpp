#include <node.hpp>

#include <ch.hpp>
#include <hal.h>
#include <uavcan/uavcan.hpp>
#include <uavcan_stm32/uavcan_stm32.hpp>


namespace node{

static constexpr int RxQueueSize = 64;
static constexpr std::uint32_t BitRate = 1000000;

uavcan::ISystemClock& getSystemClock()
{
    return uavcan_stm32::SystemClock::instance();
}

uavcan::ICanDriver& getCanDriver()
{
    static uavcan_stm32::CanInitHelper<RxQueueSize> can;
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
        int res = can.init(BitRate);
        if (res < 0)
        {
            // Handle the error
            printf("UAVCAN: initialization failed");
        }
    }
    return can.driver;
}


/**
 * Node object will be constructed at the time of the first access.
 * Note that most library objects are noncopyable (e.g. publishers, subscribers, servers, callers, timers, ...).
 * Attempt to copy a noncopyable object causes compilation failure.
 */
static Node& getNode()
{
    static Node node(getCanDriver(), getSystemClock());
    return node;
}

class NodeThread: public chibios_rt::BaseStaticThread<3000> {
    protected:
        virtual msg_t main(void) {
            return 0; //should never be reached
        }
    public:
        NodeThread(void) : chibios_rt::BaseStaticThread<3000>() {
        }
    };

static NodeThread node_thread;

int init(){

    const int self_node_id = 2;

    /*
     * Node initialization.
     * Node ID and name are required; otherwise, the node will refuse to start.
     * Version info is optional.
     */
    auto& node = getNode();

    node.setNodeID(self_node_id);
    node.setName("org.kmti.elload");

    /*
     * Start the node.
     * All returnable error codes are listed in the header file uavcan/error.hpp.
     */
    const int node_start_res = node.start();
    if (node_start_res < 0) {
        printf("Node init: node failed to start with error %d", node_start_res);
        return node_start_res;
    }

    /*
     * Informing other nodes that we're ready to work.
     * Default mode is INITIALIZING.
     */
    node.setModeOperational();

    node_thread.start(NORMALPRIO);
    return 0;
}

int run() {

    return 0;
}
}
