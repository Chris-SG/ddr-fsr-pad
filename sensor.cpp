#include "sensor.h"

extern const bool DEBUG;

Sensor::Sensor(uint16_t press_threshold, uint16_t depress_threshold, int16_t max_step)
    :
        _press_threshold{press_threshold}, _depress_threshold{depress_threshold}, _max_step{max_step}, _prev_value{0}, _state{false} {
}

Sensor::~Sensor() {}

bool Sensor::Evaluate(int16_t new_val) {
    int16_t val_to_set = min(max(new_val - _prev_value, - _max_step), _max_step);
    _prev_value += val_to_set;
    if (_state) {
        if (_prev_value <= _depress_threshold) {
            if (DEBUG) {
                Serial.print("Sensor is no longer pressed\n");
            }
            _state = false;
        }
    } else {
        if (_prev_value >= _press_threshold) {
            if (DEBUG) {
                Serial.print("Sensor is now pressed\n");
            }
            _state = true;
        }
    }
    return _state;
}

void Sensor::SetPressThreshold(uint16_t threshold) {
    _press_threshold = threshold;
}

uint16_t Sensor::GetPressThreshold() {
    return _press_threshold;
}

void Sensor::SetDepressThreshold(uint16_t threshold) {
    _depress_threshold = threshold;
}

uint16_t Sensor::GetDepressThreshold() {
    return _depress_threshold;
}

void Sensor::SetStep(uint16_t stepSize) {
    _max_step = stepSize;
}

uint16_t Sensor::GetStep() {
    return _max_step;
}