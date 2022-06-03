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
#if DEBUG
    char buffer [1024] = { 0 };
#endif
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
#if DEBUG
                sprintf(buffer, "Executing p command\n");
                Serial.print(buffer);
#endif
                uint8_t panelIndex;
                if (sscanf(_buffer, "p %hhu", &panelIndex)) {
                    PrintPanelData(panels[panelIndex]);
                }
#if DEBUG
                sprintf(buffer, "Executed p command\n");
                Serial.print(buffer);
#endif
                break;
            case 'w':
#if DEBUG
                sprintf(buffer, "Executing w command\n");
                Serial.print(buffer);
#endif
                uint8_t panel, sensor;
                uint16_t press, depress, step;
                if (sscanf(_buffer, "w %hhu %hhu %hu %hu %hu", &panel, &sensor, &press, &depress, &step) == 5) {
                    WritePanelData(panels[panel], sensor, press, depress, step);
#if DEBUG
                    sprintf(buffer, "Executed w command\n");
                Serial.print(buffer);
#endif
                    return true;
                }
                break;
            case 'c':
#if DEBUG
                sprintf(buffer, "Executing c command\n");
                Serial.print(buffer);
#endif
                clear_action();
#if DEBUG
                sprintf(buffer, "Executed c command\n");
                Serial.print(buffer);
#endif
                break;
            case 'n':
#if DEBUG
                sprintf(buffer, "Executing n command\n");
                Serial.print(buffer);
#endif
                uint8_t panelCount, sensorCount;
                if (sscanf(_buffer, "n %hhu %hhu", &panelCount, &sensorCount) == 2) {
                    set_panel_sensor_count(panelCount, sensorCount);
                }
#if DEBUG
                sprintf(buffer, "Executed n command\n");
                Serial.print(buffer);
#endif
                break;
        }
    }
    return false;
}
