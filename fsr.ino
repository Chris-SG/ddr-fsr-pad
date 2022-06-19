#include "settings.h"
#include "panel.h"
#include "serial_processor.h"
#include <EEPROM.h>
#include <Joystick.h>

#define BTN_MAX 12

#define UPDATE_RATE 1000

#define PANEL_LIMIT 4

#if defined(ARDUINO_AVR_MICRO)
#define __A0 A0
#define __A1 A1
#define __A2 A2
#define __A3 A3
#define __A4 A6
#define __A5 A7
#define __A6 A8
#define __A7 A9
#elif defined(ARDUINO_AVR_LEONARDO)
#define __A0 A0
#define __A1 A1
#define __A2 A2
#define __A3 A3
#define __A4 A4
#define __A5 A5
#define __A6 A6
#define __A7 A7
#endif

uint8_t panelCount;
uint8_t sensorCount;
Panel panels[PANEL_LIMIT];

SerialProcessor* serialProcessor;

uint8_t pins[PANEL_LIMIT * SENSOR_LIMIT] = { __A0, __A1, __A2, __A3, __A4, __A5, __A6, __A7 };

Joystick_ Joystick( JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                    BTN_MAX, 0, false, false, false, false, false,
                    false, false, false, false, false, false );

unsigned long lastUpdate = 0;

struct eeprom_sensor_data {
    uint16_t press_threshold;
    uint16_t depress_threshold;
    uint16_t step;
};

struct eeprom_data {
    uint8_t panel_count;
    uint8_t sensors_per_panel;
    eeprom_sensor_data sensor_data[PANEL_LIMIT * SENSOR_LIMIT];
    bool valid;
};

int16_t getSensorPressure(uint16_t index) {
#if DEBUG
    auto val = analogRead(pins[index]);
    sprintf(debug_buffer, "READ INDEX %hu VAL %hu\n", index, val);
    Serial.write(debug_buffer);
    return val;
#else
    return analogRead(pins[index]);
#endif
}

eeprom_data read_eeprom() {
    int eeprom_addr = 0;

    uint16_t eepromCheck1;
    EEPROM.get(eeprom_addr, eepromCheck1);
    eeprom_addr += sizeof(uint16_t);

    eeprom_data data{};
    if (eepromCheck1 == UINT16_MAX) {
#if DEBUG
        Serial.write("EEPROM data invalid, skipping\n");
#endif
        data.valid = false;
        return data;
    }

    EEPROM.get(eeprom_addr, data.panel_count);
    eeprom_addr += sizeof(data.panel_count);
    EEPROM.get(eeprom_addr, data.sensors_per_panel);
    eeprom_addr += sizeof(data.sensors_per_panel);

    for (auto i = 0; i < data.sensors_per_panel * data.panel_count; i++) {
        eeprom_sensor_data sensor;
        EEPROM.get(eeprom_addr, sensor);
        eeprom_addr += sizeof(sensor);
        data.sensor_data[i] = sensor;
    }
    uint16_t eepromCheck2;
    EEPROM.get(eeprom_addr, eepromCheck2);
    data.valid = eepromCheck1 != UINT16_MAX && eepromCheck1 == eepromCheck2;

#if DEBUG
    Serial.write("READ EEPROM DATA FROM ARDUINO\n");
    sprintf(debug_buffer, "PANEL COUNT: %hhu\nSENSORS PER PANEL: %hhu\nCHECK1: %u\nCHECK2: %u\nVALID DATA: %hhu\n", data.panel_count, data.sensors_per_panel, eepromCheck1, eepromCheck2, data.valid);
    Serial.write(debug_buffer);
    for (auto i = 0; i < sizeof(data.sensor_data) / sizeof(eeprom_sensor_data); i++) {
        sprintf(debug_buffer, "SENSOR %d:\n%hu %hu %hu\n", i, data.sensor_data[i].press_threshold, data.sensor_data[i].depress_threshold, data.sensor_data[i].step);
        Serial.write(debug_buffer);
    }
    Serial.write('\n');
#endif

    return data;
}

void write_eeprom(const eeprom_data& data) {
    int eeprom_addr = 0;
    uint16_t time = micros() & UINT16_MAX;

    EEPROM.put(eeprom_addr, time);
    eeprom_addr += sizeof(uint16_t);

    EEPROM.put(eeprom_addr, data.panel_count);
    eeprom_addr += sizeof(data.panel_count);
    EEPROM.put(eeprom_addr, data.sensors_per_panel);
    eeprom_addr += sizeof(data.sensors_per_panel);

    for (auto i = 0; i < data.sensors_per_panel * data.panel_count; i++) {
        EEPROM.put(eeprom_addr, data.sensor_data[i]);
        eeprom_addr += sizeof(data.sensor_data[i]);
    }
    EEPROM.put(eeprom_addr, time);
#if DEBUG
    Serial.write("WROTE EEPROM DATA TO ARDUINO\n");
    sprintf(debug_buffer, "PANEL COUNT: %hhu\nSENSORS PER PANEL: %hhu\nVALID CHECK: %u\n", data.panel_count, data.sensors_per_panel, time);
    Serial.write(debug_buffer);
    for (auto i = 0; i < sizeof(data.sensor_data) / sizeof(eeprom_sensor_data); i++) {
        sprintf(debug_buffer, "SENSOR %d:\n%hu %hu %hu\n", i, data.sensor_data[i].press_threshold, data.sensor_data[i].depress_threshold, data.sensor_data[i].step);
        Serial.write(debug_buffer);
    }
    Serial.write('\n');
#endif
}

