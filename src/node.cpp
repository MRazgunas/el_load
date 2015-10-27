#include <node.hpp>

#include <ch.hpp>
#include <hal.h>
#include <uavcan_stm32/uavcan_stm32.hpp>

#include <uavcan/protocol/debug/KeyValue.hpp> // uavcan.protocol.debug.KeyValue


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

class NodeThread: public chibios_rt::BaseStaticThread<1500> {
    protected:
        virtual msg_t main(void) {
            run();
            return 0; //should never be reached
        }
    public:
        NodeThread(void) : chibios_rt::BaseStaticThread<1500>() {
        }
    };

static NodeThread node_thread;

uavcan::Publisher<uavcan::protocol::debug::KeyValue> kv_pub(getNode());

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

    const int kv_pub_init_res = kv_pub.init();
    if (kv_pub_init_res < 0) {
        printf("Failed to start the publisher; error: %i", kv_pub_init_res);
    }
    /*
     * Informing other nodes that we're ready to work.
     * Default mode is INITIALIZING.
     */
    node.setModeOperational();

    node_thread.start(NORMALPRIO);
    return 0;
}

void node_spin_once()
{
    const int spin_res = getNode().spin(uavcan::MonotonicDuration::fromMSec(200));
    if (spin_res < 0) {
        printf("node spin error %i", spin_res);
    }
}

void run()
{
    while (1) {
        node_spin_once();  // Non-blocking

        uavcan::protocol::debug::KeyValue kv_msg;  // Always zero initialized
        kv_msg.value = std::rand() / float(RAND_MAX);
        kv_msg.key = "a";   // "a"
        kv_msg.key += "b";  // "ab"
        kv_msg.key += "c";  // "abc"
        const int pub_res = kv_pub.broadcast(kv_msg);
        if (pub_res < 0) {
            printf("KV publication failure: %i", pub_res);
        }
    }
}
}
