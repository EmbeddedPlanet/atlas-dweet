#ifndef UTILS_STATUS_LED_H
#define UTILS_STATUS_LED_H

#include "mbed.h"

class StatusLed {
public:
    // Singleton instance getter
    static StatusLed* get_instance();

    void enabled(bool enabled);
    bool current_state();
    void toggle(void);
    void blink(int blinks, float rateInMs);
    void blink_continous(float rateInMs);
    void set_blinkEnable(uint8_t blinkEn);
    void led_init(void);

private:
    uint8_t blinkEnable;
    StatusLed(){};
    StatusLed(StatusLed const&){};
    StatusLed& operator=(StatusLed const&){};
    static StatusLed* instance;

    Thread ledThread;
    EventQueue ledEventQueue; 
};

#endif // UTILS_STATUS_LED_H