void populatePanelSensorData(const eeprom_data& data) {
    panelCount = min(PANEL_LIMIT, data.panel_count);
    sensorCount = min(SENSOR_LIMIT, data.sensors_per_panel);

    for(auto i = 0; i < panelCount; i++) {
        panels[i] = Panel(i, sensorCount);

        for (auto j = 0; j < sensorCount; j++) {
            auto sensorData = data.sensor_data[i*sensorCount+j];
#if DEBUG
            sprintf(debug_buffer, "Setting panel %d sensor %d: %hu %hu %hu\n", i, j, sensorData.press_threshold, sensorData.depress_threshold,sensorData.step);
            Serial.print(debug_buffer);
#endif
            panels[i].SetSensor(
                    j,
                    sensorData.press_threshold,
                    sensorData.depress_threshold,
                    sensorData.step
            );
        }
    }
}

eeprom_data initialData() {
    auto data = read_eeprom();
    if (!data.valid) {
        data.valid = true;
        data.panel_count = panelCount;
        data.sensors_per_panel = sensorCount;

        for (auto i = 0; i < panelCount*sensorCount; i++) {
            data.sensor_data[i].depress_threshold = 0;
            data.sensor_data[i].press_threshold = 0;
            data.sensor_data[i].step = 0;
        }
    }

    return data;
}

void initializePanels() {
    auto data = initialData();

    if (data.valid) {
        populatePanelSensorData(data);
    }
}

void clear_eeprom() {
    for (auto const addr : EEPROM) {
        EEPROM.put(addr, UINT16_MAX);
    }
}

void print_settings() {
    sprintf(buffer, "PANEL_COUNT %hhu SENSOR_COUNT %hhu PANEL_LIMIT %d SENSOR_LIMIT %d BUTTON_LIMIT %d\n\u0004\u0003", panelCount, sensorCount, PANEL_LIMIT, SENSOR_LIMIT, BTN_MAX);
    Serial.write(buffer);
}

void set_panel_sensor_counts(uint8_t newPanelCount, uint8_t newSensorCount) {
#if DEBUG
    sprintf(debug_buffer, "CHANGING PANEL COUNT FROM %huu to %huu AND SENSOR COUNT FROM %huu to %huu\n", panelCount, newPanelCount, sensorCount, newSensorCount);
    Serial.write(debug_buffer);
#endif
    clear_eeprom();
    panelCount = newPanelCount;
    sensorCount = newSensorCount;
    initializePanels();
}

void print_panel_data(uint8_t index) {
    sprintf(buffer, "PANEL %hhu\n", index);
    Serial.write(buffer);
    for (auto i = 0; i < sensorCount; i++) {
        auto data = panels[index].SensorData(i);
        sprintf(buffer, "SENSOR %d: %d %d %d %d %hhu\n", i, data.press, data.depress, data.step, data.prev, data.state);
        Serial.write(buffer);
    }
    Serial.write("\u0004\u0003");
}

void write_panel_sensor_data(uint8_t panel, uint8_t sensor, uint16_t press, uint16_t depress, uint16_t step) {
    panels[panel].SetSensor(sensor, press, depress, step);

    eeprom_data data{};
    data.panel_count = panelCount;
    data.sensors_per_panel = sensorCount;
    for (auto i = 0; i < panelCount; i++) {
        for (auto j = 0; j < sensorCount; j++) {
            uint16_t press, depress, step;
            panels[i].GetSensor(j, press, depress, step);
            data.sensor_data[sensorCount*i+j].press_threshold = press;
            data.sensor_data[sensorCount*i+j].depress_threshold = depress;
            data.sensor_data[sensorCount*i+j].step = step;
        }
    }
    write_eeprom(data);
#if DEBUG
        sprintf(debug_buffer, "Wrote to panel %hhu sensor %hhu: %d, %d, %d\n", panel, sensor, press, depress, step);
        Serial.print(debug_buffer);
        Serial.print('\n');
#endif
}

void setup() {
    serialProcessor = new SerialProcessor(115200);
    serialProcessor->SetClearAction(clear_eeprom);
    serialProcessor->SetPrintSettingsAction(print_settings);
    serialProcessor->SetPanelSensorCountAction(set_panel_sensor_counts);
    serialProcessor->SetPrintPanelDataAction(print_panel_data);
    serialProcessor->SetWritePanelDataAction(write_panel_sensor_data);

    panelCount = 4;
    sensorCount = 2;

#if DEBUG
    while (!Serial);
#endif
    initializePanels();
    Joystick.begin();

    Serial.print("READY\n");
}

void loop() {
    unsigned long startMicros = micros();
    serialProcessor->Process();

    if (lastUpdate + UPDATE_RATE <= startMicros) {
        lastUpdate += UPDATE_RATE;
        for (auto i = 0; i < panelCount; i++) {
            if (panels[i].GetState() != panels[i].Evaluate(getSensorPressure)) {
                Joystick.setButton(panels[i].PanelIndex(), panels[i].GetState());
            }
        }
    }
}
