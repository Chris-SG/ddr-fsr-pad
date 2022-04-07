#ifndef FSR_SERIAL_PROCESSOR_H
#define FSR_SERIAL_PROCESSOR_H

#include "panel.h"

class SerialProcessor {
public:
    SerialProcessor(long baud_rate);
    bool Process(Panel panels[]);

private:
    char _buffer[128];
};

#endif //FSR_SERIAL_PROCESSOR_H
