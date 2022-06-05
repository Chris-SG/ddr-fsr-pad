#include "panel.h"

Panel::Panel(uint8_t index, uint8_t sensorCount) : _panelIndex{index}, _sensorCount{sensorCount}, _state{false} {
    auto count = min(sensorCount, SENSOR_LIMIT);
    for (auto i = 0; i < count; i++) {
        _sensors[i] = Sensor(0, 0, 0);
    }
}

Panel::~Panel() {
}

bool Panel::Evaluate(int16_t (*data_fetcher)(uint16_t)) {
    auto sensorBase = _panelIndex * _sensorCount;
    for (auto i = 0; i < _sensorCount; i++) {
        int16_t sensorPressure = data_fetcher(sensorBase + i);
        if (_sensors[i].Evaluate(sensorPressure)) {
            _state = true;
            return true;
        }
    }
    _state = false;
    return false;
}

bool Panel::GetState() {
    return _state;
}

void Panel::SetSensor(uint8_t sensorIndex, uint16_t pressThreshold, uint16_t depressThreshold, uint16_t step) {
    if (sensorIndex > _sensorCount) return;
    _sensors[sensorIndex].SetPressThreshold(pressThreshold);
    _sensors[sensorIndex].SetDepressThreshold(depressThreshold);
    _sensors[sensorIndex].SetStep(step);
}

sensor_settings Panel::SensorData(uint8_t sensor) {
    return _sensors[sensor].GetAll();
}

void Panel::GetSensor(uint8_t sensorIndex, uint16_t &pressThreshold, uint16_t &depressThreshold, uint16_t &step) {
    if (sensorIndex > _sensorCount) return;
    pressThreshold = _sensors[sensorIndex].GetPressThreshold();
    depressThreshold = _sensors[sensorIndex].GetDepressThreshold();
    step = _sensors[sensorIndex].GetStep();
}
