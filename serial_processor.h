#ifndef FSR_SERIAL_PROCESSOR_H
#define FSR_SERIAL_PROCESSOR_H

#include "panel.h"

enum SerialAction {
    PRINT,
    WRITE,
    CLEAR
};

class SerialProcessor {
public:
    SerialProcessor(long baud_rate);
    bool Process(Panel panels[]);

    void SetClearAction(void (*clear_action)()) {
        this->clear_action = clear_action;
    }

    void SetPanelSensorCountAction(void (*set_panel_sensor_count)(uint8_t panels, uint8_t sensors)) {
        this->set_panel_sensor_count = set_panel_sensor_count;
    }

private:
    char _buffer[128];
    void (*clear_action)();
    void (*set_panel_sensor_count)(uint8_t panels, uint8_t sensors);
};

#endif //FSR_SERIAL_PROCESSOR_H
