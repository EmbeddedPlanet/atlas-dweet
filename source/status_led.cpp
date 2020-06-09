#include "status_led.h"

StatusLed *StatusLed::instance = NULL;
DigitalOut status_led_internal(PIN_NAME_LED_RED);

StatusLed *StatusLed::get_instance()
{
    if (!instance) {
        instance = new StatusLed;
        //init thread and event queue
        instance->led_init();
    }

    return instance;
}

void StatusLed::enabled(bool enabled)
{
    // Write a 0 for true and 1 for false since the LED is active low
    status_led_internal = enabled ? 0 : 1;
}

bool StatusLed::current_state()
{
    // Return true for 0 since the LED is active low
    return status_led_internal == 0;
}
//invert the state of the status LED
void StatusLed::toggle()
{
    // Toggle status LED state
    bool state = status_led_internal;
    state = !state;
    status_led_internal = state;
}

//Set the status LED to blink continuously at the rate defined by the parameter
//rateInMs should be a positive integer value representing the blink rate
//in milliseconds
//This function executes as long as blinkEnable is true
void StatusLed::blink_continous(float rateInMs)
{

    if (blinkEnable) {
        ledEventQueue.call_in(rateInMs, callback(this, &StatusLed::toggle));
        ledEventQueue.call_in(2 * rateInMs, callback(this, &StatusLed::toggle));
        ledEventQueue.call_in(2 * rateInMs, callback(this, &StatusLed::blink_continous), rateInMs);
    }

}

//Set the status LED to blink using the parameters "blinks" to define
//how many times to blink and "rateInMs" to define how quickly the
//state will toggle on and off
void StatusLed::blink(int blinks, float rateInMs)
{
    ledEventQueue.call_in(rateInMs, callback(this, &StatusLed::toggle));
    ledEventQueue.call_in(2 * rateInMs, callback(this, &StatusLed::toggle));

    blinks--;
    if (blinks >= 0) {
        ledEventQueue.call_in(2 * rateInMs, callback(this, &StatusLed::blink), blinks, rateInMs);
    }

}

//Set the private blink enable variable true or false to control
//the execution of the continuous blink function
void StatusLed::set_blinkEnable(uint8_t blinkEn)
{
    blinkEnable = blinkEn;
}

//Initialize the thread and event queue objects for the led service
void StatusLed::led_init(void)
{
    ledThread.start(callback(&ledEventQueue, &EventQueue::dispatch_forever));
}
