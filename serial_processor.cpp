#include <Arduino.h>
#include "serial_processor.h"

void PrintPanelData(Panel& panel) {
    char buffer[1024] = { 0 };
    panel.SensorData(buffer);
    Serial.write(buffer);
}

void WritePanelData(Panel& panel, uint8_t sensor, uint16_t press, uint16_t depress, uint16_t step) {
    panel.SetSensor(sensor, press, depress, step);
#if DEBUG
    char buffer [1024] = { 0 };
        sprintf(buffer, "Wrote to panel %d sensor %d: %d, %d, %d\n", panel.PanelIndex(), sensor, press, depress, step);
        Serial.print(buffer);
        Serial.print('\n');
#endif
}

SerialProcessor::SerialProcessor(long baud_rate) {
    Serial.begin(baud_rate);
}

bool SerialProcessor::Process(Panel panels[]) {
    while (Serial.available() > 0) {
        auto bytes_read = Serial.readBytesUntil('\n', _buffer, 127);
        _buffer[bytes_read] = '\0';
        if (bytes_read == 0) {
            return false;
        }
        Serial.write(_buffer);
        Serial.write('\n');

        switch(_buffer[0]) {
            case 'p':
                uint8_t panelIndex;
                if (sscanf(_buffer, "p %hhu", &panelIndex)) {
                    PrintPanelData(panels[panelIndex]);
                }
                break;
            case 'w':
                uint8_t panel, sensor;
                uint16_t press, depress, step;
                if (sscanf(_buffer, "w %hhu %hhu %hu %hu %hu", &panel, &sensor, &press, &depress, &step) == 5) {
                    WritePanelData(panels[panel], sensor, press, depress, step);
                    return true;
                }
                break;
        }
    }
    return false;
}
