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
    void Process();

    void SetClearAction(void (*clear_action)()) {
        this->clear_action = clear_action;
    }

    void SetPrintSettingsAction(void (*print_settings)()) {
        this->print_settings = print_settings;
    }

    void SetPanelSensorCountAction(void (*set_panel_sensor_count)(uint8_t panels, uint8_t sensors)) {
        this->set_panel_sensor_count = set_panel_sensor_count;
    }

    void SetPrintPanelDataAction(void (*print_panel_data)(uint8_t index)) {
        this->print_panel_data = print_panel_data;
    }

    void SetWritePanelDataAction(void (*write_panel_data)(uint8_t panel, uint8_t sensor, uint16_t press, uint16_t depress, uint16_t step)) {
        this->write_panel_data = write_panel_data;
    }

private:
    void (*clear_action)();
    void (*print_settings)();
    void (*set_panel_sensor_count)(uint8_t panels, uint8_t sensors);
    void (*print_panel_data)(uint8_t index);
    void (*write_panel_data)(uint8_t panel, uint8_t sensor, uint16_t press, uint16_t depress, uint16_t step);
};

#endif //FSR_SERIAL_PROCESSOR_H
