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

private:
    char _buffer[128];
    void (*clear_action)();
};

#endif //FSR_SERIAL_PROCESSOR_H
