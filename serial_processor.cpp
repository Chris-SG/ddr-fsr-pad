#include <Arduino.h>
#include "serial_processor.h"

void WritePanelData(Panel& panel, uint8_t sensor, uint16_t press, uint16_t depress, uint16_t step) {
    panel.SetSensor(sensor, press, depress, step);
#if DEBUG
    sprintf(debug_buffer, "Wrote to panel %d sensor %d: %d, %d, %d\n", panel.PanelIndex(), sensor, press, depress, step);
    Serial.print(buffer);
    Serial.print('\n');
#endif
}

SerialProcessor::SerialProcessor(long baud_rate) {
    Serial.begin(baud_rate);
}

void SerialProcessor::Process() {
    while (Serial.available() > 0) {
        char buf[128];
        auto bytes_read = Serial.readBytesUntil('\n', buf, 127);
        buf[bytes_read] = '\0';
        if (bytes_read == 0) {
            return;
        }
        Serial.write(buf);
        Serial.write('\n');

        switch(buf[0]) {
            case 'p':
                uint8_t panelIndex;
                if (sscanf(buf, "p %hhu", &panelIndex)) {
                    print_panel_data(panelIndex);
                }
                break;
            case 'w':
                uint8_t panel, sensor;
                uint16_t press, depress, step;
                if (sscanf(buf, "w %hhu %hhu %hu %hu %hu", &panel, &sensor, &press, &depress, &step) == 5) {
                    write_panel_data(panel, sensor, press, depress, step);
                }
                break;
            case 'c':
                clear_action();
                break;
            case 'n':
                uint8_t panelCount, sensorCount;
                if (sscanf(buf, "n %hhu %hhu", &panelCount, &sensorCount) == 2) {
                    set_panel_sensor_count(panelCount, sensorCount);
                }
                break;
        }
        Serial.print("Done\n");
    }
}
