#define DEBUG true

#include "panel.h"
#include "serial_processor.h"
#include <EEPROM.h>
#include <Joystick.h>

#define BTN_MAX 12

#define UPDATE_RATE 1000

#define PANEL_LIMIT 5

uint8_t panelCount;
uint8_t sensorCount;
Panel panels[PANEL_LIMIT];

SerialProcessor* serialProcessor;

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
    return analogRead(index + A0);
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
    char debugBuf[512];
    Serial.write("READ EEPROM DATA FROM ARDUINO\n");
    sprintf(debugBuf, "PANEL COUNT: %hhu\nSENSORS PER PANEL: %hhu\nCHECK1: %u\nCHECK2: %u\nVALID DATA: %hhu\n", data.panel_count, data.sensors_per_panel, eepromCheck1, eepromCheck2, data.valid);
    Serial.write(debugBuf);
    for (auto i = 0; i < sizeof(data.sensor_data) / sizeof(eeprom_sensor_data); i++) {
        sprintf(debugBuf, "SENSOR %d:\n%hu %hu %hu\n", i, data.sensor_data[i].press_threshold, data.sensor_data[i].depress_threshold, data.sensor_data[i].step);
        Serial.write(debugBuf);
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
    char debugBuf[512];
    Serial.write("WROTE EEPROM DATA TO ARDUINO\n");
    sprintf(debugBuf, "PANEL COUNT: %hhu\nSENSORS PER PANEL: %hhu\nVALID CHECK: %u\n", data.panel_count, data.sensors_per_panel, time);
    Serial.write(debugBuf);
    for (auto i = 0; i < sizeof(data.sensor_data) / sizeof(eeprom_sensor_data); i++) {
        sprintf(debugBuf, "SENSOR %d:\n%hu %hu %hu\n", i, data.sensor_data[i].press_threshold, data.sensor_data[i].depress_threshold, data.sensor_data[i].step);
        Serial.write(debugBuf);
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
            char buf[512];
            sprintf(buf, "Setting panel %d sensor %d: %hu %hu %hu\n", i, j, sensorData.press_threshold, sensorData.depress_threshold,sensorData.step);
            Serial.print(buf);
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

void set_panel_sensor_counts(uint8_t newPanelCount, uint8_t newSensorCount) {
#if DEBUG
    char debugBuf[512];
    sprintf(debugBuf, "CHANGING PANEL COUNT FROM %huu to %huu AND SENSOR COUNT FROM %huu to %huu\n", panelCount, newPanelCount, sensorCount, newSensorCount);
    Serial.write(debugBuf);
#endif
    clear_eeprom();
    panelCount = newPanelCount;
    sensorCount = newSensorCount;
    initializePanels();
}

void setup() {
    serialProcessor = new SerialProcessor(115200);
    serialProcessor->SetClearAction(clear_eeprom);
    serialProcessor->SetPanelSensorCountAction(set_panel_sensor_counts);

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
    auto updated = serialProcessor->Process(panels);
    if (updated) {
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
    }
    if (lastUpdate + UPDATE_RATE <= startMicros) {
        lastUpdate += UPDATE_RATE;
        for (auto i = 0; i < panelCount; i++) {
            if (panels[i].GetState() != panels[i].Evaluate(getSensorPressure)) {
                Joystick.setButton(panels[i].PanelIndex(), panels[i].GetState());
            }
        }
    }
}
