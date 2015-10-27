#include <ch.hpp>
#include <hal.h>
#include <node.hpp>
//#include <uavcan/uavcan.hpp>

using namespace chibios_rt;

/*
 * LED blinker thread, times are in milliseconds.
 */
class BlinkerThread : public BaseStaticThread<128> {
protected:
    virtual msg_t main(void) {
        setName("blinker");
        while (TRUE) {
            palClearPad(GPIOC, GPIOC_LED1);
            chThdSleepMilliseconds(500);
            palSetPad(GPIOC, GPIOC_LED1);
            chThdSleepMilliseconds(500);
        }
    }
public:
    BlinkerThread(void) : BaseStaticThread<128>() {
    }
};
static BlinkerThread blinker;

/*
 * Application entry point.
 */
int main(void) {

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    System::init();

    node::init();

    //static node_handler::Node node(node_handler::getCanDriver(), node_handler::getSystemClock());

    /*
     * Starts the blinker thread.
     */
    blinker.start(NORMALPRIO - 1);

    /*
     * Normal main() thread activity, in this demo it does nothing except
     * sleeping in a loop.
     */
   // printf("Test app starting");
    while (TRUE) {
        //BaseThread::sleep((MS2ST(1000)));
      //  node.spin(uavcan::MonotonicDuration::fromMSec(1000));
    }
}
