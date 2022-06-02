#include "panel.h"

Panel::Panel(uint8_t index, uint8_t sensorCount) : _panelIndex{index}, _sensorCount{sensorCount}, _state{false} {
    for (auto i = 0; i < sensorCount; i++) {
        _sensors[i] = Sensor(0, 0, 0);
    }
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

void Panel::SensorData(char* buffer) {
    sprintf(buffer, "PANEL %d\n", _panelIndex);
    for (int i = 0; i < _sensorCount; i++) {
        char tmp[64] = { 0 };
        sprintf(tmp, "SENSOR %d: %d %d %d\n", i, _sensors[i].GetPressThreshold(), _sensors[i].GetDepressThreshold(), _sensors[i].GetStep());
        strcat(buffer, tmp);
    }
}

void Panel::GetSensor(uint8_t sensorIndex, uint16_t &pressThreshold, uint16_t &depressThreshold, uint16_t &step) {
    if (sensorIndex > _sensorCount) return;
    pressThreshold = _sensors[sensorIndex].GetPressThreshold();
    depressThreshold = _sensors[sensorIndex].GetDepressThreshold();
    step = _sensors[sensorIndex].GetStep();
}
