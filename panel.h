#ifndef FSR_PANEL_H
#define FSR_PANEL_H

#include <Arduino.h>
#include "sensor.h"

#define SENSOR_LIMIT 2

class Panel {
public:
    Panel() { Panel(0, SENSOR_LIMIT); }
    Panel(uint8_t index, uint8_t sensorCount);
    ~Panel();
    bool Evaluate(int16_t (*data_fetcher)(uint16_t));
    bool GetState();
    void SetSensor(uint8_t sensorIndex, uint16_t pressThreshold, uint16_t depressThreshold, uint16_t step);
    sensor_settings SensorData(uint8_t sensor);
    void GetSensor(uint8_t sensorIndex, uint16_t& pressThreshold, uint16_t& depressThreshold, uint16_t& step);
    uint8_t PanelIndex() { return _panelIndex; }
private:
    Sensor _sensors[SENSOR_LIMIT];
    uint8_t _sensorCount;
    uint8_t _panelIndex;
    bool _state;
};

#endif //FSR_PANEL_H
