#ifndef FSR_SENSOR_H
#define FSR_SENSOR_H

#include <Arduino.h>

struct sensor_settings {
    uint16_t press;
    uint16_t depress;
    int16_t step;
    int16_t prev;
    bool state;
};

class Sensor {
public:
    Sensor() {}
    Sensor(uint16_t press_threshold, uint16_t depress_threshold, int16_t max_step);
    ~Sensor();

    bool Evaluate(int16_t new_val);
    void SetPressThreshold(uint16_t threshold);
    uint16_t GetPressThreshold();
    void SetDepressThreshold(uint16_t threshold);
    uint16_t GetDepressThreshold();
    void SetStep(uint16_t stepSize);
    uint16_t GetStep();
    sensor_settings GetAll();

private:
    uint16_t _press_threshold;
    uint16_t _depress_threshold;
    int16_t _max_step;

    int16_t _prev_value;
    bool _state;
};

#endif //FSR_SENSOR_H
